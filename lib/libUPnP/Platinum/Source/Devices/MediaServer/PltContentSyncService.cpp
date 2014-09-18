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

NPT_Result WaitForAllResult(PLT_SyncActionDataList::Iterator &iter)
{
  while (iter) //TODO: at some point we probably want to abort and assume the device has timed out (Error Code: 704)
  {
    if(iter->finished==true)
    {
      if(iter->action->GetErrorCode()!=0)
        return NPT_ERROR_INTERNAL; //that one failed!

      ++iter;
    }
    else
      NPT_System::Sleep(NPT_TimeInterval(0.0));
  }
  return NPT_SUCCESS;
}

NPT_Result InvokeAction(PLT_SyncActionDataList::Iterator &iter, const NPT_String& serviceType, const NPT_String& actionName, NPT_Array<PLT_StringPair>& values, PLT_CtrlPointReference& ctrlPoint)
{
  while (iter)
  {
    NPT_CHECK(ctrlPoint->CreateAction(iter->device.device,
                                        serviceType,
                                        actionName,
                                        iter->action));
    for(NPT_Cardinal i=0;i<values.GetItemCount(); ++i)
    {
      NPT_CHECK(iter->action->SetArgumentValue(values[i].left, values[i].right));
    }

    iter->finished = false;
    NPT_CHECK(ctrlPoint->InvokeAction(iter->action, &*iter));
    ++iter;
  }
  return NPT_SUCCESS;
}

NPT_Result PLT_ContentSyncService::GetPartners(NPT_List<PLT_Partner>::Iterator item, PLT_SyncActionDataList& result, const NPT_String& deviceUUID)
{
  while(item)
  {
    PLT_SyncActionUserData currentActionData;
    if (deviceUUID != item->m_strDeviceUDN)
    {
      if (NPT_FAILED(NPT_ContainerFind(m_ContentSyncDevices,
                                       PLT_SyncDeviceDataFinder(item->m_strDeviceUDN), currentActionData.device)))
        return NPT_ERROR_INTERNAL;
      result.Add(currentActionData);
    }
    ++item;
  }
  if(result.GetItemCount()==0)
    return NPT_ERROR_INTERNAL;
  return NPT_SUCCESS;
}

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
PLT_ContentSyncService::StopWaiting(PLT_ActionReference& action,
                                    void*                userdata)
{
  NPT_String actionCaller;
  action->GetArgumentValue("ActionCaller", actionCaller);
  
  if(userdata && !actionCaller.IsEmpty())
  {
    PLT_SyncActionUserData* data = (PLT_SyncActionUserData*)userdata;
    data->action->SetError(action->GetErrorCode(), action->GetError());
    data->finished = true;
  }
  return NPT_SUCCESS;
}

NPT_Result
PLT_ContentSyncService::OnExchangeSyncDataResponse(NPT_Result           res,
                                                   PLT_ActionReference& action,
                                                   void*                userdata)
{
  if (action->GetErrorCode() >= 100)
  {
    NPT_LOG_WARNING_2("ExchangeSyncData failed with error (code: %i) - %s", action->GetErrorCode(), action->GetError());
    return NPT_SUCCESS;
  }
  
  NPT_String    strRemoteSyncData;
  PLT_SyncData  localSyncData;
  PLT_SyncData  remoteSyncData;
  
  if (NPT_FAILED(action->GetArgumentValue("RemoteSyncData", strRemoteSyncData)))
  {
    NPT_LOG_WARNING("Missing result arguments");
    return NPT_ERROR_INTERNAL;
  }
  
  NPT_XmlElementNode* remoteSyncNode;
  NPT_CHECK(PLT_XmlHelper::Parse(strRemoteSyncData, remoteSyncNode));
  NPT_CHECK(remoteSyncData.FromXml(remoteSyncNode, false));
  
    //we have the partners sync structure, let's get our own
  NPT_CHECK(delegate->OnGetSyncData("", &localSyncData));
  if (localSyncData.m_syncData.GetItemCount() == 0)
    return NPT_SUCCESS; //nothing to do
  
    //compare the sync structures and remove or modify them as neccessary.
  NPT_List<PLT_SyncStructureRef>::Iterator relationship = localSyncData.m_syncData.GetFirstItem();
  NPT_CHECK(CompareSyncStructures(relationship, remoteSyncData));
  
    //We have dealt with everything, maybe the app wants to do some app specific stuff?
  delegate->OnExchangeSyncData(remoteSyncData);
  
  return NPT_SUCCESS;
}

