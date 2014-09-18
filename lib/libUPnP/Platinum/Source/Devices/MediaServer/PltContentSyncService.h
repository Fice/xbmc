#ifndef _PLT_CONTENT_SYNC_SERVICE_H_
#define _PLT_CONTENT_SYNC_SERVICE_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"
#include "ContentSync/PltRelationshipStructures.h"
#include "ContentSync/PltSyncUtils.h"
#include "ContentSync/PltSyncPair.h"
#include "ContentSync/PltSyncChangelog.h"
#include "ContentSync/PltSyncValueObservers.h"
#include "PltAction.h"
#include "PltService.h"
#include "PltCtrlPoint.h"

struct PLT_SyncDeviceHolder {
  PLT_SyncDeviceHolder(PLT_DeviceDataReference& device) : device(device), syncDataExchanged(false) {}
  PLT_SyncDeviceHolder() : device(PLT_DeviceDataReference()), syncDataExchanged(false) {}
  PLT_DeviceDataReference device;

  PLT_SyncDeviceHolder& operator=(const PLT_SyncDeviceHolder& rhs)
  {
    this->device = rhs.device;
    this->syncDataExchanged = rhs.syncDataExchanged;
    return *this;
  }
  bool operator==(const PLT_SyncDeviceHolder& rhs)
  {
    return this->device == rhs.device &&
    this->syncDataExchanged == rhs.syncDataExchanged;
  }

  void OnSyncDataExchanged() { syncDataExchanged = true; }
  bool IsSyncDataExchanged() const { return syncDataExchanged; }
protected:
  bool syncDataExchanged;
};

/*----------------------------------------------------------------------
|   PLT_ContentSyncDelegate
+-----------------------------------------------------------------------
 The PLT_ContentSyncDelegate class is an interface for delegating the handling
 of the required UPnP Content Sync service actions.
 */
class PLT_ContentSyncDelegate
{
public:
    PLT_ContentSyncDelegate() {}
    virtual ~PLT_ContentSyncDelegate() {}

    ////////////////////////////////////////
    // Actual Actions
    ////////////////////////////////////////

    /*
    Gets called when a new SyncRelationship or SyncPartnership gets added
    @param SyncID - the ID of the current Sync Operation ?!?
    @param SyncData - the sync data that needs to be created
    */
    virtual NPT_Result OnAddSyncData(const NPT_String&   SyncID,
                                     const PLT_SyncData& SyncData) = 0;
    virtual NPT_Result OnModifySyncData(const NPT_String&   SyncID,
                                        const PLT_SyncData& SyncData) = 0;
    virtual NPT_Result OnDeleteSyncData(const NPT_String&  SyncID) = 0;
    virtual NPT_Result OnGetSyncData(const NPT_String& SyncID,
                                     PLT_SyncData*     SyncData) = 0;
    virtual NPT_Result OnExchangeSyncData(const PLT_SyncData& remoteSyncData) = 0;
    virtual NPT_Result OnAddSyncPair(const NPT_String&    ObjectID,
                                     const PLT_SyncPair& SyncPair,
                                     NPT_String& newObjectID) = 0;
    virtual NPT_Result OnModifySyncPair(const NPT_String&    ObjectID,
                                        const PLT_SyncPairs& SyncPair) = 0;
    virtual NPT_Result OnDeleteSyncPair(const NPT_String& ObjectID,
                                        const NPT_String& SyncID) = 0;
    virtual NPT_Result OnStartSync(const NPT_String&  SyncID) = 0;
    virtual NPT_Result OnAbortSync(const NPT_String&  SyncID) = 0;
    virtual NPT_Result OnGetChangeLog(const NPT_String& SyncID,
                                      const NPT_UInt32  StartingIndex,
                                      const NPT_UInt32  RequestedCount,
                                      PLT_ChangeLog*    Result,
                                      NPT_UInt32&       NumberReturned,
                                      NPT_UInt32&       TotalMatches) = 0;
    virtual NPT_Result OnResetChangeLog(const NPT_String&          SyncID,
                                        const PLT_ResetObjectList& ObjectIDs) = 0;
    virtual NPT_Result OnResetStatus(const NPT_String& SyncID) = 0;
    virtual NPT_Result OnGetSyncStatus(const NPT_String& SyncID,
                                       PLT_SyncStatus*   SyncStatus) { return NPT_ERROR_NOT_IMPLEMENTED; }

