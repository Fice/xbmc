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

namespace UPNP
{

NPT_Result CUPnPContentSyncService::OnAddSyncData(const NPT_String&   ActionCaller,
                                                  const NPT_String&   SyncID,                 /*A_ARG_TYPE_SyncID*/
                                                  const PLT_SyncData& SyncData,
                                                  PLT_SyncData*       SyncDataResult /* OUT */)
{
  
  CUPnPDatabase db;
  if (!db.Open())
    return NPT_ERROR_INTERNAL;
  
  if(!db.AddSyncRelationship(&SyncData.m_syncRelationship))
    return NPT_ERROR_INTERNAL;
  
  *SyncDataResult = SyncData;
  
  return NPT_SUCCESS;
}

NPT_Result CUPnPContentSyncService::OnModifySyncData(const NPT_String&   ActionCaller,
		                                                 const NPT_String&   SyncID,
		                                                 const PLT_SyncData& SyncData)
{
	return NPT_ERROR_NOT_IMPLEMENTED; //TODO:
}

NPT_Result CUPnPContentSyncService::OnDeleteSyncData(const NPT_String& ActionCaller,
		                                                 const NPT_String& SyncID)
{
  return NPT_ERROR_NOT_IMPLEMENTED; //TODO:
}

NPT_Result CUPnPContentSyncService::OnGetSyncData(const NPT_String& SyncID,
                                                  PLT_SyncData*     SyncData)
{
  return NPT_ERROR_NOT_IMPLEMENTED; 
}

NPT_Result CUPnPContentSyncService::OnExchangeSyncData(const PLT_SyncData& LocalSyncData,
                                                       PLT_SyncData*       RemoteSyncData)
{
  return NPT_ERROR_NOT_IMPLEMENTED; 
}

NPT_Result CUPnPContentSyncService::OnAddSyncPair(const NPT_String&    ActionCaller,
                                                  const NPT_String&    ObjectID,
	                                              	const PLT_SyncPairs& SyncPair)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}
	
NPT_Result CUPnPContentSyncService::OnModifySyncPair(const NPT_String&     ActionCaller,
		                                                 const NPT_String&     ObjectID,
		                                                 const PLT_SyncPairs& SyncPair)
{
	return NPT_ERROR_NOT_IMPLEMENTED;
}
	 
NPT_Result CUPnPContentSyncService::OnDeleteSyncPair(const NPT_String&     ActionCaller,
		                                                 const NPT_String&     ObjectID,
		                                                 const NPT_String&     SyncID)
{
	return NPT_ERROR_NOT_IMPLEMENTED;
}
	
NPT_Result CUPnPContentSyncService::OnStartSync(const NPT_String&  ActionCaller,
		                                            const NPT_String&  SyncID)
{
	return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result CUPnPContentSyncService::OnAbortSync(const NPT_String&  ActionCaller,
		                                            const NPT_String&  SyncID)
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

}
