/*****************************************************************
|
|   Platinum - AV Media Server Device
|
| Copyright (c) 2004-2010, Plutinosoft, LLC.
| All rights reserved.
| http://www.plutinosoft.com
|
| This program is free software; you can redistribute it and/or
| modify it under the terms of the GNU General Public License
| as published by the Free Software Foundation; either version 2
| of the License, or (at your option) any later version.
|
| OEMs, ISVs, VARs and other distributors that combine and 
| distribute commercially licensed software with Platinum software
| and do not wish to distribute the source code for the commercially
| licensed software under version 2, or (at your option) any later
| version, of the GNU General Public License (the "GPL") must enter
| into a commercial license agreement with Plutinosoft, LLC.
| licensing@plutinosoft.com
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

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "PltContentSyncService.h"
#include "PltHttp.h"
#include "ContentSync/PltSyncData.h"
#include "PltUtilities.h"
#include "PltCtrlPointTask.h"
#include "../../utils/log.h"

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

/*
template<typename T>
NPT_Result FromXMLHelper(const NPT_String& str, T& result)
{
  NPT_XmlNode *root;
  NPT_XmlParser parser;
  
  NPT_CHECK(parser.Parse(str.GetChars(), root));
  return result.FromXml(*root);
}*/

NPT_Result PLT_ContentSyncCtrlPoint::InvokeAddSyncData(PLT_DeviceDataReference& device,
                                                       const NPT_String& actionCaller, 
                                                       const NPT_String& syncID, 
                                                       const PLT_SyncData& syncData, 
                                                       PLT_SyncData& syncDataResult)
{
  PLT_Service* service;
  PLT_ActionReference action;
  
  
  NPT_CHECK(device->FindServiceById("urn:schemas-upnp-org:service:ContentSync:1", service));
  
  NPT_CHECK_SEVERE(m_CtrlPoint->CreateAction(device,
                                             "urn:schemas-upnp-org:service:ContentSync:1",
                                             "AddSyncData",
                                             action));
  
  NPT_CHECK(action->SetArgumentValue("ActionCaller", actionCaller));
  NPT_CHECK(action->SetArgumentValue("SyncID", syncID));
  NPT_String strSyncData;
  NPT_CHECK(syncData.ToXml(strSyncData));
  NPT_CHECK(action->SetArgumentValue("SyncData", strSyncData));
  
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
    m_CtrlPoint(ctrlPoint),
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

    // is it a new device?
    if (NPT_SUCCEEDED(NPT_ContainerFind(m_ContentSyncDevices,
      PLT_SyncDeviceDataFinder(uuid), data))) {
      NPT_LOG_WARNING_1("Device (%s) is already in our list!", (const char*)uuid);
      return NPT_FAILURE;
    }
    else
    {
      // Issue a 'ExchangeSyncData'
    }

    NPT_LOG_FINE_1("Device Found: %s", (const char*)*device);

    PLT_SyncDeviceHolder deviceHolder(device);
    m_ContentSyncDevices.Add(deviceHolder);
  }

  // subscribe to required services
  m_CtrlPoint->Subscribe(serviceCDS);

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
PLT_ContentSyncService::OnActionResponse(NPT_Result           res,
                                         PLT_ActionReference& action,
                                         void*                userdata)
{
  NPT_String actionName = action->GetActionDesc().GetName();

  if (actionName.Compare("ExchangeSyncData", true) == 0)
    return NPT_ERROR_NOT_IMPLEMENTED;
  else
  {
    //If we didn't do an ExchangeSyncData with that device yet
    //TODO: log a warning, that we might be working with stale sync object data... 
  }

  if (actionName.Compare("AddSyncData", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
  }
  if (actionName.Compare("ModifySyncData", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
  }
  if (actionName.Compare("DeleteSyncData", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
  }
  if (actionName.Compare("GetSyncData", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
  }
  if (actionName.Compare("ExchangeSyncData", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
  }
  if (actionName.Compare("AddSyncPair", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
  }
  if (actionName.Compare("ModifySyncPair", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
  }
  if (actionName.Compare("DeleteSyncPair", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
  }
  if (actionName.Compare("StartSync", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
  }
  if (actionName.Compare("AbortSync", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
  }
  if (actionName.Compare("GetChangeLog", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
  }
  if (actionName.Compare("ResetChangeLog", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
  }
  if (actionName.Compare("ResetStatus", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
  }
  if (actionName.Compare("GetSyncStatus", true) == 0) {
    return NPT_ERROR_NOT_IMPLEMENTED;
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
  if (NPT_FAILED(PLT_XmlHelper::Parse(strSyncData, syncNode)) || NPT_FAILED(syncData.FromXml(syncNode))) //xml parsing failed
  {
    action->SetError(702, "Invalid XML");
    return NPT_SUCCESS;
  }
  if (false) //If invalid ID
  {
    action->SetError(701, "No such sync data");
    return NPT_SUCCESS;
  }
  if (false) //the action caller is part of the sync data :(
  {
    action->SetError(703, "Invalid action caller");
    return NPT_SUCCESS;
  }

  PLT_DeviceData* device = GetDevice();

  NPT_List<PLT_Partner> partners;
  NPT_CHECK(syncData.m_syncRelationship.GetPartners(device->GetUUID(), partners));

  if (strActionCaller.IsEmpty())
  { //If we got an action caller, then the UUIDs are already set in the syncData.
    //but if the ActionCaller is empty, we need to create them
    syncData.GenerateUUIDs();
  }

  //If the action caller is not set, means a Control Point wants us to create a Sync Relationship with a third device.
  // That means we need to make sure, that partner is online
  // If we have more thant one partner, we also need to make sure those are online
  NPT_List<PLT_SyncDeviceHolder> partnersToSync; //Will hold an array of all devices we have to tell about the new sync data!
  if(strActionCaller.IsEmpty())
  {
    NPT_List<PLT_Partner>::Iterator i = partners.GetFirstItem();
    while(i)
    {
      PLT_SyncDeviceHolder currentPartnerDevice;
      if (NPT_FAILED(NPT_ContainerFind(m_ContentSyncDevices,
                                       PLT_SyncDeviceDataFinder(i->m_strDeviceUDN), currentPartnerDevice)))
      { //A control point asked us to create a sync relationship with an upnp device that's not online
        action->SetError(705, "Partner not online");
        return NPT_SUCCESS;
      }
      partnersToSync.Add(currentPartnerDevice);
      ++i;
    }
  }

  PLT_SyncData SyncDataResult;
  NPT_Result result = delegate->OnAddSyncData(strActionCaller, strSyncID, syncData, &SyncDataResult);
  NPT_CHECK_SEVERE(result);

  result = NPT_ERROR_INTERNAL; //In case we fail now, we goto cleanup; and that part will return our 'result' var. so set it to some generic error.    

  NPT_String strSyncDataResult;
  NPT_List<PLT_SyncDeviceHolder>::Iterator currentDevice = partnersToSync.GetFirstItem();

  NPT_CHECK_LABEL(SyncDataResult.ToXml(strSyncDataResult), cleanup);

  //Now we have push the new syncData to all the partners
  while (currentDevice)
  { 
    PLT_ActionReference notifyPartner;
    NPT_CHECK_LABEL(m_CtrlPoint->CreateAction(currentDevice->device,
                                              "urn:schemas-upnp-org:service:AVTransport:1",
                                              "AddSyncData",
                                              notifyPartner), cleanup_others);
    NPT_CHECK_LABEL(notifyPartner->SetArgumentValue("ActionCaller", this->GetDevice()->GetUUID()), cleanup_others);
    NPT_CHECK_LABEL(notifyPartner->SetArgumentValue("SyncID", strSyncID), cleanup_others);
    NPT_CHECK_LABEL(notifyPartner->SetArgumentValue("SyncData", strSyncDataResult), cleanup_others);

    PLT_CtrlPointInvokeActionTask* task;
    NPT_CHECK_LABEL(m_CtrlPoint->CreateActionThread(notifyPartner, NULL, &task), cleanup_others);
    
    NPT_CHECK_LABEL(m_CtrlPoint->InvokeAction(task), cleanup_others);
    ++currentDevice;
    
    NPT_String r;
    NPT_CHECK(notifyPartner->GetArgumentValue("SyncDataResult", r));
    r += "\n";

    //TODO: if this action fails, we have to revert our own call to:
    // delegate->OnAddSyncData(strActionCaller, strSyncID, syncData, &SyncDataResult);
    // and we have to revert all calls to the previous partners

    //TODO: check if SyncDataResult of the invoked action is the same SyncDataResult as of
    // delegate->OnAddSyncData(strActionCaller, strSyncID, syncData, &SyncDataResult);
    //TODO: if not, should we fail?!? what does the specs say, is that even allowed?!?

  }
  
  NPT_CHECK_SEVERE(SyncDataResult.ToXml(strSyncDataResult));
  NPT_CHECK_SEVERE(action->SetArgumentValue("SyncDataResult", strSyncDataResult));

  return NPT_SUCCESS;  
cleanup_others:
  --currentDevice;
  while(currentDevice)
  {
    //TODO: actually revert it
    --currentDevice;
  }
  
cleanup:
  //TODO: we have to revert our call to:
  //delegate->OnAddSyncData(strActionCaller, strSyncID, syncData, &SyncDataResult);

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

  NPT_XmlElementNode* syncDataNode;
  if(NPT_FAILED(PLT_XmlHelper::Parse(strSyncData, syncDataNode)) || NPT_FAILED(syncData.FromXml(syncDataNode)))
  {
    action->SetError(702, "Invalid XML");
    return NPT_SUCCESS;
  }

  return delegate->OnModifySyncData(strActionCaller, strSyncID, syncData);
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

  return delegate->OnDeleteSyncData(strActionCaller, strSyncID);
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
  NPT_CHECK_SEVERE(SyncData.ToXml(strSyncData));
  NPT_CHECK_SEVERE(action->SetArgumentValue("SyncData", strSyncData));

  return result;
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
  if (NPT_FAILED(PLT_XmlHelper::Parse(strLocalSyncData, localSyncNode)) || NPT_FAILED(localSyncData.FromXml(localSyncNode))) //xml parsing failed
  {
    action->SetError(702, "Invalid XML");
    return NPT_SUCCESS;
  }

  PLT_SyncData RemoteSyncData;
  NPT_Result result = delegate->OnExchangeSyncData(localSyncData, &RemoteSyncData);

  NPT_String strRemoteSyncData;
  NPT_CHECK_SEVERE(RemoteSyncData.ToXml(strRemoteSyncData));
  NPT_CHECK_SEVERE(action->SetArgumentValue("RemoteSyncData", strRemoteSyncData));

  return result;
}

NPT_Result PLT_ContentSyncService::OnAddSyncPair(PLT_ActionReference&          action,
                                                 const PLT_HttpRequestContext& context)
{
  NPT_String strActionCaller;
  NPT_String strObjectID;
  NPT_String strSyncPair;
  PLT_SyncPairs syncPair;

  if (NPT_FAILED(action->GetArgumentValue("ActionCaller", strActionCaller)) ||
      NPT_FAILED(action->GetArgumentValue("ObjectID", strObjectID)) || 
      NPT_FAILED(action->GetArgumentValue("SyncPair", strSyncPair)))
  {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  NPT_XmlElementNode* syncPairNode;
  if(NPT_FAILED(PLT_XmlHelper::Parse(strSyncPair, syncPairNode)) || NPT_FAILED(syncPair.FromXml(syncPairNode)))
  {
    action->SetError(702, "Invalid XML");
    return NPT_SUCCESS;
  }

  return delegate->OnAddSyncPair(strActionCaller, strObjectID, syncPair);
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
  if(NPT_FAILED(PLT_XmlHelper::Parse(strSyncPairs, syncPairsNode)) || NPT_FAILED(syncPairs.FromXml(syncPairsNode)))
  {
    action->SetError(702, "Invalid XML");
    return NPT_SUCCESS;
  }

  return delegate->OnModifySyncPair(strActionCaller, strObjectID, syncPairs);
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

  return delegate->OnDeleteSyncPair(strActionCaller, strObjectID, strSyncID);
}

NPT_Result PLT_ContentSyncService::OnStartSync(PLT_ActionReference&          action,
                                               const PLT_HttpRequestContext& context)
{
  NPT_String strActionCaller;
  NPT_String strSyncID;

  if (NPT_FAILED(action->GetArgumentValue("ActionCaller", strActionCaller)) ||
      NPT_FAILED(action->GetArgumentValue("SyncID", strSyncID)))
  {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  return delegate->OnStartSync(strActionCaller, strSyncID);
}

NPT_Result PLT_ContentSyncService::OnAbortSync(PLT_ActionReference&          action,
                                               const PLT_HttpRequestContext& context)
{
  NPT_String strActionCaller;
  NPT_String strSyncID;

  if (NPT_FAILED(action->GetArgumentValue("ActionCaller", strActionCaller)) ||
      NPT_FAILED(action->GetArgumentValue("SyncID", strSyncID)))
  {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  return delegate->OnAbortSync(strActionCaller, strSyncID);
}

NPT_Result PLT_ContentSyncService::OnGetChangeLog(PLT_ActionReference&          action,
                                                  const PLT_HttpRequestContext& context)
{
  NPT_String strSyncID;
  NPT_Int32  iStartingIndex;
  NPT_Int32  iRequestedCount;

  if (NPT_FAILED(action->GetArgumentValue("StartingIndex",  iStartingIndex)) ||
      NPT_FAILED(action->GetArgumentValue("SyncID",         strSyncID)) || 
      NPT_FAILED(action->GetArgumentValue("RequestedCount", iRequestedCount)))
  {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  PLT_ChangeLog Result;
  NPT_UInt32 NumberReturned;
  NPT_UInt32 TotalMatches;
  NPT_Result result = delegate->OnGetChangeLog(strSyncID, iStartingIndex, iRequestedCount, &Result, NumberReturned, TotalMatches);

  NPT_String strResult;
  NPT_CHECK_SEVERE(Result.ToXml(strResult));
  NPT_CHECK_SEVERE(action->SetArgumentValue("Result", strResult));
  NPT_CHECK_SEVERE(action->SetArgumentValue("NumberReturned", NPT_String::FromInteger(NumberReturned)));
  NPT_CHECK_SEVERE(action->SetArgumentValue("TotalMatches", NPT_String::FromInteger(TotalMatches)));

  return result;
}

NPT_Result PLT_ContentSyncService::OnResetChangeLog(PLT_ActionReference&          action,
                                                    const PLT_HttpRequestContext& context)
{
  NPT_String strSyncID;
  NPT_String strObjectIDs;
  PLT_ResetObjectList objectIDs;

  if (NPT_FAILED(action->GetArgumentValue("ObjectIDs", strObjectIDs)) ||
      NPT_FAILED(action->GetArgumentValue("SyncID", strSyncID)))
  {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  NPT_XmlElementNode* objectIDsNode;
  if(NPT_FAILED(PLT_XmlHelper::Parse(strObjectIDs, objectIDsNode)) || NPT_FAILED(objectIDs.FromXml(objectIDsNode)))
  {
    action->SetError(702, "Invalid XML");
    return NPT_SUCCESS;
  }

  return delegate->OnResetChangeLog(strSyncID, objectIDs);
}

NPT_Result PLT_ContentSyncService::OnResetStatus(PLT_ActionReference&          action,
                                                 const PLT_HttpRequestContext& context)
{
  NPT_String strSyncID;

  if (NPT_FAILED(action->GetArgumentValue("SyncID", strSyncID)))
  {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  return delegate->OnResetStatus(strSyncID);
}

NPT_Result PLT_ContentSyncService::OnGetSyncStatus(PLT_ActionReference&          action,
                                                   const PLT_HttpRequestContext& context)
{
  NPT_String strSyncID;

  if (NPT_FAILED(action->GetArgumentValue("SyncID", strSyncID)))
  {
    NPT_LOG_WARNING("Missing arguments");
    action->SetError(402, "Invalid args");
    return NPT_SUCCESS;
  }

  PLT_SyncStatus SyncStatus;
  NPT_Result result = delegate->OnGetSyncStatus(strSyncID, &SyncStatus);

  NPT_String strSyncStatus;
  NPT_CHECK_SEVERE(SyncStatus.ToXml(strSyncStatus));
  NPT_CHECK_SEVERE(action->SetArgumentValue("SyncStatus", strSyncStatus));

  return result;
}
