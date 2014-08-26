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

#include "Database.h"
#include "settings/AdvancedSettings.h"
#include "filesystem/SpecialProtocol.h"
#include "filesystem/File.h"
#include "profiles/ProfilesManager.h"
#include "utils/AutoPtrHandle.h"
#include "utils/log.h"
#include "utils/SortUtils.h"
#include "utils/URIUtils.h"
#include "utils/StringUtils.h"
#include "sqlitedataset.h"
#include "DatabaseManager.h"
#include "DbUrl.h"
#include <algorithm>
#include <functional>
#include <stdexcept>

#ifdef HAS_MYSQL
#include "mysqldataset.h"
#endif

using namespace AUTOPTR;
using namespace dbiplus;

#define MAX_COMPRESS_COUNT 20

void CDatabase::Filter::AppendField(const std::string &strField)
{
  if (strField.empty())
    return;

  if (fields.empty() || fields == "*")
    fields = strField;
  else
    fields += ", " + strField;
}

void CDatabase::Filter::AppendJoin(const std::string &strJoin)
{
  if (strJoin.empty())
    return;

  if (join.empty())
    join = strJoin;
  else
    join += " " + strJoin;
}

void CDatabase::Filter::AppendWhere(const std::string &strWhere, bool combineWithAnd /* = true */)
{
  if (strWhere.empty())
    return;

  if (where.empty())
    where = strWhere;
  else
  {
    where = "(" + where + ") ";
    where += combineWithAnd ? "AND" : "OR";
    where += " (" + strWhere + ")";
  }
}

void CDatabase::Filter::AppendOrder(const std::string &strOrder)
{
  if (strOrder.empty())
    return;

  if (order.empty())
    order = strOrder;
  else
    order += ", " + strOrder;
}

void CDatabase::Filter::AppendGroup(const std::string &strGroup)
{
  if (strGroup.empty())
    return;

  if (group.empty())
    group = strGroup;
  else
    group += ", " + strGroup;
}

CDatabase::CDatabase(void)
{
  m_openCount = 0;
  m_sqlite = true;
  m_bMultiWrite = false;
  m_multipleExecute = false;
}

CDatabase::~CDatabase(void)
{
  Close();
}

void CDatabase::Split(const std::string& strFileNameAndPath, std::string& strPath, std::string& strFileName)
{
  strFileName = "";
  strPath = "";
  int i = strFileNameAndPath.size() - 1;
  while (i > 0)
  {
    char ch = strFileNameAndPath[i];
    if (ch == ':' || ch == '/' || ch == '\\') break;
    else i--;
  }
  strPath = strFileNameAndPath.substr(0, i);
  strFileName = strFileNameAndPath.substr(i);
}

std::string CDatabase::PrepareSQL(std::string strStmt, ...) const
{
  std::string strResult = "";

  if (NULL != m_pDB.get())
  {
    va_list args;
    va_start(args, strStmt);
    strResult = m_pDB->vprepare(strStmt.c_str(), args);
    va_end(args);
  }

  return strResult;
}

std::string CDatabase::GetSingleValue(const std::string &query, std::auto_ptr<Dataset> &ds)
{
  std::string ret;
  try
  {
    if (!m_pDB.get() || !ds.get())
      return ret;

    if (ds->query(query.c_str()) && ds->num_rows() > 0)
      ret = ds->fv(0).get_asString();

    ds->close();
  }
  catch(...)
  {
    CLog::Log(LOGERROR, "%s - failed on query '%s'", __FUNCTION__, query.c_str());
  }
  return ret;
}

std::string CDatabase::GetSingleValue(const std::string &strTable, const std::string &strColumn, const std::string &strWhereClause /* = std::string() */, const std::string &strOrderBy /* = std::string() */)
{
  std::string query = PrepareSQL("SELECT %s FROM %s", strColumn.c_str(), strTable.c_str());
  if (!strWhereClause.empty())
    query += " WHERE " + strWhereClause;
  if (!strOrderBy.empty())
    query += " ORDER BY " + strOrderBy;
  query += " LIMIT 1";
  return GetSingleValue(query, m_pDS);
}

