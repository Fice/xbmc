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

#include "UPnPDatabase.h"
#include "UPnP.h"
#include "UPnPServer.h"
#include "utils/log.h"
#include "dbwrappers/dataset.h"
#include "Platinum.h"

namespace UPNP
{

CUPnPDatabase::CUPnPDatabase()
{
}

CUPnPDatabase::~CUPnPDatabase()
{
}
/*
int CUPnPDatabase::ToSQLValue(const PLT_OptionalBool& b)
{
  if (b.IsValid())
    return 0;
  if (b.IsFalse())
    return 1;
  return 2;
}

PLT_OptionalBool CUPnPDatabase::FromSQLValue(const int i)
{
  PLT_OptionalBool result;
  if (i==0)
    return result;
  if (i==1)
    result.SetValue(false);
  else
    result.SetValue(true);
  return result;
}*/

bool CUPnPDatabase::Open()
{
  return CDatabase::Open();
}

bool CUPnPDatabase::AddSyncRelationship(const PLT_SyncRelationship* relationship)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;
  if (NULL == relationship) return false;
  if (relationship->GetID().IsEmpty()) return false;
    //TODO: make sure such a partnership doesn't already exists.

  BeginTransaction();

  try {
    std::string sql = PrepareSQL("INSERT INTO SyncRelationship (lastSync, lastUpdateID, active, title, GUID) "
                                 "VALUES (0, 0, %s, '%s', '%s')",
                                 relationship->GetActiveState().toString().GetChars(),
                                 relationship->GetTitle().GetChars(),
                                 relationship->GetID().GetChars());

    m_pDS->exec(sql.c_str());
    int guid = (int)m_pDS->lastinsertid();

    const NPT_List<PLT_PartnershipRef>& partnerships = relationship->GetChilds();
    if (!AddPartnerships(&partnerships ,guid))
    {
      RollbackTransaction();
      return false;
    }
    CommitTransaction();
  }
  catch (...) {
    //TODO: find out if a rollback is automatically done when a excpetion is thrown?!?
    RollbackTransaction();
    CLog::Log(LOGERROR, "%s failed on relationship '%s'", __FUNCTION__, relationship->GetTitle().GetChars());

    return false;
  }

  return true;
}

bool CUPnPDatabase::AddPartnerships(const NPT_List<PLT_PartnershipRef>* const partnerships, const int relationshipID)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  bool ownTransaction = !InTransaction();

  if (ownTransaction)
    BeginTransaction();

  NPT_List<PLT_PartnershipRef>::Iterator partnership = partnerships->GetFirstItem();
  while (partnership)
  {
    if (!AddPartnership(&**partnership, relationshipID))
    {
      if (ownTransaction)
        RollbackTransaction();
      return false;
    }
    ++partnership;
  }

  if (ownTransaction)
    CommitTransaction();

  return true;
}

bool CUPnPDatabase::AddPartnership(const PLT_Partnership* partnership, const std::string& relationshipUUID)
{
  int id = UUIDToPrimaryKey("SyncRelationship", relationshipUUID);
  if (id == 0)
    return false;
  return AddPartnership(partnership, id);
}

bool CUPnPDatabase::AddPartnership(const PLT_Partnership* partnership, const int relationshipID)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  bool ownTransaction = !InTransaction();

  if (ownTransaction)
    BeginTransaction();

  try {
    const PLT_SyncPolicy& policy = partnership->GetPolicy();
    std::string sql = PrepareSQL("INSERT INTO SyncPartnership (updateID, syncRelationship, syncType, priorityPartner, bDelProtection, bAutoObjAdd, GUID, active) "
                                 "VALUES (0, %i, '%s', %i, %i, %i, '%s', %i)", relationshipID,
                                 policy.m_policyType.GetChars(),
                                 policy.m_priorityPartnerID,
                                 policy.m_delProtection.GetValue(),
                                 policy.m_autoObjAdd.GetValue(),
                                 partnership->GetID().GetChars(),
                                 partnership->GetActiveState().GetValue()
                                 );
    m_pDS->exec(sql.c_str());
    int id = (int)m_pDS->lastinsertid();



    //A partnership has two partners. One of the partners is always us. We don't need to store that we
    //are a partner. So figure out wich partner we have to store
    const PLT_Partner& partner = (partnership->GetPartner1().m_strDeviceUDN==CUPnP::GetServer()->GetUUID())
                                  ? partnership->GetPartner2() : partnership->GetPartner1();
    if (!AddPartner(partner, id))
    {
      if (ownTransaction)
        RollbackTransaction();
      return false;
    }
    if (!AddPairGroups(partnership->GetChilds(), id))
    {
      if (ownTransaction)
        RollbackTransaction();
      return false;
    }
    if (ownTransaction)
      CommitTransaction();
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s - Add partnership failed for partner: %s", __FUNCTION__, partnership->GetID().GetChars());
    return false;
  }

  return true;
}

