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
  CUPnPSyncCtrlPoint(NPT_Reference<PLT_CtrlPoint>& ctrlPoint) : PLT_ContentSyncCtrlPoint(ctrlPoint) {}

  bool CreateSyncRelationship(const CURL& url);
  bool RemoveSyncRelationship(const CURL& url);

  NPT_Result OnDeviceAdded(PLT_DeviceDataReference& device);
  NPT_Result OnDeviceRemoved(PLT_DeviceDataReference& device);
  NPT_Result OnActionResponse(NPT_Result res, PLT_ActionReference& action, void* userdata);
  NPT_Result OnEventNotify(PLT_Service* service, NPT_List<PLT_StateVariable*>* vars);



protected:
  //TODO: not sure where to put this
  //std::map<NPT_String, bool> m_cachedNeedsSync; //Stores wheter the devic needs a sync. (the string represents the UUID). 
                                                //if key not found, we have to query the DB.
                                                //TODO: hook up the DB to autoset that value, when changelogs are created
};

} /* namespace UPNP */
