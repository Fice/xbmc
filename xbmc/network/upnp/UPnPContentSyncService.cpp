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
#include <Neptune.h>
#include "UPnPContentSyncService.h"
#include "UPnPDatabase.h"
#include "UPnP.h"

namespace UPNP
{

NPT_Result CUPnPContentSyncService::OnAddSyncData(const NPT_String&   SyncID,
                                                  const PLT_SyncData& SyncData)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::OnModifySyncData(const NPT_String&   SyncID,
		                                                 const PLT_SyncData& SyncData)
{
  return NPT_ERROR_NOT_IMPLEMENTED
}

NPT_Result CUPnPContentSyncService::OnDeleteSyncData(const NPT_String& SyncID)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::GetSyncStructure(const NPT_String& SyncID, PLT_SyncStructureRef& SyncData)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::OnGetSyncData(const NPT_String& SyncID,
                                                  PLT_SyncData*     SyncData)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::OnExchangeSyncData(const PLT_SyncData& LocalSyncData)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::OnAddSyncPair(const NPT_String&    ObjectID,
							                       const PLT_SyncPair& SyncPair,
                                                  NPT_String& newObjectID)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::OnModifySyncPair(const NPT_String&     ObjectID,
		                                                 const PLT_SyncPairs& SyncPair)
{
	return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::OnDeleteSyncPair(const NPT_String&     ObjectID,
		                                                 const NPT_String&     SyncID)
{
	return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::OnStartSync(const NPT_String&  SyncID)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::OnAbortSync(const NPT_String&  SyncID)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::OnGetChangeLog(const NPT_String& SyncID,
		                                               const NPT_UInt32  StartingIndex,
                                                   const NPT_UInt32  RequestedCount,
                                                   PLT_ChangeLog*    Result,
                                                   NPT_UInt32&       NumberReturned,
                                                   NPT_UInt32&       TotalMatches)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::OnResetChangeLog(const NPT_String&          SyncID,
		                                                 const PLT_ResetObjectList& ObjectIDs)
{
	return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::OnResetStatus(const NPT_String&SyncID)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::OnGetSyncStatus(const NPT_String&SyncID)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::CanSyncNow(const NPT_String& SyncID)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

}
