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
  CUPnPContentSyncService() : bSyncInProgress(false) {}
  virtual NPT_Result OnAddSyncData(const NPT_String&   SyncID,
                                   const PLT_SyncData& SyncData);
  virtual NPT_Result OnModifySyncData(const NPT_String&   SyncID,
                                      const PLT_SyncData& SyncData);
  virtual NPT_Result OnDeleteSyncData(const NPT_String& SyncID);

  virtual NPT_Result OnGetSyncData(const NPT_String& SyncID,
                                   PLT_SyncData*     SyncData);
  virtual NPT_Result OnExchangeSyncData(const PLT_SyncData& LocalSyncData);
  virtual NPT_Result OnAddSyncPair(const NPT_String&    ObjectID,
                                   const PLT_SyncPair& SyncPair,
                                   NPT_String& newObjectID);
  virtual NPT_Result OnModifySyncPair(const NPT_String&    ObjectID,
                                      const PLT_SyncPairs& SyncPair);
  virtual NPT_Result OnDeleteSyncPair(const NPT_String&     ObjectID,
                                      const NPT_String&     SyncID);
  virtual NPT_Result OnStartSync(const NPT_String&  SyncID);
  virtual NPT_Result OnAbortSync(const NPT_String&  SyncID);
  virtual NPT_Result OnGetChangeLog(const NPT_String& SyncID,
                                    const NPT_UInt32  StartingIndex,
                                    const NPT_UInt32  RequestedCount,
                                    PLT_ChangeLog*    Result,
                                    NPT_UInt32&       NumberReturned,
                                    NPT_UInt32&       TotalMatches);
  virtual NPT_Result OnResetChangeLog(const NPT_String&         SyncID,
                                      const PLT_ResetObjectList& ObjectIDs);
  virtual NPT_Result OnResetStatus(const NPT_String&SyncID);
  virtual NPT_Result OnGetSyncStatus(const NPT_String&SyncID);

  virtual NPT_Result CanSyncNow(const NPT_String& SyncID);
  virtual NPT_Result GetSyncStructure(const NPT_String&, PLT_SyncStructureRef&);

  //Just used for error testing... returning false will just mean: everything ok
  virtual bool HasSyncPair(PLT_SyncData& syncData, NPT_String objectID) { return false; }
protected:

  bool bSyncInProgress;
};

} /* namespace UPNP */
