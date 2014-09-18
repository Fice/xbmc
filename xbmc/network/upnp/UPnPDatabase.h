/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include <Neptune.h>
#include <Platinum.h>
#include "dbwrappers/Database.h"

class PLT_SyncRelationship;
class PLT_Partnership;

namespace dbiplus
{
  class field_value;
  typedef std::vector<field_value> sql_record;
}

namespace UPNP
{

struct ImportFolder
{
  unsigned int primaryKey;
  std::string localFolder;
  std::string remoteFolder;
  std::auto_ptr<PLT_PairGroup> pairGroup;
  PLT_SyncPolicy syncPolicy;
};

class CUPnPDatabase : public CDatabase
{
public:
  CUPnPDatabase();
  virtual ~CUPnPDatabase();
  virtual bool Open();
    //TODO: make 'const std::string&' into 'const char* const'.... i don't think we need std::string anywhere,
    //but we have overhead becuase we have to create a std::string from an NPT_String everytime the UPnP stuff call us

  bool AddSyncRelationship(const PLT_SyncRelationship* const);
  bool AddPartnerships(const NPT_List<PLT_PartnershipRef>* const partnerships, const int relationshipID);
  bool AddPartnership(const PLT_Partnership* const partnership, const int relationshipID);
  bool AddPartnership(const PLT_Partnership* const partnership, const std::string& relationshipUUID);
  bool AddPartner(const PLT_Partner& partner, const int partnershipID);
  bool AddPairGroups(const NPT_List<PLT_PairGroupRef>& pairGroups, const int partnershipID);
  bool AddPairGroup(const PLT_PairGroup* const, const int partnershipID);
  bool AddPairGroup(const PLT_PairGroup* const, const std::string& partnershipUUID);

  bool ModifySyncRelationship(const PLT_SyncRelationship* const syncRelationship, bool recursive = false);
  bool ModifyPartnerships(const NPT_List<PLT_PartnershipRef>& pairGroups, bool recursive = false);
  bool ModifyPartnership(const PLT_Partnership* const partnership, bool recursive = false);
  bool ModifyPairGroups(const NPT_List<PLT_PairGroupRef>& pairGroups);
  bool ModifyPairGroup(const PLT_PairGroup* const pairGroup);

  bool DeleteSyncData(const std::string& SyncID);

  bool HasSyncRelationshipWith(const char* deviceUDN, const char* serviceID);
  bool HasSyncRelationshipWith(const char* deviceUDN);
  bool HasChanges(const char* deviceUDN, const char* serviceID);

  bool GetSyncRelationshipForDevice(const std::string& DeviceUUID, PLT_SyncRelationshipRef& syncRelationship, bool includePairGroups=false);
  bool GetSyncRelationship(const std::string& syncID, PLT_SyncRelationshipRef& syncRelationship, bool includePairGroups=false);
  bool GetSyncPartnership(const std::string& syncID, PLT_PartnershipRef& syncPartnership, bool includePairGroups=false);
  bool GetPairGroup(const std::string& syncID, PLT_PairGroupRef& syncPairGroup);
  bool GetPairGroups(const std::string& parentSyncID, NPT_List<PLT_PairGroupRef>& pairGroups);

  bool AddImportFolder(const std::string& remoteObjectID, const PLT_SyncPair& SyncPair);
  bool RemoveImportFolder(const PLT_SyncPair& SyncPair);
  bool HasImportFolder(const std::string& objectID, const std::string& remoteObjectID, const std::string pairGroupGUID);
  bool GetImportFolder(const std::string& objectID, const std::string& remoteObjectID, const std::string pairGroupGUID, ImportFolder& importFolder);

  bool GetAllSyncData(PLT_SyncData* result, bool includePairGroups = false);
  /*
  static int ToSQLValue(const PLT_OptionalBool& b);
  static PLT_OptionalBool FromSQLValue(const int i);
  */
  int UUIDToPrimaryKey(const std::string& table, const std::string& UUID);
protected:
  bool FillSyncRelationship(dbiplus::Dataset* const ds, PLT_SyncRelationship& syncRelationship, bool includePairGroups=false);
  bool FillPartnership(dbiplus::Dataset* const ds, PLT_Partnership& syncRelationship, bool includePairGroups=false);
  bool FillPairGroup(dbiplus::Dataset* const ds, PLT_PairGroup& syncRelationship);


  virtual void CreateTables();
  virtual void CreateAnalytics();
  virtual void UpdateTables(int version);
  virtual int GetSchemaVersion() const { return 1; };
  const char *GetBaseDBName() const { return "UPnP"; };
};

}