bool CUPnPDatabase::AddPartner(const PLT_Partner& partner, const int partnershipID)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;
  try {
    std::string sql = PrepareSQL("INSERT INTO SyncPartner (syncPartnership, partnerID, lastSync, DeviceUDN, ServiceID) "
                                 "VALUES (%i, %i, 0, '%s', '%s')", partnershipID, partner.m_id,  partner.m_strDeviceUDN.GetChars(), partner.m_strServiceID.GetChars()
                                 );
    m_pDS->exec(sql.c_str());
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s - Add partner failed for partner: %s", __FUNCTION__, partner.m_strDeviceUDN.GetChars());
    return false;
  }

  return true;
}

bool CUPnPDatabase::AddPairGroups(const NPT_List<PLT_PairGroupRef>& pairGroups, const int partnershipID)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  bool ownTransaction = !InTransaction();

  try {
    if (ownTransaction)
      BeginTransaction();

    NPT_List<PLT_PairGroupRef>::Iterator pairGroup = pairGroups.GetFirstItem();
    while (pairGroup)
    {
      if (!AddPairGroup(&**pairGroup, partnershipID))
      {
        if (ownTransaction)
          RollbackTransaction();
        return false;
      }
      ++pairGroup;
    }

    if (ownTransaction)
      CommitTransaction();
  }
  catch (...)
  {
    return false;
  }
  return true;
}

bool CUPnPDatabase::AddPairGroup(const PLT_PairGroup* pairGroup, const std::string& partnershipUUID)
{
  int id = UUIDToPrimaryKey("SyncPartnership", partnershipUUID);
  if (id == 0)
    return false;
  return AddPairGroup(pairGroup, id);
}

bool CUPnPDatabase::AddPairGroup(const PLT_PairGroup* pairGroup, const int partnershipID)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  try {
    const PLT_SyncPolicy& policy = pairGroup->GetPolicy();
    std::string sql = PrepareSQL("INSERT INTO SyncPairGroup (GUID, SyncPartnership, active, updateID, syncType, priorityPartner, bDelProtection, bAutoObjAdd) "
                                 "VALUES ('%s', %i, %i, %i, '%s', %i, %i, %i)", pairGroup->GetID().GetChars(),
                                                                                partnershipID,
                                                                                pairGroup->GetActiveState().GetValue(),
                                                                                pairGroup->GetUpdateID(),
                                                                                policy.m_policyType.GetChars(),
                                                                                policy.m_priorityPartnerID,
                                                                                policy.m_delProtection.GetValue(),
                                                                                policy.m_autoObjAdd.GetValue()

                                 );
    m_pDS->exec(sql);
  }
  catch (...)
  {
    CLog::Log(LOGWARNING, "%s - Adding several pair groups failed for partnership: %i", __FUNCTION__, partnershipID);
    return false;
  }
  return true;
}

