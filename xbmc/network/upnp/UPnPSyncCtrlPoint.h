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
#pragma once
#include <PltCtrlPoint.h>
#include <PltContentSyncService.h>
#include <string>
#include "URL.h"

namespace UPNP
{

  
class CUPnPSyncCtrlPoint : public PLT_CtrlPointListener,
                           protected PLT_ContentSyncCtrlPoint
{
public:
  CUPnPSyncCtrlPoint(NPT_Reference<PLT_CtrlPoint>& ctrlPoint, PLT_ContentSyncService* service = NULL)
    : PLT_ContentSyncCtrlPoint(ctrlPoint) {}

  //Helper functions to make invocation of content sync actions a little bit easier
  bool CreateSyncRelationship(const CURL& url);
  bool ModifySyncRelationship(const CURL& url, const PLT_SyncRelationshipRef syncRelationship);
  bool RemoveSyncRelationship(const CURL& url);
  bool AddSyncPairs(const CURL& url);
  std::string AddSyncPairRoot(const NPT_String object_id, PLT_DeviceDataReference device);

  //If a sync device comes online, we will automatically check, wheter that device needs a sync or not
  NPT_Result OnDeviceAdded(PLT_DeviceDataReference& device);
  NPT_Result OnDeviceRemoved(PLT_DeviceDataReference& device);
  
  NPT_Result OnRemoveSyncDataResponse(NPT_Result res, PLT_ActionReference& action, void* userdata);
  NPT_Result OnAddSyncDataResponse(NPT_Result res, PLT_ActionReference& action, void* userdata);
  NPT_Result OnGetSyncDataResponse(NPT_Result res, PLT_ActionReference& action, void* userdata);
  NPT_Result OnExchangeSyncDataResponse(NPT_Result res, PLT_ActionReference& action, void* userdata);
  NPT_Result OnAddSyncPairResponse(NPT_Result res, PLT_ActionReference& action, void* userdata);
  
  //Deals with the result of actions (and for some actions, even performs new actions
  NPT_Result OnActionResponse(NPT_Result res, PLT_ActionReference& action, void* userdata);
  NPT_Result OnEventNotify(PLT_Service* service, NPT_List<PLT_StateVariable*>* vars);
};

} /* namespace UPNP */
