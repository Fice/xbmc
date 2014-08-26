

#ifndef _PLT_SYNC_DATA_H_
#define _PLT_SYNC_DATA_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"
#include "PltStateVariable.h"
#include "PltMediaItem.h"
#include "PltSyncPolicy.h"
#include "PltUtilities.h"

//Status values for the status value in SyncStatusUpdate (STU) function
#define STU_STATUS_IN_PROGRESS            "IN_PROGRESS"
#define STU_STATUS_IN_PROGRESS_WITH_ERROR "IN_PROGRESS_WITH_ERROR"
#define STU_STATUS_COMPLETED              "COMPLETED"
#define STU_STATUS_COMPLETED_WITH_ERROR   "COMPLETED_WITH_ERROR"
#define STU_STATUS_STOPPED                "STOPPED"
#define STU_STATUS_TEMPORARILY_STOPPED    "TEMPORARILY_STOPPED"

typedef NPT_List<NPT_String> PLT_StringList;

bool IsSyncable(const NPT_String& object_class);
bool IsValidUDN(const NPT_String& udn);
bool IsValidID(const NPT_String& id);
bool TranslateErrorCode(const NPT_UInt32 errorCode, char** errorTitle, char** errorDescription);

struct PLT_UUIDGenerator
{
  template<typename T>
  void operator()(T& obj) const
  {
    obj.GenerateUUIDs();
  }
};

struct PLT_SyncStatusUpdate
{
   //TODO:
};

struct PLT_SyncStatus
{
  PLT_SyncStatusUpdate m_ongoingSyncStatus;
  PLT_SyncStatusUpdate m_lastSyncStatus;

  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;
};

struct PLT_PairGroup
{
public:
  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;
  
  void GenerateUUIDs() 
  { PLT_UPnPMessageHelper::GenerateUUID(16, m_id); }

  NPT_String m_id;
  PLT_OptionalBool m_bActive;
  NPT_UInt32 m_updateID;
  PLT_SyncPolicy m_policy;
};

struct PLT_Partner
{
public:
  PLT_Partner() : m_id(0) {}

  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

  NPT_UInt32 m_id;
  NPT_String m_strDeviceUDN;
  NPT_String m_strServiceID;
protected:
  
};

struct PLT_Partnership
{
public:
  PLT_Partnership() : m_updateID(0) {}
  
  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;
  
  void GenerateUUIDs() 
  {
    PLT_UPnPMessageHelper::GenerateUUID(16, m_id);
    m_pairGroups.Apply(PLT_UUIDGenerator());

    m_partner1.m_id = 1;
    m_partner2.m_id = 2;
  }
  PLT_OptionalBool m_bActive;
  NPT_UInt32 m_updateID;
  PLT_Partner m_partner1;
  PLT_Partner m_partner2;
  PLT_SyncPolicy m_policy;
  NPT_List<PLT_PairGroup> m_pairGroups;
  NPT_String m_id;
};

struct PLT_SyncRelationship
{
public:
  PLT_SyncRelationship() : m_bActive(true), m_updateID(0) {}

  bool IsPartnershipActive(const char* ID) const;
  bool IsPairGroupActive(const char* ID) const;
  bool IsActive() const { return m_bActive.IsTrue(); /* TODO: */ }

  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

  void GenerateUUIDs() 
  {
    PLT_UPnPMessageHelper::GenerateUUID(16, m_id);
    m_partnerships.Apply(PLT_UUIDGenerator());
  }
  
  NPT_Result GetPartners(const NPT_String& localDeviceID, NPT_List<PLT_Partner>& result) const;
  
  NPT_String m_id;
  PLT_OptionalBool m_bActive;
  NPT_String m_strSystemUpdateID;
  NPT_String m_strTitle;
  NPT_UInt32 m_updateID;
  NPT_List<PLT_Partnership> m_partnerships;
};

struct PLT_SyncData
{
  PLT_SyncRelationship m_syncRelationship;
  
  void GenerateUUIDs() { m_syncRelationship.GenerateUUIDs(); }

  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;
};