bool CUPnPDatabase::ModifySyncRelationship(const PLT_SyncRelationship* const syncRelationship, bool recursive /* = false */)
{
  bool ownTransaction = !InTransaction();

  try {
    if (ownTransaction)
      BeginTransaction();

    std::string sql = PrepareSQL("UPDATE SyncRelationship SET "
                                     "lastUpdateID = %d, "
                                     "active = %d, "
                                     "title = '%s' "
                                 "WHERE GUID = '%s'",
                                 syncRelationship->GetUpdateID(),
                                 syncRelationship->GetActiveState().GetValue(),
                                 syncRelationship->GetTitle().GetChars(),
                                 syncRelationship->GetID().GetChars()
                                );
    m_pDS->exec(sql);

    const NPT_List<PLT_PartnershipRef>& partnerships = syncRelationship->GetChilds();
    if (recursive && !ModifyPartnerships(partnerships, recursive))
    {
      if (ownTransaction)
        RollbackTransaction();
      return false;
    }

    if (ownTransaction)
      CommitTransaction();
  }
  catch (...)
  {
    if (ownTransaction)
      RollbackTransaction();
    return false;
  }
  return true;
}

bool CUPnPDatabase::ModifyPartnerships(const NPT_List<PLT_PartnershipRef>& partnerships, bool recursive /* = false */)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  bool ownTransaction = !InTransaction();

  try {
    if (ownTransaction)
      BeginTransaction();

    NPT_List<PLT_PartnershipRef>::Iterator partnership = partnerships.GetFirstItem();
    while (partnership)
    {
      if (!ModifyPartnership(&**partnership, recursive))
      {
        if (ownTransaction)
          RollbackTransaction();
        return false;
      }
      ++partnership;
    }

    if (ownTransaction)
      CommitTransaction();
  }
  catch (...)
  {
    if (ownTransaction)
      RollbackTransaction();
    return false;
  }
  return true;
}

bool CUPnPDatabase::ModifyPartnership(const PLT_Partnership* const partnership, bool recursive /* = false */)
{
  bool ownTransaction = !InTransaction();

  try {
    if (ownTransaction)
      BeginTransaction();

    //TODO:

    if (recursive && !ModifyPairGroups(partnership->GetChildren()))
    {
      if (ownTransaction)
        RollbackTransaction();
      return false;
    }
  }
  catch (...)
  {
    if (ownTransaction)
      RollbackTransaction();
    return false;
  }
  return true;
}

bool CUPnPDatabase::ModifyPairGroups(const NPT_List<PLT_PairGroupRef>& pairGroups)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  bool ownTransaction = !InTransaction();

  try {
    if (ownTransaction)
      BeginTransaction();

    NPT_List<PLT_PairGroupRef>::Iterator pairGroup = pairGroups.GetFirstItem();
    while (pairGroup)
    {
      if (!ModifyPairGroup(&**pairGroup))
      {
        if (ownTransaction)
          RollbackTransaction();
        return false;
      }
      ++pairGroup;
    }

    if (ownTransaction)
      CommitTransaction();
  }
  catch (...)
  {
    if (ownTransaction)
      RollbackTransaction();
    return false;
  }
  return true;
}

bool CUPnPDatabase::ModifyPairGroup(const PLT_PairGroup* pairGroup)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  try {
    PLT_SyncPolicy policy = pairGroup->GetPolicy();
    std::string sql = PrepareSQL("INSERT INTO SyncPairGroup "
                                 "SET active=%i, "
                                     "updateID=%i, "
                                     "syncType='%s', "
                                     "priorityPartner=%i, "
                                     "bDelProtection=%i, "
                                     "bAutoObjAdd=%i "
                                 "WHERE GUID = '%s'",
                                 pairGroup->GetActiveState().GetValue(),
                                 pairGroup->GetUpdateID(),
                                 policy.m_policyType.GetChars(),
                                 policy.m_priorityPartnerID,
                                 policy.m_delProtection.GetValue(),
                                 policy.m_autoObjAdd.GetValue(),
                                 pairGroup->GetID().GetChars());
    m_pDS->exec(sql);
  }
  catch (...)
  {
    CLog::Log(LOGWARNING, "%s - Modifying pair group failed for pair group: %s", __FUNCTION__, pairGroup->GetID().GetChars());
    return false;
  }
  return true;
}

bool CUPnPDatabase::DeleteSyncData(const std::string& syncID)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  try
  {
    std::string sql = PrepareSQL("DELETE FROM SyncRelationship WHERE GUID='%s'", syncID.c_str());
    m_pDS->exec(sql);
    sql = PrepareSQL("DELETE FROM SyncPartnership WHERE GUID='%s'", syncID.c_str());
    m_pDS->exec(sql);
    sql = PrepareSQL("DELETE FROM SyncPairGroup WHERE GUID='%s'", syncID.c_str());
    m_pDS->exec(sql);
  }
  catch (...)
  {
    CLog::Log(LOGWARNING, "%s - Deleting Sync Relationship with sync ID '%s' failed", __FUNCTION__, syncID.c_str());
      //TODO: log sth.
    return false;
  }
  return true;
}

