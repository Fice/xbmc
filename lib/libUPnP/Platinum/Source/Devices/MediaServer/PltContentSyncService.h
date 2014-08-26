/*****************************************************************
|
|   Platinum - AV Media Server Device
|
| Copyright (c) 2014, Marcel Lambert
| All rights reserved.
|
| This program is free software; you can redistribute it and/or
| modify it under the terms of the GNU General Public License
| as published by the Free Software Foundation; either version 2
| of the License, or (at your option) any later version.
|
| This program is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY; without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
| GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License
| along with this program; see the file LICENSE.txt. If not, write to
| the Free Software Foundation, Inc., 
| 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
| http://www.gnu.org/licenses/gpl-2.0.html
|
****************************************************************/


#ifndef _PLT_CONTENT_SYNC_SERVICE_H_
#define _PLT_CONTENT_SYNC_SERVICE_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"
#include "ContentSync/PltSyncData.h"
#include "PltAction.h"
#include "PltService.h"
#include "PltCtrlPoint.h"
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

    virtual NPT_Result OnAddSyncData(const NPT_String&   ActionCaller,
                                     const NPT_String&   SyncID,                 /*A_ARG_TYPE_SyncID*/
                                     const PLT_SyncData& SyncData,
                                     PLT_SyncData*       SyncDataResult /* OUT */) = 0;
    virtual NPT_Result OnModifySyncData(const NPT_String&   ActionCaller,
                                        const NPT_String&   SyncID,              /*A_ARG_TYPE_SyncID*/
                                        const PLT_SyncData& SyncData) = 0;
    virtual NPT_Result OnDeleteSyncData(const NPT_String&  ActionCaller,
                                        const NPT_String&  SyncID) = 0;
    //OUT: A_ARG_TYPE_SyncData SyncData
    virtual NPT_Result OnGetSyncData(const NPT_String& SyncID,                 /*A_ARG_TYPE_SyncID*/
                                     PLT_SyncData*     SyncData      /* OUT*/) = 0;
    //OUT: A_ARG_TYPE_SyncData RemoteSyncData
    virtual NPT_Result OnExchangeSyncData(const PLT_SyncData& LocalSyncData,
                                          PLT_SyncData*       RemoteSyncData /* OUT */) = 0;
    virtual NPT_Result OnAddSyncPair(const NPT_String&    ActionCaller,
                                     const NPT_String&    ObjectID,          /*A_ARG_TYPE_ObjectID*/
                                     const PLT_SyncPairs& SyncPair) = 0;
    virtual NPT_Result OnModifySyncPair(const NPT_String&    ActionCaller,
                                        const NPT_String&    ObjectID,       /*A_ARG_TYPE_ObjectID*/
                                        const PLT_SyncPairs& SyncPair) = 0;
    virtual NPT_Result OnDeleteSyncPair(const NPT_String& ActionCaller,
                                        const NPT_String& ObjectID,      /*A_ARG_TYPE_ObjectID*/
                                        const NPT_String& SyncID         /*A_ARG_TYPE_SyncID*/) = 0;
    virtual NPT_Result OnStartSync(const NPT_String&  ActionCaller,
                                   const NPT_String&  SyncID                 /*A_ARG_TYPE_SyncID*/) = 0;
    virtual NPT_Result OnAbortSync(const NPT_String&  ActionCaller,
                                   const NPT_String&  SyncID                 /*A_ARG_TYPE_SyncID*/) = 0;
    //OUT: A_ARG_TYPE_COUNT NumberReturned
    //OUT: A_ARG_TYPE_Count TotalMatches
    virtual NPT_Result OnGetChangeLog(const NPT_String& SyncID,         /*A_ARG_TYPE_SyncID*/
                                      const NPT_UInt32  StartingIndex,  /*A_ARG_TYPE_Index*/
                                      const NPT_UInt32  RequestedCount, /*A_ARG_TYPE_Count*/
                                      PLT_ChangeLog*    Result,         /* OUT */
                                      NPT_UInt32&       NumberReturned, /* OUT */
                                      NPT_UInt32&       TotalMatches    /* OUT */) = 0;
    virtual NPT_Result OnResetChangeLog(const NPT_String&          SyncID,   /*A_ARG_TYPE_SyncID*/
                                        const PLT_ResetObjectList& ObjectIDs) = 0;
    virtual NPT_Result OnResetStatus(const NPT_String& SyncID                 /*A_ARG_TYPE_SyncID*/) = 0;
    //OUT: A_ARG_TYPE_SyncStatus
    virtual NPT_Result OnGetSyncStatus(const NPT_String& SyncID, /*A_ARG_TYPE_SyncID*/
                                       PLT_SyncStatus*   SyncStatus /* OUT */) { return NPT_ERROR_NOT_IMPLEMENTED; }
};

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

typedef NPT_List<PLT_SyncDeviceHolder> PLT_SyncDeviceDataReferenceList;

/* A nice thin interface that makes invoking the Sync Service Actions easier*/
class PLT_ContentSyncCtrlPoint
{
public:
  PLT_ContentSyncCtrlPoint(PLT_CtrlPointReference ctrlPoint) : m_CtrlPoint(ctrlPoint) {}

  virtual NPT_Result InvokeAddSyncData(PLT_DeviceDataReference& device,
                                       const NPT_String& actionCaller, 
                                       const NPT_String& syncID, 
                                       const PLT_SyncData& syncData, 
                                       PLT_SyncData& syncDataResult);
  
protected:
  PLT_CtrlPointReference m_CtrlPoint;
};

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
private:
  PLT_CtrlPointReference m_CtrlPoint;

	PLT_ContentSyncService();
	NPT_Reference<PLT_ContentSyncDelegate> delegate;


  NPT_Reference<PLT_SyncChangeObserver>     m_syncChangeStateVariable;
  NPT_Lock<PLT_SyncDeviceDataReferenceList> m_ContentSyncDevices;
};

#endif /* _PLT_CONTENT_SYNC_SERVICE_H_ */