NPT_Result
PLT_ContentSyncService::OnAddSyncDataResponse(NPT_Result           res,
                                              PLT_ActionReference& action,
                                              void*                userdata)
{
  return StopWaiting(action, userdata);
}

NPT_Result
PLT_ContentSyncService::OnDeleteSyncDataResponse(NPT_Result           res,
                                                 PLT_ActionReference& action,
                                                 void*                userdata)
{
  return StopWaiting(action, userdata);
}

NPT_Result
PLT_ContentSyncService::OnModifySyncDataResponse(NPT_Result           res,
                                                 PLT_ActionReference& action,
                                                 void*                userdata)
{
  return StopWaiting(action, userdata);
}

NPT_Result
PLT_ContentSyncService::OnGetSyncDataResponse(NPT_Result           res,
                                              PLT_ActionReference& action,
                                              void*                userdata)
{
  return NPT_SUCCESS;
}

NPT_Result
PLT_ContentSyncService::OnAddSyncPairResponse(NPT_Result           res,
                                              PLT_ActionReference& action,
                                              void*                userdata)
{
  return StopWaiting(action, userdata);
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
  return StopWaiting(action, userdata);
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
  NPT_String    strActionCaller;
  NPT_String    strSyncID;
  NPT_String    strSyncData;
  PLT_SyncData  syncData;

  if (NPT_FAILED(action->GetArgumentValue("ActionCaller", strActionCaller)) ||
      NPT_FAILED(action->GetArgumentValue("SyncID", strSyncID)) ||
      NPT_FAILED(action->GetArgumentValue("SyncData", strSyncData))) {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  NPT_XmlElementNode* syncNode;
  if (NPT_FAILED(PLT_XmlHelper::Parse(strSyncData, syncNode)))
  {
    action->SetError(702, "Invalid XML");
    return NPT_SUCCESS;
  }

  if (NPT_FAILED(syncData.FromXml(syncNode, true))) //xml parsing failed
  {
    action->SetError(702, "Invalid XML");
    return NPT_SUCCESS;
  }

  if (strSyncID.IsEmpty())
  {
    if ((*syncData.m_syncData.GetFirstItem())->GetType()!=SYNC_RELATIONSHIP)
    { //we dont have the syncID of the parent. So it has to be a sync relationship
      action->SetError(402, "Invalid args");
      return NPT_SUCCESS;
    }
  }
  else
  {
    if ((*syncData.m_syncData.GetFirstItem())->GetType()==PARTNERSHIP)
    { //empty sync relationships aren't allowed.
      //that means the sync relationship already has a partnership.
      //Theoratically a sync relationships can have one or more partnerships BUT version 1 (and that the only one I know of)
      //doesn't allo multiple partnerships... (Although the spec allows to call 'AddSyncPartner' with a partnership... That just doesn't make any sense ;))
      action->SetError(402, "Invalid args");
      return NPT_SUCCESS;
    }
    if ((*syncData.m_syncData.GetFirstItem())->GetType()==SYNC_RELATIONSHIP)
    { //a sync relationship is the root element, so it cannot have a parent syncID
      action->SetError(402, "Invalid args");
      return NPT_SUCCESS;
    }
  }

  //If the action caller is not set, means a Control Point wants us to create a Sync Relationship with a third device.
  // That means we need to make sure, that all required partners are online
  PLT_DeviceData* device = GetDevice();
  PLT_SyncActionDataList partnersToSync; //Will hold an array of all devices we have to tell about the new sync data!
  if(strActionCaller.IsEmpty())
  {
    NPT_List<PLT_Partner> partners;
    syncData.GenerateUUIDs();
    syncData.GetPartners(partners);

    if (NPT_FAILED(GetPartners(partners.GetFirstItem(), partnersToSync, device->GetUUID())))
    {
      action->SetError(705, "Partner not online");
      return NPT_SUCCESS;
    }
  }

  //Make sure our app actualy adds the sync data
  NPT_CHECK_SEVERE(delegate->OnAddSyncData(strSyncID, syncData));
  NPT_String strNewID = (*syncData.m_syncData.GetFirstItem())->GetID();

  NPT_LOG_INFO_1("Create a new sync structure with the id: %s", strNewID);

  NPT_Result result = NPT_ERROR_INTERNAL; //In case we fail now, we goto cleanup; and that part will return our 'result' var. so set it to some generic error.

  PLT_SyncActionDataList::Iterator currentDevice = partnersToSync.GetFirstItem();

  //When we are called without a actionCaller, we generated IDs for each new structure.
  //Also, delegate->OnAddSyncData might have done sth... so let's create the SyncData xml again, that we pass to our partners
  NPT_String strSyncDataResult;
  NPT_Array<PLT_StringPair> arguments;
  NPT_CHECK_LABEL(syncData.ToXml(strSyncDataResult, true), cleanup);

  NPT_CHECK_LABEL(arguments.Add(PLT_StringPair("ActionCaller", this->GetDevice()->GetUUID())), cleanup);
  NPT_CHECK_LABEL(arguments.Add(PLT_StringPair("SyncID", strSyncID)), cleanup);
  NPT_CHECK_LABEL(arguments.Add(PLT_StringPair("SyncData", strSyncDataResult)), cleanup);

  //Now we have to push the new syncData to all partners
  NPT_CHECK_LABEL(InvokeAction(currentDevice, "urn:schemas-upnp-org:service:ContentSync:1", "AddSyncData", arguments, m_CtrlPoint->m_CtrlPoint), cleanup);

  //UPnP spec says, we are only allowed to return, once all our partners are finished... let's wait
  currentDevice = partnersToSync.GetFirstItem();
  NPT_CHECK_LABEL(WaitForAllResult(currentDevice), cleanup_others);

  //Set our result value!
  NPT_CHECK_LABEL(action->SetArgumentValue("SyncDataResult", strSyncDataResult), cleanup_others);

  return NPT_SUCCESS;
cleanup_others:
  //TODO: do the cleanup in anther thread... so we can return here!

  //Atleast one device wasn't able to add the new sync structure... so let's remove it from everyone who did!
  currentDevice  = partnersToSync.GetFirstItem();
  while(currentDevice)
  {
    if (currentDevice->action->GetErrorCode()==0) //that one created the sync data, so let's revert it
    {
      PLT_ActionReference action;
      if (NPT_SUCCEEDED(m_CtrlPoint->m_CtrlPoint->CreateAction(currentDevice->device.device,
                                                               "urn:schemas-upnp-org:service:ContentSync:1",
                                                               "RemoveSyncData",
                                                               action)))
      {
        if (NPT_SUCCEEDED(action->SetArgumentValue("SyncID", strNewID)))
          m_CtrlPoint->m_CtrlPoint->InvokeAction(action, NULL);
      }
    }
    ++currentDevice;
  }
cleanup:
  delegate->OnDeleteSyncData(strNewID);

  return result;
}

NPT_Result PLT_ContentSyncService::OnModifySyncData(PLT_ActionReference&          action,
                                                    const PLT_HttpRequestContext& context)
{
  NPT_String    strActionCaller;
  NPT_String    strSyncID;
  NPT_String    strSyncData;
  PLT_SyncData  syncData;

  if (NPT_FAILED(action->GetArgumentValue("ActionCaller", strActionCaller)) ||
      NPT_FAILED(action->GetArgumentValue("SyncID", strSyncID)) ||
      NPT_FAILED(action->GetArgumentValue("SyncData", strSyncData))) {
      NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  if (strSyncID.IsEmpty())
  {
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  NPT_XmlElementNode* syncNode;
  if (NPT_FAILED(PLT_XmlHelper::Parse(strSyncData, syncNode)))
  {
    action->SetError(702, "Invalid XML");
    return NPT_SUCCESS;
  }

  if (NPT_FAILED(syncData.FromXml(syncNode, true))) //xml parsing failed
  {
    action->SetError(702, "Invalid XML");
    return NPT_SUCCESS;
  }

  PLT_SyncStructureRef syncStructure = *syncData.m_syncData.GetFirstItem();
  if (syncStructure->GetID() != strSyncID)
  { //the sync id should be the id of the sync data that needs to be modified.
    //I'm not sure if you can pass a SyncRelationship and the ID of the sub SyncPartnership... right now, just disallow this (easier)
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  PLT_SyncStructureRef localSyncStructure;
  if (NPT_FAILED(delegate->GetSyncStructure(strSyncID, localSyncStructure)))
  {
    action->SetError(701, "No such sync data");
    return NPT_SUCCESS;
  }
  if (localSyncStructure->GetUpdateID() >= syncStructure->GetUpdateID())
  {
    //our partner had a old sync structure
    //ABORT
    //TODO: right now the user has to wait until an exchange sync operation has finished until he can modify the sync structure again...
    //maybe we should trigger one in here
    return NPT_ERROR_INTERNAL;
  }

  PLT_DeviceData* device = GetDevice();

  PLT_SyncActionDataList partnersToSync; //Will hold an array of all devices we have to tell about the new sync data!
  if (strActionCaller.IsEmpty())
  {
    NPT_List<PLT_Partner> partners;
    syncData.GetPartners(partners);

    if (NPT_FAILED(GetPartners(partners.GetFirstItem(), partnersToSync, device->GetUUID())))
    {
      action->SetError(705, "Partner not online");
      return NPT_SUCCESS;
    }
  }

  NPT_Array<PLT_StringPair> arguments;
  NPT_CHECK(arguments.Add(PLT_StringPair("ActionCaller", this->GetDevice()->GetUUID())));
  NPT_CHECK(arguments.Add(PLT_StringPair("SyncID", strSyncID)));
  NPT_CHECK(arguments.Add(PLT_StringPair("SyncData", strSyncData)));

  NPT_CHECK(delegate->OnModifySyncData(strSyncID, syncData));

  PLT_SyncActionDataList::Iterator currentDevice = partnersToSync.GetFirstItem();
  NPT_CHECK(InvokeAction(currentDevice, "urn:schemas-upnp-org:service:ContentSync:1", "ModifySyncData", arguments, m_CtrlPoint->m_CtrlPoint));

  currentDevice = partnersToSync.GetFirstItem();
  return WaitForAllResult(currentDevice);
}

NPT_Result PLT_ContentSyncService::OnDeleteSyncData(PLT_ActionReference&          action,
                                                    const PLT_HttpRequestContext& context)
{
  NPT_String    strActionCaller;
  NPT_String    strSyncID;

  if (NPT_FAILED(action->GetArgumentValue("ActionCaller", strActionCaller)) ||
      NPT_FAILED(action->GetArgumentValue("SyncID", strSyncID))) {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }
  if (strSyncID.IsEmpty())
  {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  PLT_DeviceData* device = GetDevice();
  PLT_SyncData syncData;
  if(NPT_FAILED(delegate->OnGetSyncData(strSyncID, &syncData)))
  {
    action->SetError(701, "No such sync data");
    return NPT_SUCCESS;
  }

  PLT_SyncActionDataList partnersToSync;
  if (strActionCaller.IsEmpty())
  {
    NPT_List<PLT_Partner> partners;
    NPT_CHECK(syncData.GetPartners(partners));
    //Deletions are allowed to fail on partners... so don't mind if a partner is not online
    //Henve we don't check for failure here.
    GetPartners(partners.GetFirstItem(), partnersToSync, device->GetUUID());
  }
  NPT_CHECK(delegate->OnDeleteSyncData(strSyncID));

  PLT_SyncActionDataList::Iterator currentDevice = partnersToSync.GetFirstItem();

  //Now we have to push the new syncData to all the partners
  NPT_Array<PLT_StringPair> arguments;
  NPT_CHECK(arguments.Add(PLT_StringPair("ActionCaller", this->GetDevice()->GetUUID())));
  NPT_CHECK(arguments.Add(PLT_StringPair("SyncID", strSyncID)));

  NPT_CHECK(InvokeAction(currentDevice, "urn:schemas-upnp-org:service:ContentSync:1", "DeleteSyncData", arguments, m_CtrlPoint->m_CtrlPoint));

  //TODO: figure out a better way to wait for our result
  currentDevice = partnersToSync.GetFirstItem();
  NPT_CHECK(WaitForAllResult(currentDevice));

  return NPT_SUCCESS;
}

NPT_Result PLT_ContentSyncService::OnGetSyncData(PLT_ActionReference&          action,
                                                 const PLT_HttpRequestContext& context)
{
  NPT_String    strSyncID;
  if (NPT_FAILED(action->GetArgumentValue("SyncID", strSyncID))) {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  PLT_SyncData SyncData;
  NPT_Result result = delegate->OnGetSyncData(strSyncID, &SyncData);

  NPT_String strSyncData;
  NPT_CHECK_SEVERE(SyncData.ToXml(strSyncData, !strSyncID.IsEmpty()));
  NPT_CHECK_SEVERE(action->SetArgumentValue("SyncData", strSyncData));

  return result;
}

NPT_Result PLT_ContentSyncService::CompareSyncStructures(NPT_List<PLT_SyncStructureRef>::Iterator ourRelationship,
                                                          const PLT_SyncData& remoteSyncData)
{ //TODO: this seems should  probably go into libupnp, as it's mostly stuff for the upnp spec
  while (ourRelationship)
  {
    NPT_List<PLT_SyncStructureRef>::Iterator remotePartner = remoteSyncData.m_syncData.Find(IDFinder((*ourRelationship)->GetID()));
    if (!remotePartner)
    { //it is not allowed to add a sync structure without notifying the partner
      //but deleting is, so we can assume, that the partner has deleted that relationship, so we should do so as well
      delegate->OnDeleteSyncData((*ourRelationship)->GetID());
    }
    else
    {
      if ((*ourRelationship)->GetUpdateID() < (*remotePartner)->GetUpdateID())
      { //remote partner has a newer version of the sync relationship
        if ((*ourRelationship)->GetType() != (*remotePartner)->GetType())
          return NPT_ERROR_INTERNAL;

        PLT_SyncData syncData;
        syncData.m_syncData.Add(*remotePartner);
        NPT_CHECK(delegate->OnModifySyncData((*ourRelationship)->GetID(), syncData));
      }
        //TODO: maybe we should check if the sync data is actually the same? Right now we have the problem, that
        //both partners could have updated the sync structure once (they would have the same update ID but might differ)...
        //The spec doesn't say anything about that?!? other solution would be to make sure that modifications are only done if they worked
        //in all partners...Anyway, kodi doesn't modify change structures at the moment anyway, so this whole part is mainly for standard
        //compliance and only rudimentary tested ;)

        //check child-structures, if they need modification!
      NPT_List<PLT_SyncStructureRef> ourChilds;
      (*ourRelationship)->GetChilds(ourChilds);
      if (ourChilds.GetItemCount() > 0)
        NPT_CHECK(CompareSyncStructures(ourChilds.GetFirstItem(), remoteSyncData));
    }
      //if we have a newer version, than the partner will deal with it when we return our sync data as a response to this action
      //so, we don't need do deal with that in here!
    ++ourRelationship;
  }
  return NPT_SUCCESS;
}

NPT_Result PLT_ContentSyncService::OnExchangeSyncData(PLT_ActionReference&          action,
                                                      const PLT_HttpRequestContext& context)
{
  NPT_String    strLocalSyncData;
  PLT_SyncData  localSyncData;

  if (NPT_FAILED(action->GetArgumentValue("LocalSyncData", strLocalSyncData)))
  {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  NPT_XmlElementNode* localSyncNode;
  if (NPT_FAILED(PLT_XmlHelper::Parse(strLocalSyncData, localSyncNode)) || NPT_FAILED(localSyncData.FromXml(localSyncNode, false))) //xml parsing failed
  {
    action->SetError(702, "Invalid XML");
    return NPT_SUCCESS;
  }



  //we have the partners sync structure, let's get our own
  PLT_SyncData ourSyncData;
  NPT_CHECK(delegate->OnGetSyncData("", &ourSyncData));

  //compare the sync structures and remove or modify them as neccessary.
  NPT_List<PLT_SyncStructureRef>::Iterator relationship = ourSyncData.m_syncData.GetFirstItem();
  NPT_CHECK(CompareSyncStructures(relationship, localSyncData));

  //We actually took care of everything, but let's allow the delegate to do its app specific buiseness if it wants to.
  NPT_Result result = delegate->OnExchangeSyncData(localSyncData);

  //Get the sync data again, as it might have changed
  PLT_SyncData RemoteSyncData;
  NPT_CHECK(delegate->OnGetSyncData("", &RemoteSyncData));

  NPT_String strRemoteSyncData;
  NPT_CHECK_SEVERE(RemoteSyncData.ToXml(strRemoteSyncData, false));
  NPT_CHECK_SEVERE(action->SetArgumentValue("RemoteSyncData", strRemoteSyncData));

  return result;
}

NPT_Result PLT_ContentSyncService::OnAddSyncPair(PLT_ActionReference&          action,
                                                 const PLT_HttpRequestContext& context)
{
  NPT_String strActionCaller;
  NPT_String strObjectID;
  NPT_String strSyncPair;
  PLT_SyncPair syncPair;

  if (NPT_FAILED(action->GetArgumentValue("ActionCaller", strActionCaller)) ||
      NPT_FAILED(action->GetArgumentValue("ObjectID", strObjectID)) ||
      NPT_FAILED(action->GetArgumentValue("SyncPair", strSyncPair)))
  {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  NPT_XmlElementNode* syncPairNode;
  if (NPT_FAILED(PLT_XmlHelper::Parse(strSyncPair, syncPairNode)) || NPT_FAILED(syncPair.FromXml(syncPairNode)))
  {
    action->SetError(702, "Invalid XML");
    return NPT_SUCCESS;
  }

  PLT_DeviceData* device = GetDevice();

  PLT_SyncData syncData;
  if (NPT_FAILED(delegate->OnGetSyncData(syncPair.m_syncRelationshipID, &syncData)) ||
     !syncData.Contains(syncPair.m_partnershipID) ||
     !syncData.Contains(syncPair.m_pairGroupID))
  {
    action->SetError(701, "No such sync data");
    return NPT_SUCCESS;
  }
  if (syncData.m_syncData.GetItemCount()!=1)
    return NPT_ERROR_INTERNAL;
  PLT_ActiveChecker ac;
  if (NPT_FAILED(syncData.Visit(&ac, true)))
    return NPT_ERROR_INVALID_PARAMETERS;

  PLT_SyncActionDataList partnersToSync;
  if (strActionCaller.IsEmpty())
  {
    NPT_List<PLT_Partner> partners;
    NPT_CHECK(syncData.GetPartners(partners));
    NPT_CHECK(GetPartners(partners.GetFirstItem(), partnersToSync, device->GetUUID()));
  }

  PLT_SyncActionDataList::Iterator currentDevice = partnersToSync.GetFirstItem();

  //Get the sync policy for the PairGroup
  PLT_SyncPolicy usedPolicy;
  (*syncData.m_syncData.GetFirstItem())->AggreagateSyncPolicy(usedPolicy);
  PLT_SyncPolicy::Merge(usedPolicy, syncPair.m_syncPolicy, syncPair.m_syncPolicy); //Override the pair group sync policy with syncPair specific values (only where supplied)

  usedPolicy.m_policyType = "Kodi";
  if (delegate->HasSyncPair(syncData, syncPair.m_remoteObjectID))
  { //there already is a syncpair for that object in the same sync relationship... we have to obey some rules:
    if (usedPolicy.m_policyType.Compare("replace", false)==0)
    {
        //TODO: if we are not the source of the item we are not allowed to have the same item in two syncpairs
    }
    else if (usedPolicy.m_policyType.Compare("blend", false)==0)
    {
        //TODO: if we are not the priority partner we are not allowed to have the same item in two syncpairs
    }
    else if (usedPolicy.m_policyType.Compare("merge", false)==0)
    {
      //duplicates are never allowed
      action->SetError(709, "The AddSyncpair() request failed, because the specified SyncPair argument is invalid.");
      return NPT_SUCCESS;
    }
  }

  NPT_String strOurObjectID;
  NPT_CHECK(delegate->OnAddSyncPair(strObjectID, syncPair, strOurObjectID));

  //Now we have to push the new syncData to all the partners
  NPT_Array<PLT_StringPair> arguments;
  NPT_CHECK(arguments.Add(PLT_StringPair("ActionCaller", this->GetDevice()->GetUUID())));
  NPT_CHECK(arguments.Add(PLT_StringPair("ObjectID", strOurObjectID))); //we need to tell the partners the id, that object has on us.
  syncPair.ToXml(strSyncPair);
  NPT_CHECK(arguments.Add(PLT_StringPair("SyncPair", strSyncPair)));


  NPT_CHECK(InvokeAction(currentDevice, "urn:schemas-upnp-org:service:ContentSync:1", "AddSyncPair", arguments, m_CtrlPoint->m_CtrlPoint));

  //TODO: figure out a better way to wait for our result
  currentDevice = partnersToSync.GetFirstItem();
  NPT_CHECK(WaitForAllResult(currentDevice));

  return NPT_SUCCESS;

}

NPT_Result PLT_ContentSyncService::OnModifySyncPair(PLT_ActionReference&          action,
                                                    const PLT_HttpRequestContext& context)
{
  NPT_String strActionCaller;
  NPT_String strObjectID;
  NPT_String strSyncPairs;
  PLT_SyncPairs syncPairs;

  if (NPT_FAILED(action->GetArgumentValue("ActionCaller", strActionCaller)) ||
      NPT_FAILED(action->GetArgumentValue("ObjectID", strObjectID)) ||
      NPT_FAILED(action->GetArgumentValue("SyncPair", strSyncPairs)))
  {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  NPT_XmlElementNode* syncPairsNode;
  if (NPT_FAILED(PLT_XmlHelper::Parse(strSyncPairs, syncPairsNode)) || NPT_FAILED(syncPairs.FromXml(syncPairsNode)))
  {
    action->SetError(702, "Invalid XML");
    return NPT_SUCCESS;
  }

  return delegate->OnModifySyncPair(strObjectID, syncPairs);
}

NPT_Result PLT_ContentSyncService::OnDeleteSyncPair(PLT_ActionReference&          action,
                                                    const PLT_HttpRequestContext& context)
{
  NPT_String strActionCaller;
  NPT_String strObjectID;
  NPT_String strSyncID;

  if (NPT_FAILED(action->GetArgumentValue("ActionCaller", strActionCaller)) ||
      NPT_FAILED(action->GetArgumentValue("ObjectID", strObjectID)) ||
      NPT_FAILED(action->GetArgumentValue("SyncID", strSyncID)))
  {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  return delegate->OnDeleteSyncPair(strObjectID, strSyncID);
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
