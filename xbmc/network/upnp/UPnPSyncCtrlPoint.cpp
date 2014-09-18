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
  return NPT_SUCCESS;
}

NPT_Result CUPnPSyncCtrlPoint::OnRemoveSyncDataResponse(NPT_Result res, PLT_ActionReference& action, void* userdata)
{
  if (action->GetErrorCode() < 100)
    CGUIDialogKaiToast::QueueNotification("upnp sync", "Sync relation has been removed"); //TODO: localize
  else
    CGUIDialogKaiToast::QueueNotification("upnp sync", "Failure when trying to remove sync relationship"); //TODO: localize
  return NPT_SUCCESS;
}

NPT_Result CUPnPSyncCtrlPoint::OnAddSyncDataResponse(NPT_Result res, PLT_ActionReference& action, void* userdata)
{
  if (action->GetErrorCode() < 100)
  {
    NPT_String actionCaller;
    NPT_CHECK(action->GetArgumentValue("ActionCaller", actionCaller));
    if (actionCaller.IsEmpty()) //We are the ctrl point that created the syncData
    { //Now our job is to create a sync pair for ALL the items we and our partner have
      CGUIDialogKaiToast::QueueNotification("upnp sync", "Sync relation has been established"); //TODO: localize
      std::auto_ptr<CURL> url = std::auto_ptr<CURL>((CURL*)userdata);
      
      NPT_String    strSyncData;
      PLT_SyncData  syncData;
      
      NPT_CHECK(action->GetArgumentValue("SyncDataResult", strSyncData));
      NPT_XmlElementNode* syncNode;
      NPT_CHECK(PLT_XmlHelper::Parse(strSyncData, syncNode));
      NPT_CHECK(syncData.FromXml(syncNode, true));
      
      std::string DeviceUUID = url->GetHostName();
      NPT_String path = url->Get().c_str();
      PLT_DeviceDataReference device;
      if (NPT_FAILED(CUPnP::GetInstance()->m_MediaBrowser->FindServer(DeviceUUID.c_str(), device) && !device.IsNull()))
        return false;
      
      /*
       NPT_String object_id;
       { //Get object ID
       //TODO: unify! This code has been copied from CUPnPDirectory to get the object id from a path
       int next_slash = path.Find('/', 7);
       NPT_String uuid = (next_slash == -1) ? path.SubString(7) : path.SubString(7, next_slash - 7);
       object_id = (next_slash == -1) ? "" : path.SubString(next_slash + 1);
       object_id.TrimRight("/");
       if (object_id.GetLength()) {
       object_id = CURL::Decode((char*)object_id).c_str();
       }
       // issue a browse request with object_id
       // if object_id is empty use "0" for root
       object_id = object_id.IsEmpty() ? "0" : object_id;
       
       }*/
      if (syncData.m_syncData.GetItemCount()==0)
        return NPT_ERROR_INTERNAL;
      
      PLT_SyncStructureRef relation = *syncData.m_syncData.GetFirstItem();
      if(relation->GetType() != SYNC_RELATIONSHIP)
        return NPT_ERROR_INTERNAL;
      
      PLT_SyncRelationship *syncRelation = (PLT_SyncRelationship*)&*relation;
      
      NPT_List<PLT_PartnershipRef>& partnerships = syncRelation->GetChilds();
      if (partnerships.GetItemCount()==0)
        return NPT_ERROR_INTERNAL;
      PLT_PartnershipRef partnership = *partnerships.GetFirstItem();
      
      NPT_List<PLT_PairGroupRef>& pairGroups = partnership->GetChilds();
      if (pairGroups.GetItemCount() == 0)
        return NPT_ERROR_INTERNAL;
      PLT_PairGroupRef pairGroup = *pairGroups.GetFirstItem();
      
      PLT_SyncPair syncPair;
      syncPair.m_syncRelationshipID = syncRelation->GetID();
      syncPair.m_partnershipID = partnership->GetID();
      syncPair.m_pairGroupID = pairGroup->GetID();
      
        //This is a hack ^^
        //First: we ignore, what path the user actually wanted to sync. We hardcode wich paths to sync.
        //TODO 1: allow to  restrict what kind of content to sync (Movies/TV SHows/Music etc...) That should be easily doable.
        //TODO 2: we sync with content from hardcoded paths.... that would probably not work with 3rd party devices.
        //        Solution: every container/item has a avsc:syncable property (not implemented yet), so we should check wether the given
        //        curl is a syncable container/item (abort if not, better yet abort before invoking addsyncdata). Then recusively go through all children that have the syncable flag set to true.
        //        I tried to do this, but the Buil() function where kodi builds the MediaObject was quite confusing and i didn't know the best approach.
        //        At first we should only send syncable==true for the same paths that are hardcoded below.
        //TODO 3: allow other folders to be syncable. Problem: when a new item is added, we need to tell our partners.
        //        So everytime a new Item is added, we need to check if there is a SyncPair with one of the actors of that new item,
        //        if there is a syncPair with one of the genres, if there is a syncPair with the pg rating ....
        //        that list gets long and it would be rather inefficient. I'm open for ideas here.
      syncPair.m_remoteObjectID = "library://video/tvshows/titles/";
      if (NPT_FAILED(InvokeAddSyncPair(device,
                                       "", //TODO: empty action caller
                                       "library://video/tvshows/titles/",
                                       syncPair)))
        CGUIDialogKaiToast::QueueNotification("upnp sync", "Could not create a sync pair for tv shows"); //TODO: localize
      syncPair.m_remoteObjectID = "library://video/movies/titles/";
      if (NPT_FAILED(InvokeAddSyncPair(device,
                                       "", //TODO: empty action caller
                                       "library://video/movies/titles/",
                                       syncPair)))
        CGUIDialogKaiToast::QueueNotification("upnp sync", "Could not create a sync pair for movies"); //TODO: localize
      syncPair.m_remoteObjectID = "library://music/songs/titles/";
      if (NPT_FAILED(InvokeAddSyncPair(device,
                                       "", //TODO: empty action caller
                                       "library://music/songs/titles/",
                                       syncPair)))
        CGUIDialogKaiToast::QueueNotification("upnp sync", "Could not create a sync pair for music"); //TODO: localize
      
      
      /*
       if (mediaObject->m_contentSyncInfo.syncable == true)
       { //The content type itself can be synced...
       bool bCanSync = false;
       if (mediaObject->m_Resources.GetItemCount() != 0)
       {
       //Check if there is atleas one resurce that has syncAllowed == METADATA_ONLY or syncAllowed == ALL
       //TODO: bCanSync = true;
       
       //TODO:
       //if (check if one sync pair has a sync policy that disallowes more tha one sync pairs)
       //bCanSync = false;
       }
       else
       bCanSync = true;
       }*/
        //TODO: recusively go through all children
      
        //Todo: create sync pairs for all items!
    }
    
    
    return NPT_SUCCESS;
  }
  else
    CGUIDialogKaiToast::QueueNotification("upnp sync", "Sync relation could not be established"); //TODO: localize
  return NPT_SUCCESS;
}

