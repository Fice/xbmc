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

namespace UPNP
{

/* TODO: not sure where to put this
bool CUPnPCtrlPointListener::HasSyncData(NPT_String deviceUUID)
{
  //first check m_cachedNeedsSync
  //if key not found: ask all DBs
  return true; //TODO:
}*/
  
bool CUPnPSyncCtrlPoint::CreateSyncRelationship(const CURL& url)
{
  std::string DeviceUUID = url.GetHostName();
  
  XFILE::CUPnPDirectory dir;
  CFileItemList items;
  if(!dir.GetDirectory(url, items))
  {
    CLog::Log(LOGERROR, "%s - couldn't create a sync relationship with partner %s because GetDirectory for path %s failed.", __FUNCTION__, DeviceUUID.c_str(), url.GetWithoutUserDetails().c_str());
    return false;
  }

  NPT_String actionCaller;
  NPT_String syncID;
  
  bool shortcut = false;
  if (shortcut)
  {
      //TODO:
  }
  else 
  {
    actionCaller = "";
    syncID = "";
  }

  PLT_DeviceDataReference device;
  if(NPT_FAILED(CUPnP::GetInstance()->m_MediaBrowser->FindServer(DeviceUUID.c_str(), device) && !device.IsNull()))
    return false;

  CUPnPServer* server = CUPnP::GetInstance()->GetServer();
  if (!server)
    return false;

  //Create the sync relationship that we want to establish
  PLT_PairGroup pairGroup;

  PLT_SyncPolicy policy;
  policy.m_policyType = "TRACKING";
  policy.m_delProtection.SetValue(false);
  policy.m_autoObjAdd.SetValue(true);

  PLT_Partnership partnership;
  
  partnership.m_partner1.m_id = 1;
  partnership.m_partner1.m_strDeviceUDN = server->GetUUID();
  PLT_Service *syncService;
  NPT_CHECK(server->FindServiceByType("urn:schemas-upnp-org:service:ContentSync:1", syncService));
  partnership.m_partner1.m_strServiceID = syncService->GetServiceID();

  partnership.m_partner2.m_id = 2;
  partnership.m_partner2.m_strDeviceUDN = device->GetUUID();
  NPT_CHECK(device->FindServiceByType("urn:schemas-upnp-org:service:ContentSync:1", syncService));
  partnership.m_partner2.m_strServiceID = syncService->GetServiceID();

  partnership.m_policy = policy;
  partnership.m_pairGroups.Add(pairGroup);

  PLT_SyncRelationship relationship;
  relationship.m_strTitle = "Sync between "+server->GetFriendlyName()+" and " + device->GetFriendlyName();
  relationship.m_partnerships.Add(partnership);

  PLT_SyncData syncData;
  syncData.m_syncRelationship = relationship;

  PLT_SyncData syncDataResult;
  if(NPT_FAILED(InvokeAddSyncData(device,
                                  actionCaller, 
                                  syncID, 
                                  syncData, 
                                  syncDataResult)))
     return false;
    
  return true;
}
  
bool CUPnPSyncCtrlPoint::RemoveSyncRelationship(const CURL& url)
{
  return false;
}

NPT_Result CUPnPSyncCtrlPoint::OnDeviceAdded(PLT_DeviceDataReference& device)
{
  PLT_Service *serviceCDS;
  
  NPT_String type = "urn:schemas-upnp-org:service:ContentSync:1";
  if (NPT_FAILED(device->FindServiceByType(type, serviceCDS))) {
    return NPT_SUCCESS; //Not possible to sync with that device anyway!
  }
  
  //check if we already have a sync relationship with that device!
  CUPnPDatabase db;
  if(!db.HasSyncRelationshipWith(device->GetUUID().GetChars(), serviceCDS->GetServiceID().GetChars()))
    return NPT_ERROR_INVALID_STATE;
  
  Changesets changes;
  //void GetChangesets(std::string DeviceUUID, Changesets& result)
  if (!changes.Empty())
  {
    //TODO: start a sync with that device
  }
  return NPT_SUCCESS;
}

NPT_Result CUPnPSyncCtrlPoint::OnDeviceRemoved(PLT_DeviceDataReference& device)
{
  return NPT_SUCCESS;
}

NPT_Result CUPnPSyncCtrlPoint::OnActionResponse(NPT_Result res, PLT_ActionReference& action, void* userdata)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
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