bool CUPnPDatabase::FillPartnership(dbiplus::Dataset* const ds, PLT_Partnership& syncPartnership, bool includePairGroups /*=false*/)
{
  CUPnPServer* server = CUPnP::GetInstance()->GetServer();
  if (!server)
    return false;

  PLT_Service *syncService;
  NPT_CHECK(server->FindServiceByType("urn:schemas-upnp-org:service:ContentSync:1", syncService));

  PLT_Partner us, them;
    //One of the partners is always us. We don't store that, so we have to manually at us as a partner!
  us.m_strDeviceUDN = server->GetUUID();

  us.m_strServiceID = syncService->GetServiceID();

  unsigned int partnerID = ds->get_field_value("SyncPartner.partnerID").get_asUInt();
  them.m_strDeviceUDN = NPT_String(ds->get_field_value("SyncPartner.DeviceUDN").get_asString().c_str());
  them.m_strServiceID = NPT_String(ds->get_field_value("SyncPartner.ServiceID").get_asString().c_str());
  if (partnerID==1)
  {
    syncPartnership.SetPartner1(them);
    syncPartnership.SetPartner2(us);
  }
  else if (partnerID==2)
  {
    syncPartnership.SetPartner1(us);
    syncPartnership.SetPartner2(them);
  }
  else
    return false;

  PLT_SyncPolicy partnershipPolicy;
  partnershipPolicy.m_policyType = NPT_String(ds->get_field_value("SyncPartnership.syncType").get_asString().c_str());
  partnershipPolicy.m_priorityPartnerID = ds->get_field_value("SyncPartnership.priorityPartner").get_asUInt();
  partnershipPolicy.m_delProtection.SetValue((PLT_OptionalBool::Values)ds->get_field_value("SyncPartnership.bDelProtection").get_asInt());
  partnershipPolicy.m_autoObjAdd.SetValue((PLT_OptionalBool::Values)ds->get_field_value("SyncPartnership.bAutoObjAdd").get_asInt());
  syncPartnership.SetPolicy(partnershipPolicy);
  syncPartnership.GetActiveState().SetValue((PLT_OptionalBool::Values)ds->get_field_value("SyncPartnership.active").get_asInt());
  syncPartnership.SetUpdateID(ds->get_field_value("SyncPartnership.updateID").get_asInt());
  syncPartnership.SetID(ds->get_field_value("SyncPartnership.GUID").get_asString().c_str());

  if (includePairGroups)
  {
    if(!GetPairGroups(syncPartnership.GetID().GetChars(), syncPartnership.GetChilds()))
      return false;
  }
  return true;

}
bool CUPnPDatabase::FillPairGroup(dbiplus::Dataset* const ds, PLT_PairGroup& syncRelationship)
{
  return true;
}

bool CUPnPDatabase::FillSyncRelationship(dbiplus::Dataset* const ds, PLT_SyncRelationship& syncRelationship, bool includePairGroups /*= false*/)
{
  if (ds->eof())
    return false;

  PLT_PartnershipRef partnership(new PLT_Partnership());
  if(!FillPartnership(ds, *partnership, includePairGroups))
     return false;

  syncRelationship.SetID(ds->get_field_value("SyncRelationship.GUID").get_asString().c_str());
  syncRelationship.GetActiveState().fromString(ds->get_field_value("SyncRelationship.active").get_asString().c_str());
    //syncRelationship.m_strSystemUpdateID = ""; //TODO:!!
  syncRelationship.SetTitle(ds->get_field_value("SyncRelationship.title").get_asString().c_str());
  syncRelationship.SetUpdateID(ds->get_field_value("SyncRelationship.lastUpdateID").get_asInt());
  syncRelationship.AddChild(partnership);

  return true;
}