NPT_Result CUPnPSyncCtrlPoint::OnGetSyncDataResponse(NPT_Result res, PLT_ActionReference& action, void* userdata)
{
  //TODO: remove this part... just for testing right now!
  NPT_String syncData;
  NPT_CHECK(action->GetArgumentValue("SyncData", syncData));
  CLog::Log(LOGDEBUG, "%s - SyncData: %s", __FUNCTION__, syncData.GetChars());
  return NPT_SUCCESS;
}
  
NPT_Result CUPnPSyncCtrlPoint::OnExchangeSyncDataResponse(NPT_Result res, PLT_ActionReference& action, void* userdata)
{
    //We have successfully exchanged sync data with our partner... now it's time to start a sync, i guess?
  /* TODO:  const char* deviceUUID = action->VerifyArguments device->GetUUID().GetChars();
   const char* serviceID = serviceCDS->GetServiceID().GetChars();
   
   CUPnPDatabase db;
   if (db.HasSyncRelationshipWith(deviceUUID, serviceID))
   {
   if (db.HasChanges(deviceUUID, serviceID))
   {
   //TODO: start a sync with that device
   }
   }
   else
   {
   //Get SyncData
   
   //Check if that device syncs any items with a partner, that we also sync with
   //if thats true:
   //Auto add a sync relationship
   //and add sync pair with all of the common items
   }*/
}

