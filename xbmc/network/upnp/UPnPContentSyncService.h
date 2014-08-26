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

#include <../lib/libUPnP/Neptune/Source/Core/Neptune.h>
#include <../lib/libUPnP/Platinum/Source/Platinum/Platinum.h>

namespace UPNP
{

class CUPnPContentSyncService : public PLT_ContentSyncDelegate
{
public:
  virtual NPT_Result OnAddSyncData(const NPT_String&   ActionCaller,
                                   const NPT_String&   SyncID,                 /*A_ARG_TYPE_SyncID*/
                                   const PLT_SyncData& SyncData,
                                   PLT_SyncData*       SyncDataResult /* OUT */);
  virtual NPT_Result OnModifySyncData(const NPT_String&   ActionCaller,
                                      const NPT_String&   SyncID,          /*A_ARG_TYPE_SyncID*/
                                      const PLT_SyncData& SyncData);
  virtual NPT_Result OnDeleteSyncData(const NPT_String& ActionCaller,
                                      const NPT_String& SyncID            /*A_ARG_TYPE_SyncID*/);
	//OUT: A_ARG_TYPE_SyncData SyncData
  virtual NPT_Result OnGetSyncData(const NPT_String& SyncID,                 /*A_ARG_TYPE_SyncID*/
                                   PLT_SyncData*     SyncData      /* OUT*/);
	//OUT: A_ARG_TYPE_SyncData RemoteSyncData
  virtual NPT_Result OnExchangeSyncData(const PLT_SyncData& LocalSyncData,
                                        PLT_SyncData*       RemoteSyncData /* OUT */);
  virtual NPT_Result OnAddSyncPair(const NPT_String&    ActionCaller,
                                   const NPT_String&    ObjectID,          /*A_ARG_TYPE_ObjectID*/
                                   const PLT_SyncPairs& SyncPair);
  virtual NPT_Result OnModifySyncPair(const NPT_String&    ActionCaller,
                                      const NPT_String&    ObjectID,       /*A_ARG_TYPE_ObjectID*/
                                      const PLT_SyncPairs& SyncPair);
  virtual NPT_Result OnDeleteSyncPair(const NPT_String&     ActionCaller,
                                      const NPT_String&     ObjectID,      /*A_ARG_TYPE_ObjectID*/
                                      const NPT_String&     SyncID         /*A_ARG_TYPE_SyncID*/);
  virtual NPT_Result OnStartSync(const NPT_String&  ActionCaller,
                                 const NPT_String&  SyncID                 /*A_ARG_TYPE_SyncID*/);
  virtual NPT_Result OnAbortSync(const NPT_String&  ActionCaller,
                                 const NPT_String&  SyncID                 /*A_ARG_TYPE_SyncID*/);
	//OUT: A_ARG_TYPE_COUNT NumberReturned
	//OUT: A_ARG_TYPE_Count TotalMatches
  virtual NPT_Result OnGetChangeLog(const NPT_String& SyncID,     /*A_ARG_TYPE_SyncID*/
                                    const NPT_UInt32  StartingIndex, /*A_ARG_TYPE_Index*/
                                    const NPT_UInt32  RequestedCount, /*A_ARG_TYPE_Count*/
                                    PLT_ChangeLog*    Result,         /* OUT */
                                    NPT_UInt32&       NumberReturned, /* OUT */
                                    NPT_UInt32&       TotalMatches);
  virtual NPT_Result OnResetChangeLog(const NPT_String&         SyncID,   /*A_ARG_TYPE_SyncID*/
                                      const PLT_ResetObjectList& ObjectIDs);
  virtual NPT_Result OnResetStatus(const NPT_String&SyncID                 /*A_ARG_TYPE_SyncID*/);
	//OUT: A_ARG_TYPE_SyncStatus
  virtual NPT_Result OnGetSyncStatus(const NPT_String&SyncID /*A_ARG_TYPE_SyncID*/);
protected:
};

} /* namespace UPNP */