bool CUPnPDatabase::GetSyncRelationshipForDevice(const std::string& DeviceUUID, PLT_SyncRelationshipRef& syncRelationship, bool includePairGroups /* = false */)
{

  try
  {
    if (!Open()) return false;
    if (NULL == m_pDB.get()) return false;
    if (NULL == m_pDS.get()) return false;

    CLog::Log(LOGINFO, "Get Sync relationship for device '%s'.", DeviceUUID.c_str());
    std::string strSQL = PrepareSQL("SELECT SyncRelationship.lastUpdateID, SyncRelationship.active, SyncRelationship.title, SyncRelationship.GUID, "
                                           "SyncPartnership.syncType, SyncPartnership.priorityPartner, SyncPartnership.bDelProtection, SyncPartnership.bAutoObjAdd, SyncPartnership.GUID, SyncPartnership.active, SyncPartnership.updateID, "
                                           "SyncPartner.partnerID, SyncPartner.DeviceUDN, SyncPartner.ServiceID "
                                    "FROM SyncPartner "
                                    "INNER JOIN SyncPartnership ON SyncPartnership.id = SyncPartner.syncPartnership "
                                    "INNER JOIN SyncRelationship ON SyncRelationship.id = SyncPartnership.syncRelationship "
                                    "where SyncPartner.DeviceUDN='%s'", DeviceUUID.c_str());
    syncRelationship = PLT_SyncRelationshipRef(new PLT_SyncRelationship);
    if (!m_pDS->query(strSQL.c_str()) || !FillSyncRelationship(m_pDS.get(), *syncRelationship, includePairGroups))
    {
      CLog::Log(LOGERROR, "%s - Getting the sync relationship for device with id '%s' failed", __FUNCTION__, DeviceUUID.c_str());
      return false;
    }

    return true;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s - Getting the sync relationship for device with id '%s' failed", __FUNCTION__, DeviceUUID.c_str());
  }
  return false;
}

bool CUPnPDatabase::GetAllSyncData(PLT_SyncData* result, bool includePairGroups /* = false */)
{
  try
  {
    if (!Open()) return false;
    if (NULL == m_pDB.get()) return false;
    if (NULL == m_pDS.get()) return false;

    std::string strSQL = PrepareSQL("SELECT SyncRelationship.lastUpdateID, SyncRelationship.active, SyncRelationship.title, SyncRelationship.GUID, "
      "SyncPartnership.syncType, SyncPartnership.priorityPartner, SyncPartnership.bDelProtection, SyncPartnership.bAutoObjAdd, SyncPartnership.GUID, SyncPartnership.active, SyncPartnership.updateID, "
      "SyncPartner.partnerID, SyncPartner.DeviceUDN, SyncPartner.ServiceID "
      "FROM SyncPartner "
      "INNER JOIN SyncPartnership ON SyncPartnership.id = SyncPartner.syncPartnership "
      "INNER JOIN SyncRelationship ON SyncRelationship.id = SyncPartnership.syncRelationship ");
    if (!m_pDS->query(strSQL.c_str()))
    {
      CLog::Log(LOGERROR, "%s - Getting all sync relationships failed", __FUNCTION__);
      return false;
    }
    while (!m_pDS->eof())
    {
      PLT_SyncRelationshipRef syncRelationship(new PLT_SyncRelationship);
      if (!FillSyncRelationship(m_pDS.get(), *syncRelationship, includePairGroups))
        return false;
      result->m_syncData.Add(syncRelationship);
      m_pDS->next();
    }

    return true;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s - Getting all sync relationships failed", __FUNCTION__);
  }
  return false;
}