    ////////////////////////////////////
    //System status functions
    ////////////////////////////////////
    virtual NPT_Result GetSyncStructure(const NPT_String& SyncID,
                                        PLT_SyncStructureRef& SyncStructure) = 0;
    virtual NPT_Result CanSyncNow(const NPT_String& SyncID) = 0;

    ///////////////////////////////////
    //Data retrieval functions
    ///////////////////////////////////
    //should fail, if syncDataID is invalid
    //should only return partners that are in an active syncdata structure
    virtual bool HasSyncPair(PLT_SyncData& syncData, NPT_String objectID) = 0;
};

typedef NPT_List<PLT_SyncDeviceHolder> PLT_SyncDeviceDataReferenceList;

/* A nice thin interface that makes invoking the Sync Service Actions easier*/
class PLT_ContentSyncCtrlPoint
{
public:
  PLT_ContentSyncCtrlPoint(PLT_CtrlPointReference ctrlPoint) : m_CtrlPoint(ctrlPoint) {}

  NPT_Result InvokeAddSyncData(PLT_DeviceDataReference& device,
                               const NPT_String& actionCaller,
                               const NPT_String& syncID,
                               const PLT_SyncData& syncData,
                               void *userdata);
  NPT_Result InvokeModifySyncData(PLT_DeviceDataReference& device,
                                  const NPT_String& actionCaller,
                                  const NPT_String& syncID,
                                  const PLT_SyncData& syncData);
  NPT_Result InvokeDeleteSyncData(PLT_DeviceDataReference& device,
                                  const NPT_String& actionCaller,
                                  const NPT_String& syncID);
  NPT_Result InvokeExchangeSyncData(PLT_DeviceDataReference& device,
                                    const PLT_SyncData& syncData);
  NPT_Result InvokeGetSyncData(PLT_DeviceDataReference& device,
                               const NPT_String& syncID);
  NPT_Result InvokeAddSyncPair(PLT_DeviceDataReference& device,
                               const NPT_String& actionCaller,
                               const NPT_String& objectID,
                              const PLT_SyncPair& syncPair);

  PLT_CtrlPointReference m_CtrlPoint;
};
typedef NPT_Reference<PLT_ContentSyncCtrlPoint> PLT_ContentSyncCtrlPointReference;

/*----------------------------------------------------------------------
|   PLT_MediaServer
+---------------------------------------------------------------------*/
/**
 The PLT_MediaServer class implements the base class for a UPnP AV
 Media Server device.
 */