std::string CDatabase::GetSingleValue(const std::string &query)
{
  return GetSingleValue(query, m_pDS);
}

bool CDatabase::DeleteValues(const std::string &strTable, const Filter &filter /* = Filter() */)
{
  std::string strQuery;
  BuildSQL(PrepareSQL("DELETE FROM %s ", strTable.c_str()), filter, strQuery);
  return ExecuteQuery(strQuery);
}

bool CDatabase::BeginMultipleExecute()
{
  m_multipleExecute = true;
  return true;
}

bool CDatabase::CommitMultipleExecute()
{
  m_multipleExecute = false;
  BeginTransaction();
  for (std::vector<std::string>::const_iterator i = m_multipleQueries.begin(); i != m_multipleQueries.end(); ++i)
  {
    if (!ExecuteQuery(*i))
    {
      RollbackTransaction();
      return false;
    }
  }
  CommitTransaction();
  return true;
}

bool CDatabase::ExecuteQuery(const std::string &strQuery)
{
  if (m_multipleExecute)
  {
    m_multipleQueries.push_back(strQuery);
    return true;
  }

  bool bReturn = false;

  try
  {
    if (NULL == m_pDB.get()) return bReturn;
    if (NULL == m_pDS.get()) return bReturn;
    m_pDS->exec(strQuery.c_str());
    bReturn = true;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s - failed to execute query '%s'",
        __FUNCTION__, strQuery.c_str());
  }

  return bReturn;
}

bool CDatabase::ResultQuery(const std::string &strQuery)
{
  bool bReturn = false;

  try
  {
    if (NULL == m_pDB.get()) return bReturn;
    if (NULL == m_pDS.get()) return bReturn;

    std::string strPreparedQuery = PrepareSQL(strQuery.c_str());

    bReturn = m_pDS->query(strPreparedQuery.c_str());
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s - failed to execute query '%s'",
        __FUNCTION__, strQuery.c_str());
  }

  return bReturn;
}

bool CDatabase::QueueInsertQuery(const std::string &strQuery)
{
  if (strQuery.empty())
    return false;

  if (!m_bMultiWrite)
  {
    if (NULL == m_pDB.get()) return false;
    if (NULL == m_pDS2.get()) return false;

    m_bMultiWrite = true;
    m_pDS2->insert();
  }

  m_pDS2->add_insert_sql(strQuery);

  return true;
}

bool CDatabase::CommitInsertQueries()
{
  bool bReturn = true;

  if (m_bMultiWrite)
  {
    try
    {
      m_bMultiWrite = false;
      m_pDS2->post();
      m_pDS2->clear_insert_sql();
    }
    catch(...)
    {
      bReturn = false;
      CLog::Log(LOGERROR, "%s - failed to execute queries",
          __FUNCTION__);
    }
  }

  return bReturn;
}

bool CDatabase::Open()
{
  DatabaseSettings db_fallback;
  return Open(db_fallback);
}

bool CDatabase::Open(const DatabaseSettings &settings)
{
  if (IsOpen())
  {
    m_openCount++;
    return true;
  }

  // check our database manager to see if this database can be opened
  if (!CDatabaseManager::Get().CanOpen(GetBaseDBName()))
    return false;

  DatabaseSettings dbSettings = settings;
  InitSettings(dbSettings);

  std::string dbName = dbSettings.name;
  dbName += StringUtils::Format("%d", GetSchemaVersion());
  return Connect(dbName, dbSettings, false);
}