NPT_Result CUPnPSyncCtrlPoint::OnAddSyncPairResponse(NPT_Result res, PLT_ActionReference& action, void* userdata)
{
  NPT_String actionCaller;
  NPT_CHECK(action->GetArgumentValue("ActionCaller", actionCaller));
  if (!actionCaller.IsEmpty())
  {
    NPT_String ObjectID;
    NPT_CHECK(action->GetArgumentValue("ObjectID", ObjectID));
    
    if (action->GetErrorCode() >= 100)
    {
      CLog::Log(LOGWARNING, "%s - our partner failed to add a sync pair for object: %s", __FUNCTION__, ObjectID.GetChars());
      return NPT_SUCCESS;
    }
      //Let's browse the childs and add them ;)
    
    
      //Create syncpairs for all the content we have ;)
    if (ObjectID == "library://video/tvshows/titles/")
    {
    }
    else if (ObjectID == "library://video/movies/titles/")
    {
    }
    else if (ObjectID == "library://video/movies/titles/")
    {
    }
    else if (ObjectID == "library://music/songs/titles/")
    {
    }
    
      //create syncpairs for all the content our partner has ;)
    NPT_String strSyncPair;
    NPT_CHECK(action->GetArgumentValue("SyncPair", strSyncPair));
    PLT_SyncPair syncPair;
    NPT_XmlElementNode* syncPairsNode;
    NPT_CHECK(PLT_XmlHelper::Parse(strSyncPair, syncPairsNode));
    NPT_CHECK(syncPair.FromXml(syncPairsNode));
    
    XFILE::CUPnPDirectory dir;
    CFileItemList items;
    dir.GetDirectory(CURL(syncPair.m_remoteObjectID.GetChars()), items);
    
    
  }
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
  CURL url(id);
    PLT_DeviceDataReference device;
  PLT_Service* cds;
  PLT_ActionReference action;

  CLog::Log(LOGDEBUG, "UPNP: attempting to invoke UpdateObject for %s", id);

  // check this server supports UpdateObject action
  NPT_CHECK_LABEL(FindServer(url.GetHostName().c_str(), device), failed);
  NPT_CHECK_LABEL(device->FindServiceById("urn:upnp-org:serviceId:ContentDirectory", cds), failed);

  NPT_CHECK_SEVERE(m_CtrlPoint->CreateAction(
    device,
    "urn:schemas-upnp-org:service:ContentDirectory:1",
    "UpdateObject",
    action));

  NPT_CHECK_LABEL(action->SetArgumentValue("ObjectID", url.GetFileName().c_str()), failed);
  NPT_CHECK_LABEL(action->SetArgumentValue("CurrentTagValue", curr_value), failed);
  NPT_CHECK_LABEL(action->SetArgumentValue("NewTagValue", new_value), failed);

  NPT_CHECK_LABEL(m_CtrlPoint->InvokeAction(action, NULL), failed);

  CLog::Log(LOGDEBUG, "UPNP: invoked UpdateObject successfully");
  return true;

failed:
  CLog::Log(LOGINFO, "UPNP: invoking UpdateObject failed");
  return false;
}*/

}
