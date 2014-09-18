#include "PltContentSyncService.h"
#include "PltHttp.h"
#include "ContentSync/PltRelationshipStructures.h"
#include "ContentSync/PltSyncUtils.h"
#include "PltUtilities.h"
#include "PltCtrlPointTask.h"

NPT_SET_LOCAL_LOGGER("platinum.media.server")

/*----------------------------------------------------------------------
|   forward references
+---------------------------------------------------------------------*/
extern NPT_UInt8 MS_ContentSyncServiceSCPD[];

class PLT_SyncDeviceDataFinder
{
public:
  // methods
  PLT_SyncDeviceDataFinder(const char* uuid) : m_UUID(uuid) {}
  virtual ~PLT_SyncDeviceDataFinder() {}

  bool operator()(const PLT_SyncDeviceHolder& data) const {
    return data.device->GetUUID().Compare(m_UUID, true) ? false : true;
  }

private:
  // members
  NPT_String m_UUID;
};

struct PLT_StringPair
{
  PLT_StringPair(const NPT_String& left, const NPT_String& right) : left(left), right(right) {}

  NPT_String left;
  NPT_String right;
};


NPT_Result PLT_ContentSyncCtrlPoint::InvokeAddSyncData(PLT_DeviceDataReference& device,
                                                       const NPT_String& actionCaller,
                                                       const NPT_String& syncID,
                                                       const PLT_SyncData& syncData,
                                                       void *userdata)
{
  PLT_Service* service;
  PLT_ActionReference action;
  NPT_CHECK(device->FindServiceByType("urn:schemas-upnp-org:service:ContentSync:*", service));

  NPT_CHECK_SEVERE(m_CtrlPoint->CreateAction(device,
                                             "urn:schemas-upnp-org:service:ContentSync:1",
                                             "AddSyncData",
                                             action));
  NPT_String strSyncData;
  NPT_CHECK(syncData.ToXml(strSyncData, true));

  NPT_CHECK(action->SetArgumentValue("SyncData", strSyncData));
  NPT_CHECK(action->SetArgumentValue("ActionCaller", actionCaller));
  NPT_CHECK(action->SetArgumentValue("SyncID", syncID));

  return m_CtrlPoint->InvokeAction(action, userdata);
}

NPT_Result PLT_ContentSyncCtrlPoint::InvokeModifySyncData(PLT_DeviceDataReference& device,
                                                          const NPT_String& actionCaller,
                                                          const NPT_String& syncID,
                                                          const PLT_SyncData& syncData)
{
  PLT_Service* service;
  PLT_ActionReference action;
  NPT_CHECK(device->FindServiceByType("urn:schemas-upnp-org:service:ContentSync:*", service));

  NPT_CHECK_SEVERE(m_CtrlPoint->CreateAction(device,
                                             "urn:schemas-upnp-org:service:ContentSync:1",
                                             "ModifySyncData",
                                             action));
  NPT_String strSyncData;
  NPT_CHECK(syncData.ToXml(strSyncData, true));

  NPT_CHECK(action->SetArgumentValue("SyncData", strSyncData));
  NPT_CHECK(action->SetArgumentValue("ActionCaller", actionCaller));
  NPT_CHECK(action->SetArgumentValue("SyncID", syncID));

  return m_CtrlPoint->InvokeAction(action, NULL);
}

NPT_Result PLT_ContentSyncCtrlPoint::InvokeDeleteSyncData(PLT_DeviceDataReference& device,
                                                          const NPT_String& actionCaller,
                                                          const NPT_String& syncID)
{
  if (syncID.IsEmpty())
    return NPT_ERROR_INVALID_PARAMETERS;

  PLT_Service* service;
  PLT_ActionReference action;
  NPT_CHECK(device->FindServiceByType("urn:schemas-upnp-org:service:ContentSync:*", service));

  NPT_CHECK_SEVERE(m_CtrlPoint->CreateAction(device,
                                             "urn:schemas-upnp-org:service:ContentSync:1",
                                             "DeleteSyncData",
                                             action));

  NPT_CHECK(action->SetArgumentValue("ActionCaller", actionCaller));
  NPT_CHECK(action->SetArgumentValue("SyncID", syncID));

  return m_CtrlPoint->InvokeAction(action, NULL);
}

NPT_Result PLT_ContentSyncCtrlPoint::InvokeExchangeSyncData(PLT_DeviceDataReference& device,
                                                            const PLT_SyncData& syncData)
{
  PLT_Service* service;
  PLT_ActionReference action;
  NPT_CHECK(device->FindServiceByType("urn:schemas-upnp-org:service:ContentSync:*", service));

  NPT_CHECK_SEVERE(m_CtrlPoint->CreateAction(device,
                                             "urn:schemas-upnp-org:service:ContentSync:1",
                                             "ExchangeSyncData",
                                             action));

  NPT_String strSyncData;
  NPT_CHECK(syncData.ToXml(strSyncData, false));

  NPT_CHECK(action->SetArgumentValue("LocalSyncData", strSyncData));

  return m_CtrlPoint->InvokeAction(action, NULL);
}