void CDatabase::InitSettings(DatabaseSettings &dbSettings)
{
  m_sqlite = true;

#ifdef HAS_MYSQL
  if ( dbSettings.type.Equals("mysql") )
  {
    // check we have all information before we cancel the fallback
    if ( ! (dbSettings.host.empty() ||
            dbSettings.user.empty() || dbSettings.pass.empty()) )
      m_sqlite = false;
    else
      CLog::Log(LOGINFO, "Essential mysql database information is missing. Require at least host, user and pass defined.");
  }
  else
#else
  if ( dbSettings.type.Equals("mysql") )
    CLog::Log(LOGERROR, "MySQL library requested but MySQL support is not compiled in. Falling back to sqlite3.");
#endif
  {
    dbSettings.type = "sqlite3";
    if (dbSettings.host.empty())
      dbSettings.host = CSpecialProtocol::TranslatePath(CProfilesManager::Get().GetDatabaseFolder());
  }

  // use separate, versioned database
  if (dbSettings.name.empty())
    dbSettings.name = GetBaseDBName();
}

bool CDatabase::Update(const DatabaseSettings &settings)
{
  DatabaseSettings dbSettings = settings;
  InitSettings(dbSettings);

  int version = GetSchemaVersion();
  std::string latestDb = dbSettings.name;
  latestDb += StringUtils::Format("%d", version);

  while (version >= GetMinSchemaVersion())
  {
    std::string dbName = dbSettings.name;
    if (version)
      dbName += StringUtils::Format("%d", version);

    if (Connect(dbName, dbSettings, false))
    {
      // Database exists, take a copy for our current version (if needed) and reopen that one
      if (version < GetSchemaVersion())
      {
        CLog::Log(LOGNOTICE, "Old database found - updating from version %i to %i", version, GetSchemaVersion());

        bool copy_fail = false;

        try
        {
          m_pDB->copy(latestDb.c_str());
        }
        catch(...)
        {
          CLog::Log(LOGERROR, "Unable to copy old database %s to new version %s", dbName.c_str(), latestDb.c_str());
          copy_fail = true;
        }

        Close();

        if ( copy_fail )
          return false;

        if (!Connect(latestDb, dbSettings, false))
        {
          CLog::Log(LOGERROR, "Unable to open freshly copied database %s", latestDb.c_str());
          return false;
        }
      }

      // yay - we have a copy of our db, now do our worst with it
      if (UpdateVersion(latestDb))
        return true;

      // update failed - loop around and see if we have another one available
      Close();
    }

    // drop back to the previous version and try that
    version--;
  }
  // try creating a new one
  if (Connect(latestDb, dbSettings, true))
    return true;

  // failed to update or open the database
  Close();
  CLog::Log(LOGERROR, "Unable to create new database");
  return false;
}