bool CUPnPDatabase::GetSyncRelationship(const std::string& syncID, PLT_SyncRelationshipRef& syncRelationship, bool includePairGroups /* = false*/)
{
  try
  {
    if (!Open()) return false;
    if (NULL == m_pDB.get()) return false;
    if (NULL == m_pDS.get()) return false;

    CLog::Log(LOGINFO, "Get Sync relationship with id '%s'.", syncID.c_str());
    std::string strSQL = PrepareSQL("select SyncRelationship.lastUpdateID, SyncRelationship.active, SyncRelationship.title, SyncRelationship.GUID, "
                                    "SyncPartnership.syncType, SyncPartnership.priorityPartner, SyncPartnership.bDelProtection, SyncPartnership.bAutoObjAdd, SyncPartnership.GUID, SyncPartnership.active, SyncPartnership.updateID, "
                                    "SyncPartner.partnerID, SyncPartner.DeviceUDN, SyncPartner.ServiceID "
                                    "from SyncPartner "
                                    "INNER JOIN SyncPartnership ON SyncPartnership.id = SyncPartner.syncPartnership "
                                    "INNER JOIN SyncRelationship ON SyncRelationship.id = SyncPartnership.syncRelationship "
                                    "where SyncRelationship.GUID='%s'", syncID.c_str(), syncID.c_str());
    syncRelationship = PLT_SyncRelationshipRef(new PLT_SyncRelationship);
    if (!m_pDS->query(strSQL.c_str()) || !FillSyncRelationship(m_pDS.get(), *syncRelationship, includePairGroups))
    {
      CLog::Log(LOGERROR, "%s - Getting the sync relationship with id '%s' failed", __FUNCTION__, syncID.c_str());
      return false;
    }

    return true;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s - Getting the sync relationship with the id '%s' failed", __FUNCTION__, syncID.c_str());
  }
  return false;
}

bool CUPnPDatabase::GetSyncPartnership(const std::string& syncID, PLT_PartnershipRef& syncPartnership, bool includePairGroups /*=false*/)
{
  try
  {
    if (!Open()) return false;
    if (NULL == m_pDB.get()) return false;
    if (NULL == m_pDS.get()) return false;

    CLog::Log(LOGINFO, "Get Sync partnership with id '%s'.", syncID.c_str());
    std::string strSQL = PrepareSQL("select SyncPartnership.syncType, SyncPartnership.priorityPartner, SyncPartnership.bDelProtection, SyncPartnership.bAutoObjAdd, SyncPartnership.GUID, SyncPartnership.active, SyncPartnership.updateID, "
                                           "SyncPartner.partnerID, SyncPartner.DeviceUDN, SyncPartner.ServiceID "
                                    "from SyncPartner "
                                    "INNER JOIN SyncPartnership ON SyncPartnership.id = SyncPartner.syncPartnership "
                                    "where SyncPartnership.GUID='%s'", syncID.c_str(), syncID.c_str());
    syncPartnership = PLT_PartnershipRef(new PLT_Partnership);
    if (!m_pDS->query(strSQL.c_str()) || !FillPartnership(m_pDS.get(), *syncPartnership, includePairGroups))
    {
      CLog::Log(LOGERROR, "%s - Getting the sync partnerhip id '%s' failed", __FUNCTION__, syncID.c_str());
      return false;
    }

    return true;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s - Getting the sync partnership with the id '%s' failed", __FUNCTION__, syncID.c_str());
  }
  return false;
}
bool CUPnPDatabase::GetPairGroup(const std::string& syncID, PLT_PairGroupRef& syncPairGroup)
{
  return false;
}
bool CUPnPDatabase::GetPairGroups(const std::string& parentSyncID, NPT_List<PLT_PairGroupRef>& pairGroups)
{
  return true;
}

bool CUPnPDatabase::AddImportFolder(const std::string& remoteObjectID, const PLT_SyncPair& SyncPair)
{
  try
  {
    if (!Open()) return false;
    if (NULL == m_pDB.get()) return false;
    if (NULL == m_pDS.get()) return false;

    ImportFolder importFolder;
    //check if import folder already exists... if so, we should check wether the sync policies are the same (if so we can go ahead and return true)
    if (GetImportFolder(SyncPair.m_remoteObjectID.GetChars(), remoteObjectID, SyncPair.m_pairGroupID.GetChars(), importFolder))
      return (importFolder.syncPolicy == SyncPair.m_syncPolicy);

    std::string sql = PrepareSQL("INSERT INTO ImportFolder (localFolder, remoteFolder, syncType, priorityPartner, delProtection, autoObjAdd) "
                                   "SELECT id, '%s', '%s', '%s', %i, %i, %i FROM SyncPairGroup where SyncPairGroup.GUID='%s'",
                                                    SyncPair.m_remoteObjectID.GetChars(),
                                                    remoteObjectID.c_str(),
                                                    SyncPair.m_syncPolicy.m_policyType.GetChars(),
                                                    SyncPair.m_syncPolicy.m_priorityPartnerID,
                                                    SyncPair.m_syncPolicy.m_delProtection.GetValue(),
                                                    SyncPair.m_syncPolicy.m_autoObjAdd.GetValue(),
                                                    SyncPair.m_pairGroupID.GetChars());
    m_pDS->exec(sql.c_str());

    return true;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s - Adding import folder '%s' failed", __FUNCTION__, SyncPair.m_remoteObjectID.GetChars());
  }
  return false;
}


