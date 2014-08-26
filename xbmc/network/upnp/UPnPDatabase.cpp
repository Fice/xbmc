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

bool CUPnPDatabase::Open()
{
  return CDatabase::Open();
}
  
bool CUPnPDatabase::HasSyncRelationshipWith(const char* deviceUDN, const char* serviceID)
{
  try
  {
    if (NULL == m_pDB.get()) return false;
    if (NULL == m_pDS.get()) return false;
    
    std::string strSQL = PrepareSQL("select syncPartnership from SyncPartner where SyncPartner.DeviceUDN=%s AND SyncPartner.ServiceID=%s",deviceUDN, serviceID);
    m_pDS->query(strSQL.c_str());
    if (m_pDS->num_rows() > 0)
      return true;

    return false;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s (%s, %s) failed", __FUNCTION__, deviceUDN, serviceID);
  }
  return false;
}
    

bool CUPnPDatabase::AddSyncRelationship(const PLT_SyncRelationship* relationship)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;
  if (NULL == relationship) return false;
  if (relationship->m_id.IsEmpty()) return false;
  
  //TODO: make sure such a partnership doesn't already exists.
  
  
  std::list<std::pair<std::string, std::string> > values;
  
  std::string columnsStr = "lastSync, lastUpdateID, active, title";
  std::string valuesStr  = "0, 0, ";
  valuesStr += (relationship->IsActive()) ? "1" : "0";
  valuesStr += ", ";
  valuesStr += "'"+relationship->m_strTitle+"'";

  columnsStr += ", GUID";
  valuesStr  += ", '"+relationship->m_id+"'";

 
  
  try {
    std::string sql = PrepareSQL("INSERT INTO SyncRelationship (%s) VALUES (%s)", columnsStr.c_str(), valuesStr.c_str());
    m_pDS->exec(sql.c_str());
    int guid = (int)m_pDS->lastinsertid();

    AddPartnerships(&relationship->m_partnerships ,guid);
  }
  catch (...) {
      //TODO: delete SyncRelationship
    CLog::Log(LOGERROR, "%s failed on relationship '%s'", __FUNCTION__, relationship->m_strTitle.GetChars());
    return false;
  }  
  
  return true;
}
  
bool CUPnPDatabase::AddPartnerships(const NPT_List<PLT_Partnership>* const partnerships, const int relationshipGUID)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;
  
  return true;
}
bool CUPnPDatabase::AddPartnership(const PLT_Partnership* partnership, const int relationshipGUID)
{
  if (NULL == m_pDB.get()) return false;
  if (NULL == m_pDS.get()) return false;
  
  return true;
}
  
void CUPnPDatabase::CreateTables()
{
  CLog::Log(LOGINFO, "Create SyncRelationship table.");
  m_pDS->exec("CREATE TABLE IF NOT EXISTS SyncRelationship ("
                "id PRIMARY KEY,"
                "lastSync DATETIME DEFAULT CURRENT_TIMESTAMP,"
                "lastUpdateID INTEGER,"
                "active INTEGER,"
                "title TEXT,"
                "GUID LONG UNIQUE NOT NULL" //Protocol says, that each SyncRelationship needs a GUID
              ")");
  CLog::Log(LOGINFO, "Create SyncPartnership table.");
  m_pDS->exec("CREATE TABLE IF NOT EXISTS SyncPartnership ("
                "syncRelationdship INTEGER NOT NULL, " //'Foreign Key' to the sync relationship
                "syncType VARCHAR, "          //"replace", "merge", "blend", "tracking" or "xbmc"
                "priorityPartner INTEGER, "  //In case of a merge issue, whose changesets have a higher priority 
                "bDelProtection INTEGER, "   //If the item gets deleted in our partner, should we also delete it in here?
                "bAutoObjAdd INTEGER, "      //If this sync policy belong to a 'virtual folder' e.g. album. If the user added a new song to this album on our partner device, should we automatically add this song to our DB?              
                "GUID LONG UNIQUE NOT NULL " //Protocol says, that each SyncPartnership needs a GUID
              ")");
  CLog::Log(LOGINFO, "Create SyncPartner table.");
  m_pDS->exec("CREATE TABLE IF NOT EXISTS SyncPartner ("
                "syncPartnership INTEGER NOT NULL, "            //'Foreign Key' to the sync partnership
                "partnerID INTEGER NOT NULL, "                  
                "lastSync DATETIME DEFAULT CURRENT_TIMESTAMP, "
                "DeviceUDN TEXT, "
                "ServiceID TEXT, "
                "UNIQUE (syncPartnership, partnerID)"
              ")");
  CLog::Log(LOGINFO, "Create SyncPair table.");
  m_pDS->exec("CREATE TABLE IF NOT EXISTS SyncPair ("
                "syncPartnership INTEGER NOT NULL, " //foreign key to the sync partnership
                "foreignKey INTEGER NOT NULL, "      //foreign key to the media object we're syncing... (e.g. a song or an album)
                "mediaType TEXT,"                    //the media type (also the table name) where our foreignKey is pointing to
                "lastSync DATETIME DEFAULT CURRENT_TIMESTAMP,"
                "GUID LONG UNIQUE NOT NULL"          //Protocol says, that each SyncPair needs a GUID
              ")");
}

void CUPnPDatabase::CreateAnalytics()
{
  CLog::Log(LOGINFO, "%s creating indices", __FUNCTION__);
  //TODO:

  CLog::Log(LOGINFO, "%s creating triggers", __FUNCTION__);
  m_pDS->exec("CREATE TRIGGER syncRelationshipDelete AFTER delete ON SyncRelationship "
              "FOR EACH ROW BEGIN "
                 "delete from SyncPartnership where SyncPartnership.syncRelationdship=old.id; "
              "END");
  m_pDS->exec("CREATE TRIGGER syncPartnershipDelete AFTER delete ON SyncPartnership "
              "FOR EACH ROW BEGIN "
                 "delete from SyncPair where SyncPair.syncPartnership=old.id; "
                 "delete from SyncPartner where SyncPartner.syncPartnership=old.id; "
              "END");
}

void CUPnPDatabase::UpdateTables(int version)
{
}

}