bool CDatabase::Connect(const std::string &dbName, const DatabaseSettings &dbSettings, bool create)
{
  // create the appropriate database structure
  if (dbSettings.type.Equals("sqlite3"))
  {
    m_pDB.reset( new SqliteDatabase(this) ) ;
  }
#ifdef HAS_MYSQL
  else if (dbSettings.type.Equals("mysql"))
  {
    m_pDB.reset( new MysqlDatabase(this) ) ;
  }
#endif
  else
  {
    CLog::Log(LOGERROR, "Unable to determine database type: %s", dbSettings.type.c_str());
    return false;
  }

  // host name is always required
  m_pDB->setHostName(dbSettings.host.c_str());

  if (!dbSettings.port.empty())
    m_pDB->setPort(dbSettings.port.c_str());

  if (!dbSettings.user.empty())
    m_pDB->setLogin(dbSettings.user.c_str());

  if (!dbSettings.pass.empty())
    m_pDB->setPasswd(dbSettings.pass.c_str());

  // database name is always required
  m_pDB->setDatabase(dbName.c_str());

  // set SSL configuration regardless if any are empty (all empty means no SSL).
  m_pDB->setSSLConfig(dbSettings.key.c_str(), dbSettings.cert.c_str(), dbSettings.ca.c_str(), dbSettings.capath.c_str(), dbSettings.ciphers.c_str());

  // create the datasets
  m_pDS.reset(m_pDB->CreateDataset());
  m_pDS2.reset(m_pDB->CreateDataset());

  if (m_pDB->connect(create) != DB_CONNECTION_OK)
    return false;

  try
  {
    // test if db already exists, if not we need to create the tables
    if (!m_pDB->exists() && create)
    {
      if (dbSettings.type.Equals("sqlite3"))
      {
        //  Modern file systems have a cluster/block size of 4k.
        //  To gain better performance when performing write
        //  operations to the database, set the page size of the
        //  database file to 4k.
        //  This needs to be done before any table is created.
        m_pDS->exec("PRAGMA page_size=4096\n");

        //  Also set the memory cache size to 16k
        m_pDS->exec("PRAGMA default_cache_size=4096\n");
      }
      CreateDatabase();
    }

    // sqlite3 post connection operations
    if (dbSettings.type.Equals("sqlite3"))
    {
      m_pDS->exec("PRAGMA cache_size=4096\n");
      m_pDS->exec("PRAGMA synchronous='NORMAL'\n");
      m_pDS->exec("PRAGMA count_changes='OFF'\n");
    }
  }
  catch (DbErrors &error)
  {
    CLog::Log(LOGERROR, "%s failed with '%s'", __FUNCTION__, error.getMsg());
    m_openCount = 1; // set to open so we can execute Close()
    Close();
    return false;
  }

  m_openCount = 1; // our database is open

  if(IsChangelogged()) //TODO: && (HasUPnP Sources || SomeUsesUsesUsAsUpnpSource)
  {
    if(!m_pDB->RegisterChangeCallback())
    {
        //TODO: check if we can implement such a callback for mysql... if not, perhabs implement a background job that pulls the changesets periodically
      CLog::Log(LOGINFO, "%s unable to register a change callback. We will not be able to automatically start a sync with UPnP Devices", __FUNCTION__);
    }
  }

  return true;
}

bool CDatabase::ApplyChangeset(const std::string& tablename,
                               const ChangelogData& changelog,
                               const std::string& changelogGUID,
                               const std::string& originGUID)
{
  ASSERT(IsChangelogged());

  try {
    BeginTransaction();
    m_pDB->DeactivateChangelogTriggers();

      //Apply data...
      //TODO:

    m_pDB->ActivateChangelogTriggers();
    CommitTransaction();
  }
  catch (...) {
    return false;
  }
  return true;
}

struct FieldFinder
{
  FieldFinder(const std::string &fieldName) : fieldName(fieldName) {}
  bool operator() (const field_prop& fp)
  {
    return fp.name==fieldName;
  }
private:
  std::string fieldName;
};