bool CUPnPDatabase::RemoveImportFolder(const PLT_SyncPair& SyncPair)
{
  return false;
}
bool CUPnPDatabase::HasImportFolder(const std::string& objectID, const std::string& remoteObjectID, const std::string pairGroupGUID)
{
  return false;
}


bool CUPnPDatabase::GetImportFolder(const std::string& objectID, const std::string& remoteObjectID, const std::string pairGroupGUID, ImportFolder& importFolder)
{
  return false;
}

bool CUPnPDatabase::HasSyncRelationshipWith(const char* deviceUDN)
{
  try
  {
    if (!Open()) return false;
    if (NULL == m_pDB.get()) return false;
    if (NULL == m_pDS.get()) return false;

    std::string strSQL = PrepareSQL("select partnerID from SyncPartner where SyncPartner.DeviceUDN='%s'", deviceUDN);
    m_pDS->query(strSQL.c_str());

    return m_pDS->num_rows() > 0;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s Getting Sync Relationship for device (%s) failed", __FUNCTION__, deviceUDN);
  }
  return false;
}

bool CUPnPDatabase::HasSyncRelationshipWith(const char* deviceUDN, const char* serviceID)
{
  try
  {
    if (!Open()) return false;
    if (NULL == m_pDB.get()) return false;
    if (NULL == m_pDS.get()) return false;

    std::string strSQL = PrepareSQL("select partnerID from SyncPartner where SyncPartner.DeviceUDN='%s' AND SyncPartner.ServiceID='%s'",deviceUDN, serviceID);
    m_pDS->query(strSQL.c_str());

    return m_pDS->num_rows() > 0;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s (%s, %s) failed", __FUNCTION__, deviceUDN, serviceID);
  }
  return false;
}


bool CUPnPDatabase::HasChanges(const char* deviceUDN, const char* serviceID)
{
  return false; //TODO;
}

int CUPnPDatabase::UUIDToPrimaryKey(const std::string& table, const std::string& UUID)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;

  try {
    std::string strSQL = PrepareSQL("SELECT id FROM %s where GUID='%s'", table.c_str(), UUID.c_str());
    m_pDS->exec(strSQL);
    return m_pDS->get_field_value("id").get_asInt();

  }
  catch (...)
  {
    CLog::Log(LOGWARNING, "%s - Getting the primary key for %s with UUID failed: %s", __FUNCTION__, table.c_str(), UUID.c_str());
    return 0;
  }
}