class PLT_SyncPair
{
public:
  enum STATUS
  {
    NEW,
    MODIFIED,
    DELETED
  };

  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;
private:
  
  NPT_String     m_syncRelationshipID;
  NPT_String     m_partnershipID;
  NPT_String     m_pairGroupID;
  NPT_String     m_remoteObjectID;
  PLT_SyncPolicy m_syncPolicy;
  STATUS         m_status;
};

class PLT_SyncPairs
{
public:
  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;
private:
  NPT_List<PLT_SyncPair> m_pairs;
  NPT_UInt32 m_updateID;


};

class PLT_ResetObjectList
{
public:
  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

private:
    //hmm... not sure if ResetObjectList should actually contain a LIST of objectIDs...
    //specs only say that ObjectID is optional, not that it can occur multiple times
  NPT_String m_objectID_id;
  NPT_String m_objectID_remoteObjID;
  NPT_UInt32 m_objectID_updateID;
};

class PLT_Change
{
  PLT_MediaObjectReference mediaObject;
    //TODO: ChangeLog specific data
};

class PLT_ChangeLog
{
public:
  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

protected:
  NPT_List<PLT_Change> m_changes;
};


class PLT_SyncChangeObserver
{
public:
	PLT_SyncChangeObserver(PLT_StateVariable* stateVariable);
	void OnSyncDataChanged(const PLT_PairGroup& pair_group) { OnSyncDataChanged(pair_group.m_id); }
	void OnSyncDataChanged(const PLT_Partnership& partnership) { OnSyncDataChanged(partnership.m_id); }
	void OnSyncDataChanged(const PLT_SyncRelationship& sync_relationship) { OnSyncDataChanged(sync_relationship.m_id); }
	void OnSyncDataChanged(const NPT_String& GUID);

	void OnSyncDataSynced(const PLT_PairGroup& pair_group) { OnSyncDataSynced(pair_group.m_id); }
	void OnSyncDataSynced(const PLT_Partnership& partnership) { OnSyncDataSynced(partnership.m_id); }
	void OnSyncDataSynced(const PLT_SyncRelationship& sync_relationship) { OnSyncDataSynced(sync_relationship.m_id); }
	void OnSyncDataSynced(const NPT_String& GUID);

  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

private:
	PLT_SyncChangeObserver();

	PLT_StateVariable* m_stateVariable;
	PLT_StringList m_syncDataUpdate;
	PLT_StringList m_syncObjUpdate;
};

struct PLT_SyncStatusInformation
{
	PLT_SyncStatusInformation(NPT_UInt32 total, NPT_UInt32 complete = 0, NPT_UInt32 failed = 0)
		: totalObjects(total), completeObjects(complete), failedObjects(failed)
	{}

	NPT_UInt32 totalObjects;
	NPT_UInt32 completeObjects;
	NPT_UInt32 failedObjects;
};

struct PLT_SyncLogEntry
{
	NPT_String localObjID;
	NPT_String remoteObjID;
	NPT_UInt32 statusCode;

	bool IsError() { return statusCode > 99; }
	bool IsSuccessfull() { return statusCode < 100; }
};

struct PLT_SyncStatusPairGroup
{
	NPT_List<PLT_SyncLogEntry> logEntries;
//	PLT_SyncStatusInformation statusInformation;
	NPT_String id;
};

struct PLT_SyncStatusPartnership
{
//	PLT_SyncStatusInformation statusInformaton;
	NPT_String id;
	PLT_SyncStatusPairGroup child;
};

struct PLT_SyncStatusRelationship
{
	//PLT_SyncStatusInformation statusInformaton;
	NPT_String id;
	PLT_SyncStatusPartnership child;
};

//Responsible for observing the status of current sync operations and pushing changes to the SyncStatusUpdate State Variable
class PLT_SyncStatusObserver
{
public:
	PLT_SyncStatusObserver(PLT_StateVariable* stateVariable); 
	
  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;
private:
	PLT_SyncStatusObserver();

	PLT_StateVariable* m_stateVariable;
	NPT_List<PLT_SyncStatusRelationship> syncStatus;
};

#endif /* _PLT_SYNC_DATA_H_ */