bool CDatabase::CreateMirrorTable(const std::string& tablename, const record_prop& tableDefinition)
{
  std::string mirroredTablename = tablename + "_changesets";
  std::string strSQL = PrepareSQL("SELECT * FROM %s LIMIT 1", mirroredTablename.c_str());
  bool doesntExist = false;

  try
  {
    m_pDS2->query(strSQL.c_str());
  }
  catch(DbErrors &e)
  {
      //Mirrored table definition doesn't exist yet, recreate it!
    CLog::Log(LOGINFO, "%s - Mirrored changelogtable for %s doesn't exist. Creating one", __FUNCTION__, tablename.c_str());
    doesntExist = true;
  }

  const record_prop& mirrored_tableDefinition = m_pDS2->get_result_set().record_header;
  if(mirrored_tableDefinition.empty())
    doesntExist = true;

  record_prop::const_iterator i = tableDefinition.begin(),
                              end = tableDefinition.end();

  std::vector<std::string> columnNames;
  columnNames.reserve(tableDefinition.size());

  if(doesntExist)
  {
    std::string strColumnValues;
    for (; i != end; ++i)
    {
      std::string name = i->name;
      strColumnValues += ", old" + name+" NONE, new" + name+" NONE";
      columnNames.push_back(name);
    }
    std::string query = "CREATE TABLE "+mirroredTablename+" "
                        "(id INTEGER PRIMARY KEY NOT NULL, chagesetId INTEGER UNIQUE NOT NULL "+strColumnValues+");";
    if (m_pDS2->exec(query) != SQLITE_OK)
    {
      CLog::Log(LOGERROR, "%s - creating mirrored changelog table failed: %s", __FUNCTION__, query.c_str());
      return false;
    }
  }
  else
  {
    record_prop::const_iterator mtdBegin = mirrored_tableDefinition.begin(),
                                mtdEnd   = mirrored_tableDefinition.end();
    for (; i != end; ++i)
    {
      if (std::find_if(mtdBegin, mtdEnd, FieldFinder(i->name))==mtdEnd)
      { //This column does not exist in the mirrored table!
        //Let's create it
        columnNames.push_back(i->name);

        std::string query = "ALTER TABLE "+mirroredTablename+" ADD COLUMN "+i->name+" NONE";
        CLog::Log(LOGERROR, "%s - mirrored table %s is missing the field: %s", __FUNCTION__, mirroredTablename.c_str(), i->name.c_str());
        if (m_pDS2->exec(query) != SQLITE_OK)
        {
          CLog::Log(LOGERROR, "%s - error creating the missing field %s", __FUNCTION__, query.c_str());
          return false;
        }
      }
    }
  }

  if(!columnNames.empty())
    GetDataForNewColumne(tablename, columnNames);

  return true;
}

bool CDatabase::CreateChangelogTrigger(const std::string& tablename,
                                       const std::string& eventname,
                                       const std::string& columns,
                                       const std::string& values)
{
  std::string strTriggerTablename = "tr_" + eventname + "_" + tablename;

  m_pDS->exec("DROP TRIGGER IF EXISTS " + strTriggerTablename);

  std::string toDo = "INSERT INTO changelog"
                     "       (type           , origin) "
                     "VALUES ('"+tablename+"', 'local'); "
                     "INSERT INTO "+tablename+"_changesets"
                     "       (chagesetId "+columns+") "
                     "VALUES (last_insert_rowid() "+values+"); ";

  std::string strTrigger = m_pDB->CreateChangelogTriggerDefinition(strTriggerTablename,
                                                                   tablename,
                                                                   eventname,
                                                                   toDo);

  CLog::Log(LOGINFO, "%s - creating changelog triggers for table %s. EVENT: %s\n%s", __FUNCTION__, tablename.c_str(), eventname.c_str(), strTrigger.c_str());
  try
  {
    if (m_pDS->exec(strTrigger) != SQLITE_OK)
    {
      CLog::Log(LOGERROR, "%s - creating changelog trigger failed: %s", __FUNCTION__, strTrigger.c_str());
      return false;
    }
  } catch(DbErrors& e)
  {
    CLog::Log(LOGERROR, "%s - DbError: %s", __FUNCTION__, e.getMsg());
    return false;
  }
  return true;
}

bool CDatabase::CheckChangelogTable(const std::string& tablename)
{
  std::string strSQL = PrepareSQL("SELECT * FROM %s LIMIT 1", tablename.c_str());
  m_pDS->query(strSQL.c_str());

  const record_prop& tableDefinition = m_pDS->get_result_set().record_header;
  if(tableDefinition.empty())
    return false;

  if(!CreateMirrorTable(tablename, tableDefinition))
    return false;

  std::string strOldColumnNames; // ', old{COLUMN_NAME1}, old{COLUMN_NAME2}'
  std::string strNewColumnNames; // ', new{COLUMN_NAME1}, new{COLUMN_NAME2}'
  std::string strOldColumnValues;// ', old.{COLUMN_NAME1}, old.{COLUMN_NAME2}'
  std::string strNewColumnValues;// ', new.{COLUMN_NAME1}, new.{COLUMN_NAME2}'

  record_prop::const_iterator i   = tableDefinition.begin(),
  end = tableDefinition.end();
  for (; i != end; ++i)
  {
    std::string name = i->name;
    std::string type = FTypeToTypeString(i->type);

    strOldColumnNames  += ",old"  + name;
    strNewColumnNames  += ",new"  + name;
    strOldColumnValues += ",old." + name;
    strNewColumnValues += ",new." + name;
  }

  return (CreateChangelogTrigger(tablename, "insert", strNewColumnNames,                   strNewColumnValues) &&
          CreateChangelogTrigger(tablename, "update", strNewColumnNames+strOldColumnNames, strNewColumnValues+strOldColumnValues) &&
          CreateChangelogTrigger(tablename, "delete", strOldColumnNames,                   strOldColumnValues));
}

