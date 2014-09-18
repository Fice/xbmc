/*
 *      Copyright (C) 2014 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#include <Platinum/Source/Platinum/Platinum.h>
#include "UPnPSyncCtrlPoint.h"
#include "UPnPDatabase.h"
#include "dbwrappers/changeset.h"
#include "filesystem/UPnPDirectory.h"
#include "FileItem.h"
#include "utils/log.h"
#include "UPnP.h"
#include "UPnPServer.h"
#include "dialogs/GUIDialogKaiToast.h"

namespace UPNP
{

bool CUPnPSyncCtrlPoint::CreateSyncRelationship(const CURL& url)
{
  std::string DeviceUUID = url.GetHostName();

  PLT_DeviceDataReference device;
  if(NPT_FAILED(CUPnP::GetInstance()->m_MediaBrowser->FindServer(DeviceUUID.c_str(), device) && !device.IsNull()))
    return false;

  CUPnPServer* server = CUPnP::GetInstance()->GetServer();
  if (!server)
    return false;

  PLT_PairGroupRef pairGroup(new PLT_PairGroup);

  PLT_SyncPolicy policy;
  policy.m_policyType = "TRACKING";
  policy.m_delProtection.SetValue(false);
  policy.m_autoObjAdd.SetValue(true);

  PLT_PartnershipRef partnership(new PLT_Partnership);

  PLT_Partner partner1;
  partner1.m_strDeviceUDN = server->GetUUID();
  PLT_Service *syncService;
  if (NPT_FAILED(server->FindServiceByType("urn:schemas-upnp-org:service:ContentSync:1", syncService)))
    return false;
  partner1.m_strServiceID = syncService->GetServiceID();
  partnership->SetPartner1(partner1);

  PLT_Partner partner2;
  partner2.m_strDeviceUDN = device->GetUUID();
  if (NPT_FAILED(device->FindServiceByType("urn:schemas-upnp-org:service:ContentSync:1", syncService)))
    return false;
  partner2.m_strServiceID = syncService->GetServiceID();
  partnership->SetPartner2(partner2);

  partnership->SetPolicy(policy);
  partnership->AddChild(pairGroup);

  PLT_SyncRelationshipRef relationship(new PLT_SyncRelationship());
  relationship->SetTitle("Sync between "+server->GetFriendlyName()+" and " + device->GetFriendlyName());
  relationship->AddChild(partnership);
  relationship->GetActiveState().SetValue(true);

  PLT_SyncData syncData;
  syncData.m_syncData.Add(relationship);

  //TODO: get 'parent' of the curl path, so we can set syncID. (Only needed if we actually want to change syncPairs or so?!?)

  PLT_SyncData syncDataResult;
  if(NPT_FAILED(InvokeAddSyncData(device,
                                  "", //TODO: action caller
                                  "",
                                  syncData, new CURL(url))))
    return false;
  return true;
}

bool CUPnPSyncCtrlPoint::ModifySyncRelationship(const CURL& url,  PLT_SyncRelationshipRef syncRelationship)
{
  std::string DeviceUUID = url.GetHostName();

  PLT_DeviceDataReference device;
  if (NPT_FAILED(CUPnP::GetInstance()->m_MediaBrowser->FindServer(DeviceUUID.c_str(), device) && !device.IsNull()))
  {
    CLog::Log(LOGINFO, "%s - Modifiying sync relationship with the device '%s'. "
                            "That device is currently not online, we will tell him when we're connected the next time", __FUNCTION__, DeviceUUID.c_str());
    return false; //TODO: Instead we need to tell CUPnPSyncService that it needs to delete the sync relationship, and then we can return early here (with true as the result)
  }

  PLT_SyncData syncData;
  syncData.m_syncData.Add((const PLT_SyncStructureRef)syncRelationship);
  if (NPT_FAILED(InvokeModifySyncData(device, "", syncRelationship->GetID(), syncData)))
    return false;

  return true;
}

bool CUPnPSyncCtrlPoint::RemoveSyncRelationship(const CURL& url)
{
  std::string DeviceUUID = url.GetHostName();

  PLT_DeviceDataReference device;
  if(NPT_FAILED(CUPnP::GetInstance()->m_MediaBrowser->FindServer(DeviceUUID.c_str(), device) && !device.IsNull()))
  {
    CLog::Log(LOGINFO, "%s - Removing sync relationship with the device '%s'. "
                       "That device is currently not online, we will tell him the next time we're connected", __FUNCTION__, DeviceUUID.c_str());
    return false; //TODO: Instead we need to tell CUPnPSyncService that it needs to delete the sync relationship, and then we can return early here (with true as the result)
  }

  CUPnPDatabase db;
  PLT_SyncRelationshipRef syncRelationship;
  if(!db.GetSyncRelationshipForDevice(DeviceUUID, syncRelationship)) //TODO: a more specialised version that just returns the id, for efficiency?!?
    return false;

  if(NPT_FAILED(InvokeDeleteSyncData(device, "", syncRelationship->GetID())))
    return false;

  return true;
}


bool CUPnPSyncCtrlPoint::AddSyncPairs(const CURL& url)
{
  CFileItemList items;
  XFILE::CUPnPDirectory dir;

  if (!dir.GetDirectory(url, items))
  {
    CLog::Log(LOGERROR, "%s - Couldn't create sync Pairs for items in url '%s'", __FUNCTION__, url.GetWithoutUserDetails().c_str());
    return false;
  }
  const VECFILEITEMS& vecItems = items.GetList();
  VECFILEITEMS::const_iterator iter;
  for (iter = vecItems.begin(); iter != vecItems.end(); ++iter)
  {
    if (iter->get()->IsFileFolder())
    {
      AddSyncPairs(iter->get()->GetURL());
    }
    else
    {
/*      if (NPT_FAILED(InvokeAddSyncPair(device, "", syncRelationship->GetID())))
        return false;*/
    }
  }
  return true;
}

