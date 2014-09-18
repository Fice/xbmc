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
 class AddSyncDataVisitor : public PLT_SyncStructureConstVisitor
 {
 public:
   AddSyncDataVisitor(const NPT_String& parentID) : parentID(parentID) {}
   virtual NPT_Result Visit(const PLT_SyncRelationship* const relationship)
   {
     if (!parentID.IsEmpty())
       return NPT_ERROR_INTERNAL;
     CUPnPDatabase db;
     if (!db.Open())
       return NPT_ERROR_INTERNAL;
     if (!db.AddSyncRelationship(&*relationship))
       return NPT_ERROR_INTERNAL;
     return NPT_SUCCESS;
   }
   virtual NPT_Result Visit(const PLT_Partnership* const partnership)
   {
     if (parentID.IsEmpty())
       return NPT_ERROR_INTERNAL;
     CUPnPDatabase db;
     if (!db.Open())
       return NPT_ERROR_INTERNAL;
     if (!db.AddPartnership(partnership, parentID.GetChars()))
       return NPT_ERROR_INTERNAL;
     return NPT_SUCCESS;
   }
   virtual NPT_Result Visit(const PLT_PairGroup* const pairGroup)
   {
     if (parentID.IsEmpty())
       return NPT_ERROR_INTERNAL;
     CUPnPDatabase db;
     if (!db.Open())
       return NPT_ERROR_INTERNAL;
     if (!db.AddPairGroup(pairGroup, parentID.GetChars()))
       return NPT_ERROR_INTERNAL;
     return NPT_SUCCESS;
   }
 protected:
   NPT_String parentID;
 };

 class ModifySyncDataVisitor : public PLT_SyncStructureConstVisitor
 {
 public:
   virtual NPT_Result Visit(const PLT_SyncRelationship* const relationship)
   {
     CUPnPDatabase db;
     if (!db.Open())
       return NPT_ERROR_INTERNAL;
     if (!db.ModifySyncRelationship(relationship))
       return NPT_ERROR_INTERNAL;
     return NPT_SUCCESS;
   }
   virtual NPT_Result Visit(const PLT_Partnership* const partnership)
   {
     CUPnPDatabase db;
     if (!db.Open())
       return NPT_ERROR_INTERNAL;
     if (!db.ModifyPartnership(partnership))
       return NPT_ERROR_INTERNAL;
     return NPT_SUCCESS;
   }
   virtual NPT_Result Visit(const PLT_PairGroup* const pairGroup)
   {
     CUPnPDatabase db;
     if (!db.Open())
       return NPT_ERROR_INTERNAL;
     if (!db.ModifyPairGroup(pairGroup))
       return NPT_ERROR_INTERNAL;
     return NPT_SUCCESS;
   }
 };

NPT_Result CUPnPContentSyncService::OnAddSyncData(const NPT_String&   SyncID,
                                                  const PLT_SyncData& SyncData)
{
  CUPnPDatabase db;
  if (!db.Open())
    return NPT_ERROR_INTERNAL;

  AddSyncDataVisitor visitor(SyncID);
  NPT_CHECK(SyncData.Visit(&visitor, false));

  return NPT_SUCCESS;
}

NPT_Result CUPnPContentSyncService::OnModifySyncData(const NPT_String&   SyncID,
		                                                 const PLT_SyncData& SyncData)
{
  ModifySyncDataVisitor visitor;
  return (SyncData.Visit(&visitor, false)); //Hmm... I don't know if you could send a Partnership, where also the childs have changed...
                                            //I suspect no?!? so no recursion here, but that's rather unclear
}

NPT_Result CUPnPContentSyncService::OnDeleteSyncData(const NPT_String& SyncID)
{
  CUPnPDatabase db;
  if (!db.Open())
    return NPT_ERROR_INTERNAL;

  if(!db.DeleteSyncData(SyncID.GetChars()))
    return NPT_ERROR_INTERNAL;

  return NPT_SUCCESS;
}

NPT_Result CUPnPContentSyncService::GetSyncStructure(const NPT_String& SyncID, PLT_SyncStructureRef& SyncData)
{
  CUPnPDatabase db;
  if(!db.Open())
    return NPT_ERROR_INTERNAL;
  if (SyncID.IsEmpty())
    return NPT_ERROR_INTERNAL;

  PLT_SyncRelationshipRef rel;
  if (!db.GetSyncRelationship(SyncID.GetChars(), rel))
  {
    PLT_PartnershipRef partnership;
    if (!db.GetSyncPartnership(SyncID.GetChars(), partnership))
    {
      PLT_PairGroupRef pairGroup;
      if (!db.GetPairGroup(SyncID.GetChars(), pairGroup))
        return NPT_ERROR_INTERNAL;
      else
        SyncData = PLT_SyncStructureRef(pairGroup);
    }
    else
      SyncData = PLT_SyncStructureRef(partnership);
  }
  else
    SyncData = PLT_SyncStructureRef(rel);

  return NPT_SUCCESS;
}

NPT_Result CUPnPContentSyncService::OnGetSyncData(const NPT_String& SyncID,
                                                  PLT_SyncData*     SyncData)
{
  CUPnPDatabase db;
  if(!db.Open())
    return NPT_ERROR_INTERNAL;

  if (SyncID.IsEmpty())
  {
    if (!db.GetAllSyncData(SyncData))
      return NPT_ERROR_INTERNAL;
  }
  else
  {
    PLT_SyncStructureRef ref;
    NPT_CHECK(GetSyncStructure(SyncID, ref));
    SyncData->m_syncData.Add(PLT_SyncRelationshipRef(ref));
  }
  return NPT_SUCCESS;
}

NPT_Result CUPnPContentSyncService::OnExchangeSyncData(const PLT_SyncData& LocalSyncData)
{
  return NPT_SUCCESS; //libUPnP takes care of everything.
}

NPT_Result CUPnPContentSyncService::OnAddSyncPair(const NPT_String&    ObjectID,
							                       const PLT_SyncPair& SyncPair,
                                                  NPT_String& newObjectID)
{
  CUPnPDatabase db;
  if(!db.Open())
    return NPT_ERROR_INTERNAL;

  if (SyncPair.m_remoteObjectID == "library://video/movies/titles" ||
      SyncPair.m_remoteObjectID == "library://video/tvshows/titles" ||
      SyncPair.m_remoteObjectID == "library://music/songs")
  {
    if(!db.AddImportFolder(ObjectID.GetChars(), SyncPair))
      return NPT_ERROR_INTERNAL;
    newObjectID = SyncPair.m_remoteObjectID;
    return NPT_SUCCESS;
  }
  else if (false) //GetItem
  {
    return NPT_ERROR_INTERNAL;
  }

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