bool CDatabase::SetupChangelogTrigger()
{
  BeginTransaction();

  std::list<std::string> changeloggedTables;
  if(!GetChangelogedTables(changeloggedTables))
  {
    RollbackTransaction();
    return false;
  }

  std::list<std::string>::iterator i,
                                   begin = changeloggedTables.begin(),
                                   end = changeloggedTables.end();
  for(i = begin; i != end; ++i)
  {
    if(!CheckChangelogTable(*i))
    {
      RollbackTransaction();
      return false;
    }
  }
  CommitTransaction();

  return true;
}

void CDatabase::SetupChangelogTables()
{
    //TODO: remove?!? i guess
}

int CDatabase::GetDBVersion()
{
  m_pDS->query("SELECT idVersion FROM version\n");
  if (m_pDS->num_rows() > 0)
    return m_pDS->fv("idVersion").get_asInt();
  return 0;
}

bool CDatabase::UpdateVersion(const std::string &dbName)
{
  int version = GetDBVersion();
  if (version < GetMinSchemaVersion())
  {
    CLog::Log(LOGERROR, "Can't update database %s from version %i - it's too old", dbName.c_str(), version);
    return false;
  }
  else if (version < GetSchemaVersion())
  {
    CLog::Log(LOGNOTICE, "Attempting to update the database %s from version %i to %i", dbName.c_str(), version, GetSchemaVersion());
    bool success = true;
    BeginTransaction();
    try
    {
      // drop old analytics, update table(s), recreate analytics, update version
      m_pDB->drop_analytics();
      UpdateTables(version);
      CreateAnalytics();
      UpdateVersionNumber();
    }
    catch (...)
    {
      CLog::Log(LOGERROR, "Exception updating database %s from version %i to %i", dbName.c_str(), version, GetSchemaVersion());
      success = false;
    }
    if (!success)
    {
      CLog::Log(LOGERROR, "Error updating database %s from version %i to %i", dbName.c_str(), version, GetSchemaVersion());
      RollbackTransaction();
      return false;
    }
    CommitTransaction();

    if(IsChangelogged() && !SetupChangelogTrigger())
      CLog::Log(LOGERROR, "Creation of changelog Triggers failed. Automatic changelog creation will not be available");

    CLog::Log(LOGINFO, "Update to version %i successful", GetSchemaVersion());
  }
  else if (version > GetSchemaVersion())
  {
    CLog::Log(LOGERROR, "Can't open the database %s as it is a NEWER version than what we were expecting?", dbName.c_str());
    return false;
  }
  else 
    CLog::Log(LOGNOTICE, "Running database version %s", dbName.c_str());
  return true;
}

bool CDatabase::IsOpen()
{
  return m_openCount > 0;
}

void CDatabase::Close()
{
  if (m_openCount == 0)
    return;

  if (m_openCount > 1)
  {
    m_openCount--;
    return;
  }

  m_openCount = 0;
  m_multipleExecute = false;

  if (NULL == m_pDB.get() ) return ;
  if (NULL != m_pDS.get()) m_pDS->close();
  m_pDB->disconnect();
  m_pDB.reset();
  m_pDS.reset();
  m_pDS2.reset();
}