class PLT_ContentSyncService : public PLT_Service,
                               public PLT_CtrlPointListener
{
public:
    // constructor
  PLT_ContentSyncService(PLT_DeviceData* device, NPT_Reference<PLT_ContentSyncDelegate> delegate, PLT_CtrlPointReference ctrlPoint);
	virtual ~PLT_ContentSyncService() {}

  // PLT_CtrlPointListener methods
  virtual NPT_Result OnDeviceAdded(PLT_DeviceDataReference& device);
  virtual NPT_Result OnDeviceRemoved(PLT_DeviceDataReference& device);
  virtual NPT_Result OnActionResponse(NPT_Result res, PLT_ActionReference& action, void* userdata);
  virtual NPT_Result OnEventNotify(PLT_Service* service, NPT_List<PLT_StateVariable*>* vars);

  virtual NPT_Result OnAction(PLT_ActionReference&          action,
                              const PLT_HttpRequestContext& context);

protected:
  //Sync Service Functions
  virtual NPT_Result OnAddSyncData(PLT_ActionReference&          action,
                                   const PLT_HttpRequestContext& context);
  virtual NPT_Result OnModifySyncData(PLT_ActionReference&          action,
                                      const PLT_HttpRequestContext& context);
  virtual NPT_Result OnDeleteSyncData(PLT_ActionReference&          action,
                                      const PLT_HttpRequestContext& context);
  virtual NPT_Result OnGetSyncData(PLT_ActionReference&          action,
                                   const PLT_HttpRequestContext& context);
  virtual NPT_Result OnExchangeSyncData(PLT_ActionReference&          action,
                                        const PLT_HttpRequestContext& context);
  virtual NPT_Result OnAddSyncPair(PLT_ActionReference&          action,
                                   const PLT_HttpRequestContext& context);
  virtual NPT_Result OnModifySyncPair(PLT_ActionReference&          action,
                                      const PLT_HttpRequestContext& context);
  virtual NPT_Result OnDeleteSyncPair(PLT_ActionReference&          action,
                                      const PLT_HttpRequestContext& context);
  virtual NPT_Result OnStartSync(PLT_ActionReference&          action,
                                 const PLT_HttpRequestContext& context);
  virtual NPT_Result OnAbortSync(PLT_ActionReference&          action,
                                 const PLT_HttpRequestContext& context);
  virtual NPT_Result OnGetChangeLog(PLT_ActionReference&          action,
                                    const PLT_HttpRequestContext& context);
  virtual NPT_Result OnResetChangeLog(PLT_ActionReference&          action,
                                      const PLT_HttpRequestContext& context);
  virtual NPT_Result OnResetStatus(PLT_ActionReference&          action,
                                   const PLT_HttpRequestContext& context);
  virtual NPT_Result OnGetSyncStatus(PLT_ActionReference&          action,
                                     const PLT_HttpRequestContext& context);
  
  //handling of the results of sync service actions
  virtual NPT_Result OnExchangeSyncDataResponse(NPT_Result           res,
                                                PLT_ActionReference& action,
                                                void*                userdata);
  virtual NPT_Result OnAddSyncDataResponse(NPT_Result           res,
                                           PLT_ActionReference& action,
                                           void*                userdata);
  virtual NPT_Result OnModifySyncDataResponse(NPT_Result           res,
                                              PLT_ActionReference& action,
                                              void*                userdata);
  virtual NPT_Result OnDeleteSyncDataResponse(NPT_Result           res,
                                              PLT_ActionReference& action,
                                              void*                userdata);
  virtual NPT_Result OnGetSyncDataResponse(NPT_Result           res,
                                           PLT_ActionReference& action,
                                           void*                userdata);
  virtual NPT_Result OnAddSyncPairResponse(NPT_Result           res,
                                           PLT_ActionReference& action,
                                           void*                userdata);
  virtual NPT_Result OnAddModifyPairResponse(NPT_Result           res,
                                             PLT_ActionReference& action,
                                             void*                userdata);
  virtual NPT_Result OnDeleteSyncPairResponse(NPT_Result           res,
                                              PLT_ActionReference& action,
                                              void*                userdata);
  virtual NPT_Result OnStartSycResponse(NPT_Result           res,
                                        PLT_ActionReference& action,
                                        void*                userdata);
  virtual NPT_Result OnAbortSyncResponse(NPT_Result           res,
                                         PLT_ActionReference& action,
                                         void*                userdata);
  virtual NPT_Result OnGetChangeLogResponse(NPT_Result           res,
                                            PLT_ActionReference& action,
                                            void*                userdata);
  virtual NPT_Result OnResetChangeLogResponse(NPT_Result           res,
                                              PLT_ActionReference& action,
                                              void*                userdata);
  virtual NPT_Result OnResetStatusResponse(NPT_Result           res,
                                           PLT_ActionReference& action,
                                           void*                userdata);
  virtual NPT_Result OnGetSyncStatusResponse(NPT_Result           res,
                                             PLT_ActionReference& action,
                                             void*                userdata);

private:
  NPT_Result CompareSyncStructures(NPT_List<PLT_SyncStructureRef>::Iterator ourRelationship,
                                   const PLT_SyncData& remoteSyncData);

  NPT_List<NPT_String> currentSyncIDs;
  PLT_ContentSyncCtrlPointReference m_CtrlPoint;

	PLT_ContentSyncService();
	NPT_Reference<PLT_ContentSyncDelegate> delegate;

  NPT_Reference<PLT_SyncChangeObserver>     m_syncChangeStateVariable;
  NPT_Lock<PLT_SyncDeviceDataReferenceList> m_ContentSyncDevices;
};

#endif /* _PLT_CONTENT_SYNC_SERVICE_H_ */