std::string CUPnPSyncCtrlPoint::AddSyncPairRoot(const NPT_String object_id, PLT_DeviceDataReference device)
{
  //First get the metadata of the root path we want to add... (we need to make sure it allows syncing!)
  PLT_MediaObjectListReference list;
  if (NPT_FAILED(CUPnP::GetInstance()->m_MediaBrowser->BrowseSync(device, object_id.GetChars(), list, true)))
    return "";
  PLT_MediaObject* mediaObject = *list->GetFirstItem();

  std::string str = "<";
  str += mediaObject->m_ObjectID + ">";
  PLT_MediaObjectListReference childs;
  if (NPT_FAILED(CUPnP::GetInstance()->m_MediaBrowser->BrowseSync(device, object_id.GetChars(), list)))
    return "";
  PLT_MediaObjectList::Iterator iter = childs->GetFirstItem();
  while (iter)
  {
    str += AddSyncPairRoot((*iter)->m_ObjectID, device);
    ++iter;
  }
  str += "</" + mediaObject->m_ObjectID + ">";
  return str;
}

NPT_Result CUPnPSyncCtrlPoint::OnDeviceAdded(PLT_DeviceDataReference& device)
{
  //TODO: remove, this is just for testing... in the OnGetSyncData response handler, will we print it to the console!
  NPT_CHECK(InvokeGetSyncData(device, ""));

  CUPnPDatabase db;
  if (!db.Open())
    return NPT_ERROR_INTERNAL;
  if (db.HasSyncRelationshipWith(device->GetUUID()))
  {
    /* TODO: if (StartSync(AddSyncPairs))
      return true;*/
  }

  return NPT_SUCCESS;
}

NPT_Result CUPnPSyncCtrlPoint::OnDeviceRemoved(PLT_DeviceDataReference& device)
{
  return NPT_ERROR_NOT_IMPLEMENTED;;
}

NPT_Result CUPnPSyncCtrlPoint::OnRemoveSyncDataResponse(NPT_Result res, PLT_ActionReference& action, void* userdata)
{
  return return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPSyncCtrlPoint::OnAddSyncDataResponse(NPT_Result res, PLT_ActionReference& action, void* userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPSyncCtrlPoint::OnGetSyncDataResponse(NPT_Result res, PLT_ActionReference& action, void* userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPSyncCtrlPoint::OnAddSyncPairResponse(NPT_Result res, PLT_ActionReference& action, void* userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPSyncCtrlPoint::OnActionResponse(NPT_Result res, PLT_ActionReference& action, void* userdata)
{
  NPT_String actionName = action->GetActionDesc().GetName();

  if (actionName.Compare("RemoveSyncData", true) == 0)
    return OnRemoveSyncDataResponse(res, action, userdata);
  if (actionName.Compare("AddSyncData", true) == 0)
    return OnAddSyncDataResponse(res, action, userdata);
  else if (actionName.Compare("GetSyncData", true) == 0)
    return OnGetSyncDataResponse(res, action, userdata);
  else if (actionName.Compare("ExchangeSyncData", true) == 0)
    return OnExchangeSyncDataResponse(res, action, userdata);
  else if (actionName.Compare("AddSyncPair", true) == 0)
    return OnAddSyncPairResponse(res, action, userdata);
  return NPT_SUCCESS;
}

NPT_Result CUPnPSyncCtrlPoint::OnEventNotify(PLT_Service* service, NPT_List<PLT_StateVariable*>* vars)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}
/*
bool InvokeUpdateObject(const char* id, const char* curr_value, const char* new_value)
{
  return false;
}*/

}
