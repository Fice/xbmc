#pragma once

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"
#include "PltStateVariable.h"
#include "PltSyncStatus.h"

class PLT_SyncStructure;

typedef NPT_List<NPT_String> PLT_StringList;

class PLT_SyncChangeObserver
{
public:
	PLT_SyncChangeObserver(PLT_StateVariable* stateVariable);
  void OnSyncDataChanged(const PLT_SyncStructure* const sync);
	void OnSyncDataChanged(const NPT_String& GUID);

  void OnSyncDataSynced(const PLT_SyncStructure* const sync);
	void OnSyncDataSynced(const NPT_String& GUID);

  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

private:
	PLT_SyncChangeObserver();

	PLT_StateVariable* m_stateVariable;
	PLT_StringList m_syncDataUpdate;
	PLT_StringList m_syncObjUpdate;
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
	NPT_List<NPT_Reference<PLT_SyncStatusRelationship> > syncStatus;
};