NPT_Result PLT_ContentSyncCtrlPoint::InvokeGetSyncData(PLT_DeviceDataReference& device,
                                                       const NPT_String& syncID)
{
  PLT_Service* service;
  PLT_ActionReference action;
  NPT_CHECK(device->FindServiceByType("urn:schemas-upnp-org:service:ContentSync:*", service));

  NPT_CHECK_SEVERE(m_CtrlPoint->CreateAction(device,
                                             "urn:schemas-upnp-org:service:ContentSync:1",
                                             "GetSyncData",
                                             action));

  NPT_CHECK(action->SetArgumentValue("SyncID", syncID));

  return m_CtrlPoint->InvokeAction(action, NULL);
}

NPT_Result PLT_ContentSyncCtrlPoint::InvokeAddSyncPair(PLT_DeviceDataReference& device,
                                                       const NPT_String& actionCaller,
                                                       const NPT_String& objectID,
                                                       const PLT_SyncPair& syncPair)
{
  PLT_Service* service;
  PLT_ActionReference action;
  NPT_CHECK(device->FindServiceByType("urn:schemas-upnp-org:service:ContentSync:*", service));

  NPT_CHECK_SEVERE(m_CtrlPoint->CreateAction(device,
                                             "urn:schemas-upnp-org:service:ContentSync:1",
                                             "AddSyncPair",
                                             action));

  NPT_CHECK(action->SetArgumentValue("ActionCaller", actionCaller));
  NPT_CHECK(action->SetArgumentValue("ObjectID", objectID));
  NPT_String strSyncPair;
  NPT_CHECK(syncPair.ToXml(strSyncPair));
  NPT_CHECK(action->SetArgumentValue("SyncPair", strSyncPair));

  return m_CtrlPoint->InvokeAction(action, NULL);
}

/*----------------------------------------------------------------------
|   PLT_ContentSyncService::PLT_MediaServer
+---------------------------------------------------------------------*/
PLT_ContentSyncService::PLT_ContentSyncService(PLT_DeviceData* device,
                                               NPT_Reference<PLT_ContentSyncDelegate> delegate,
                                               PLT_CtrlPointReference ctrlPoint)
: PLT_Service(device,
              "urn:schemas-upnp-org:service:ContentSync:1",
              "urn:upnp-org:serviceId:ContentSync",
              "ContentSync"),
  m_CtrlPoint(new PLT_ContentSyncCtrlPoint(ctrlPoint)),
  delegate(delegate)
{
  NPT_ASSERT(delegate.IsNull());
  NPT_ASSERT(ctrlPoint.IsNull());

	this->SetSCPDXML((const char*)MS_ContentSyncServiceSCPD);

	PLT_StateVariable* syncChange = this->FindStateVariable("SyncChange");
	m_syncChangeStateVariable = new PLT_SyncChangeObserver(syncChange);

	this->SetStateVariable(    "SyncStatusUpdate", "");
	this->SetStateVariableRate("SyncStatusUpdate", NPT_TimeInterval(2.));

  device->AddService(this);

  ctrlPoint->AddListener(this);
}