void CUPnPDatabase::CreateTables()
{
  CLog::Log(LOGINFO, "Create SyncRelationship table.");
  m_pDS->exec("CREATE TABLE IF NOT EXISTS SyncRelationship (" //TODO: systemUpdateID... don't know where yet
                "id INTEGER PRIMARY KEY,"
                "lastSync DATETIME DEFAULT CURRENT_TIMESTAMP,"
                "lastUpdateID INTEGER,"
                "active INTEGER,"
                "title TEXT,"
                "GUID TEXT NOT NULL UNIQUE"   //Protocol says, that each SyncRelationship needs a GUID
              ")");
  CLog::Log(LOGINFO, "Create SyncPartnership table.");
  m_pDS->exec("CREATE TABLE IF NOT EXISTS SyncPartnership ("
                "id INTEGER PRIMARY KEY,"
                "syncRelationship INTEGER NOT NULL, " //'Foreign Key' to the sync relationship
                "syncType VARCHAR NOT NULL, "         //"replace", "merge", "blend", "tracking" or "xbmc"
                "priorityPartner INTEGER, "           //In case of a merge issue, whose changesets have a higher priority
                "bDelProtection INTEGER, "            //If the item gets deleted in our partner, should we also delete it in here?
                "bAutoObjAdd INTEGER, "               //If this sync policy belong to a 'virtual folder' e.g. album. If the user added a new song to this album on our partner device, should we automatically add this song to our DB?
                "GUID TEXT NOT NULL UNIQUE, "         //Protocol says, that each SyncPartnership needs a GUID
                "active INTEGER,"
                "updateID INTEGER"
              ")");
  CLog::Log(LOGINFO, "Create SyncPartner table.");
  m_pDS->exec("CREATE TABLE IF NOT EXISTS SyncPartner ("
                "id INTEGER PRIMARY KEY,"
                "syncPartnership INTEGER NOT NULL, "            //'Foreign Key' to the sync partnership
                "partnerID INTEGER NOT NULL, "
                "lastSync DATETIME DEFAULT CURRENT_TIMESTAMP, "
                "DeviceUDN TEXT, "
                "ServiceID TEXT, "
                "UNIQUE (syncPartnership, partnerID))");
  CLog::Log(LOGINFO, "Create SyncPairGroup table.");
  m_pDS->exec("CREATE TABLE IF NOT EXISTS SyncPairGroup ("
                "id INTEGER PRIMARY KEY, "
                "GUID TEXT NOT NULL UNIQUE,"          //Protocol says, that each SyncPair needs a GUID
                "SyncPartnership INTEGER NOT NULL,"
                "active INTEGER, "
                "updateID INTEGER, "
                "syncType VARCHAR, "         //"replace", "merge", "blend", "tracking" or "xkodi"
                "priorityPartner INTEGER, "  //In case of a merge issue, whose changesets have a higher priority
                "bDelProtection INTEGER, "   //If the item gets deleted in our partner, should we also delete it in here?
                "bAutoObjAdd INTEGER"        //If this sync policy belong to a 'virtual folder' e.g. album. If the user added a new song to this album on our partner device, should we automatically add this song to our DB?
              ")");
  m_pDS->exec("CREATE TABLE IF NOT EXISTS ImportFolder ("
                "id INTEGER PRIMARY KEY, "
                "localFolder TEXT, "
                "remoteFolder TEXT, "
                "syncType VARCHAR, "
                "priorityPartner INTEGER, "
                "delProtection INTEGER, "
                "autoObjAdd INTEGER, "
                "idSyncPairGroup INTEGER " //Foreign key
              ")");
}

void CUPnPDatabase::CreateAnalytics()
{
  CLog::Log(LOGINFO, "%s creating indizes", __FUNCTION__);
  //TODO:

  CLog::Log(LOGINFO, "%s creating triggers", __FUNCTION__);
  m_pDS->exec("CREATE TRIGGER syncRelationshipDelete AFTER delete ON SyncRelationship "
              "FOR EACH ROW BEGIN "
                 "delete from SyncPartnership where SyncPartnership.syncRelationship=old.id; "
              "END");
  m_pDS->exec("CREATE TRIGGER syncPartnershipDelete AFTER delete ON SyncPartnership "
              "FOR EACH ROW BEGIN "
                 "delete from SyncPairGroup where SyncPairGroup.syncPartnership=old.id; "
                 "delete from SyncPartner where SyncPartner.syncPartnership=old.id; "
                 "delete from syncRelationship where syncRelationship.id=old.syncRelationship AND NOT (SELECT COUNT(*) from SyncPartnership where SyncPartnership.syncRelationship=old.syncRelationship); "
              "END");
  m_pDS->exec("CREATE TRIGGER syncPairGroupDelete AFTER delete ON SyncPairGroup "
              "FOR EACH ROW BEGIN "
                "delete from ImportFolder where ImportFolder.idSyncPairGroup=old.id; "
                "delete from syncPartnership where syncPartnership.id=old.syncPartnership AND NOT (SELECT COUNT(*) from SyncPairGroup where SyncPairGroup.syncPartnership=old.syncPartnership); "
              "END");
}

void CUPnPDatabase::UpdateTables(int version)
{
}

}