bool CDatabase::Compress(bool bForce /* =true */)
{
  if (!m_sqlite)
    return true;

  try
  {
    if (NULL == m_pDB.get()) return false;
    if (NULL == m_pDS.get()) return false;
    if (!bForce)
    {
      m_pDS->query("select iCompressCount from version");
      if (!m_pDS->eof())
      {
        int iCount = m_pDS->fv(0).get_asInt();
        if (iCount > MAX_COMPRESS_COUNT)
          iCount = -1;
        m_pDS->close();
        std::string strSQL=PrepareSQL("update version set iCompressCount=%i\n",++iCount);
        m_pDS->exec(strSQL.c_str());
        if (iCount != 0)
          return true;
      }
    }

    if (!m_pDS->exec("vacuum\n"))
      return false;
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s - Compressing the database failed", __FUNCTION__);
    return false;
  }
  return true;
}

void CDatabase::Interupt()
{
  m_pDS->interrupt();
}

void CDatabase::BeginTransaction()
{
  try
  {
    if (NULL != m_pDB.get())
      m_pDB->start_transaction();
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "database:begintransaction failed");
  }
}

bool CDatabase::CommitTransaction()
{
  try
  {
    if (NULL != m_pDB.get())
      m_pDB->commit_transaction();
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "database:committransaction failed");
    return false;
  }
  return true;
}

void CDatabase::RollbackTransaction()
{
  try
  {
    if (NULL != m_pDB.get())
      m_pDB->rollback_transaction();
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "database:rollbacktransaction failed");
  }
}

bool CDatabase::InTransaction()
{
  if (NULL != m_pDB.get()) return false;
  return m_pDB->in_transaction();
}

bool CDatabase::CreateDatabase()
{
  BeginTransaction();
  try
  {
    CLog::Log(LOGINFO, "creating version table");
    m_pDS->exec("CREATE TABLE version (idVersion integer, iCompressCount integer)\n");
    std::string strSQL=PrepareSQL("INSERT INTO version (idVersion,iCompressCount) values(%i,0)\n", GetSchemaVersion());
    m_pDS->exec(strSQL.c_str());

    CreateTables();
    CreateAnalytics();
    SetupChangelogTables();
  }
  catch (...)
  {
    CLog::Log(LOGERROR, "%s unable to create database:%i", __FUNCTION__, (int)GetLastError());
    RollbackTransaction();
    return false;
  }
  CommitTransaction();
  return true;
}

void CDatabase::UpdateVersionNumber()
{
  std::string strSQL=PrepareSQL("UPDATE version SET idVersion=%i\n", GetSchemaVersion());
  m_pDS->exec(strSQL.c_str());
}

bool CDatabase::BuildSQL(const std::string &strQuery, const Filter &filter, std::string &strSQL)
{
  strSQL = strQuery;

  if (!filter.join.empty())
    strSQL += filter.join;
  if (!filter.where.empty())
    strSQL += " WHERE " + filter.where;
  if (!filter.group.empty())
    strSQL += " GROUP BY " + filter.group;
  if (!filter.order.empty())
    strSQL += " ORDER BY " + filter.order;
  if (!filter.limit.empty())
    strSQL += " LIMIT " + filter.limit;

  return true;
}

bool CDatabase::BuildSQL(const std::string &strBaseDir, const std::string &strQuery, Filter &filter, std::string &strSQL, CDbUrl &dbUrl)
{
  SortDescription sorting;
  return BuildSQL(strBaseDir, strQuery, filter, strSQL, dbUrl, sorting);
}

bool CDatabase::BuildSQL(const std::string &strBaseDir, const std::string &strQuery, Filter &filter, std::string &strSQL, CDbUrl &dbUrl, SortDescription &sorting /* = SortDescription() */)
{
  // parse the base path to get additional filters
  dbUrl.Reset();
  if (!dbUrl.FromString(strBaseDir) || !GetFilter(dbUrl, filter, sorting))
    return false;

  return BuildSQL(strQuery, filter, strSQL);
}