// PLT_CtrlPointListener methods
NPT_Result
PLT_ContentSyncService::OnDeviceAdded(PLT_DeviceDataReference& device)
{
  PLT_Service *serviceCDS;

  NPT_String type = "urn:schemas-upnp-org:service:ContentSync:*";
  if (NPT_FAILED(device->FindServiceByType(type, serviceCDS))) {
    NPT_LOG_WARNING_2("Service %s not found in device \"%s\"",
      type.GetChars(),
      device->GetFriendlyName().GetChars());
    return NPT_FAILURE;
  }
  else {
    // in case it's a newer upnp implementation, force to 1
    serviceCDS->ForceVersion(1);
  }

  {
    NPT_AutoLock lock(m_ContentSyncDevices);

    PLT_SyncDeviceHolder data;
    NPT_String uuid = device->GetUUID();

    NPT_Result deviceFound = NPT_ContainerFind(m_ContentSyncDevices,
                                               PLT_SyncDeviceDataFinder(uuid),
                                               data);
    // is it a old device?
    if (NPT_SUCCEEDED(deviceFound)) {
      NPT_LOG_WARNING_1("Device (%s) is already in our list!", (const char*)uuid);
      return NPT_FAILURE;
    }

    NPT_LOG_FINE_1("Device Found: %s", (const char*)*device);

    PLT_SyncDeviceHolder deviceHolder(device);
    m_ContentSyncDevices.Add(deviceHolder);

    if (NPT_FAILED(deviceFound)) //new device, let's make sure our SyncData is in sync
    {
      PLT_SyncData syncData;
      delegate->OnGetSyncData("", &syncData);
      if (NPT_FAILED(m_CtrlPoint->InvokeExchangeSyncData(device, syncData))) {
        NPT_LOG_WARNING_1("Exchanging Sync Data with device (%s) failed!", (const char*)uuid);
        return NPT_FAILURE;
      }
    }
  }

  // subscribe to required services
  m_CtrlPoint->m_CtrlPoint->Subscribe(serviceCDS);

  return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_MediaController::OnDeviceRemoved
+---------------------------------------------------------------------*/
NPT_Result
PLT_ContentSyncService::OnDeviceRemoved(PLT_DeviceDataReference& device)
{
  {
    NPT_AutoLock lock(m_ContentSyncDevices);

    // only release if we have kept it around
    PLT_SyncDeviceHolder data;
    NPT_String uuid = device->GetUUID();

    // Have we seen that device?
    if (NPT_FAILED(NPT_ContainerFind(m_ContentSyncDevices, PLT_SyncDeviceDataFinder(uuid), data))) {
      NPT_LOG_WARNING_1("Device (%s) not found in our list!", (const char*)uuid);
      return NPT_FAILURE;
    }

    NPT_LOG_FINE_1("Device Removed: %s", (const char*)*device);

    m_ContentSyncDevices.Remove(device);
  }

  return NPT_SUCCESS;
}

NPT_Result
PLT_ContentSyncService::OnEventNotify(PLT_Service* service, NPT_List<PLT_StateVariable*>* vars)
{
  return NPT_SUCCESS;
}

NPT_Result
PLT_ContentSyncService::OnExchangeSyncDataResponse(NPT_Result           res,
                                                   PLT_ActionReference& action,
                                                   void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result
PLT_ContentSyncService::OnAddSyncDataResponse(NPT_Result           res,
                                              PLT_ActionReference& action,
                                              void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result
PLT_ContentSyncService::OnDeleteSyncDataResponse(NPT_Result           res,
                                                 PLT_ActionReference& action,
                                                 void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result
PLT_ContentSyncService::OnModifySyncDataResponse(NPT_Result           res,
                                                 PLT_ActionReference& action,
                                                 void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result
PLT_ContentSyncService::OnGetSyncDataResponse(NPT_Result           res,
                                              PLT_ActionReference& action,
                                              void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result
PLT_ContentSyncService::OnAddSyncPairResponse(NPT_Result           res,
                                              PLT_ActionReference& action,
                                              void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result
PLT_ContentSyncService::OnAddModifyPairResponse(NPT_Result           res,
                                                PLT_ActionReference& action,
                                                void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result
PLT_ContentSyncService::OnDeleteSyncPairResponse(NPT_Result           res,
                                                 PLT_ActionReference& action,
                                                 void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result
PLT_ContentSyncService::OnStartSycResponse(NPT_Result           res,
                                           PLT_ActionReference& action,
                                           void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result
PLT_ContentSyncService::OnAbortSyncResponse(NPT_Result           res,
                                            PLT_ActionReference& action,
                                            void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result
PLT_ContentSyncService::OnGetChangeLogResponse(NPT_Result           res,
                                               PLT_ActionReference& action,
                                               void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result
PLT_ContentSyncService::OnResetChangeLogResponse(NPT_Result           res,
                                                 PLT_ActionReference& action,
                                                 void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result
PLT_ContentSyncService::OnResetStatusResponse(NPT_Result           res,
                                              PLT_ActionReference& action,
                                              void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result
PLT_ContentSyncService::OnGetSyncStatusResponse(NPT_Result           res,
                                                PLT_ActionReference& action,
                                                void*                userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}


NPT_Result
PLT_ContentSyncService::OnActionResponse(NPT_Result           res,
                                         PLT_ActionReference& action,
                                         void*                userdata)
{
  NPT_String actionName = action->GetActionDesc().GetName();
  if (actionName.Compare("ExchangeSyncData", true) == 0) {
    return OnExchangeSyncDataResponse(res, action, userdata);
  }
  if (actionName.Compare("AddSyncData", true) == 0) {
    return OnAddSyncDataResponse(res, action, userdata);
  }
  if (actionName.Compare("ModifySyncData", true) == 0) {
    return OnModifySyncDataResponse(res, action, userdata);
  }
  if (actionName.Compare("DeleteSyncData", true) == 0) {
    return OnDeleteSyncDataResponse(res, action, userdata);
  }
  if (actionName.Compare("GetSyncData", true) == 0) {
    return OnGetSyncDataResponse(res, action, userdata);
  }
  if (actionName.Compare("AddSyncPair", true) == 0) {
    return OnAddSyncPairResponse(res, action, userdata);
  }
  if (actionName.Compare("ModifySyncPair", true) == 0) {
    return OnAddModifyPairResponse(res, action, userdata);
  }
  if (actionName.Compare("DeleteSyncPair", true) == 0) {
    return OnDeleteSyncPairResponse(res, action, userdata);
  }
  if (actionName.Compare("StartSync", true) == 0) {
    return OnStartSycResponse(res, action, userdata);
  }
  if (actionName.Compare("AbortSync", true) == 0) {
    return OnAbortSyncResponse(res, action, userdata);
  }
  if (actionName.Compare("GetChangeLog", true) == 0) {
    return OnGetChangeLogResponse(res, action, userdata);
  }
  if (actionName.Compare("ResetChangeLog", true) == 0) {
    return OnResetChangeLogResponse(res, action, userdata);
  }
  if (actionName.Compare("ResetStatus", true) == 0) {
    return OnResetStatusResponse(res, action, userdata);
  }
  if (actionName.Compare("GetSyncStatus", true) == 0) {
    return OnGetSyncStatusResponse(res, action, userdata);
  }
  return NPT_SUCCESS;
}

/*----------------------------------------------------------------------
|   PLT_MediaServer::OnAction
+---------------------------------------------------------------------*/
NPT_Result
PLT_ContentSyncService::OnAction(PLT_ActionReference&          action,
                                 const PLT_HttpRequestContext& context)
{
  /* parse the action name */
  NPT_String name = action->GetActionDesc().GetName();

  if (name.Compare("AddSyncData", true) == 0) {
    return OnAddSyncData(action, context);
  }
  if (name.Compare("ModifySyncData", true) == 0) {
    return OnModifySyncData(action, context);
  }
  if (name.Compare("DeleteSyncData", true) == 0) {
    return OnDeleteSyncData(action, context);
  }
  if (name.Compare("GetSyncData", true) == 0) {
    return OnGetSyncData(action, context);
  }
  if (name.Compare("ExchangeSyncData", true) == 0) {
    return OnExchangeSyncData(action, context);
  }
  if (name.Compare("AddSyncPair", true) == 0) {
    return OnAddSyncPair(action, context);
  }
  if (name.Compare("ModifySyncPair", true) == 0) {
    return OnModifySyncPair(action, context);
  }
  if (name.Compare("DeleteSyncPair", true) == 0) {
    return OnDeleteSyncPair(action, context);
  }
  if (name.Compare("StartSync", true) == 0) {
    return OnStartSync(action, context);
  }
  if (name.Compare("AbortSync", true) == 0) {
    return OnAbortSync(action, context);
  }
  if (name.Compare("GetChangeLog", true) == 0) {
    return OnGetChangeLog(action, context);
  }
  if (name.Compare("ResetChangeLog", true) == 0) {
    return OnResetChangeLog(action, context);
  }
  if (name.Compare("ResetStatus", true) == 0) {
    return OnResetStatus(action, context);
  }
  if (name.Compare("GetSyncStatus", true) == 0) {
    return OnGetSyncStatus(action, context);
  }

  return NPT_FAILURE;
}

NPT_Result PLT_ContentSyncService::OnAddSyncData(PLT_ActionReference&          action,
                                                 const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::OnModifySyncData(PLT_ActionReference&          action,
                                                    const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::OnDeleteSyncData(PLT_ActionReference&          action,
                                                    const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::OnGetSyncData(PLT_ActionReference&          action,
                                                 const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::CompareSyncStructures(NPT_List<PLT_SyncStructureRef>::Iterator ourRelationship,
                                                          const PLT_SyncData& remoteSyncData)
{ 
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::OnExchangeSyncData(PLT_ActionReference&          action,
                                                      const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::OnAddSyncPair(PLT_ActionReference&          action,
                                                 const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::OnModifySyncPair(PLT_ActionReference&          action,
                                                    const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::OnDeleteSyncPair(PLT_ActionReference&          action,
                                                    const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::OnStartSync(PLT_ActionReference&          action,
                                               const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::OnAbortSync(PLT_ActionReference&          action,
                                               const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::OnGetChangeLog(PLT_ActionReference&          action,
                                                  const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::OnResetChangeLog(PLT_ActionReference&          action,
                                                    const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::OnResetStatus(PLT_ActionReference&          action,
                                                 const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ContentSyncService::OnGetSyncStatus(PLT_ActionReference&          action,
                                                   const PLT_HttpRequestContext& context)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}
