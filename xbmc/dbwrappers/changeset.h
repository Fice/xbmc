#pragma once
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
#include <vector>
#include <string>
#include <list>
  //#include "qry_dat.h"
#include <assert.h>
#include "boost/shared_ptr.hpp"

class Changeset
{
public:
  Changeset(const std::string &origin, const std::string &sink)
    : m_origin(origin), m_sink(sink)
  {
    assert(m_origin == "local" || m_sink == "local"); //one of them must be local
    assert(!(m_origin == "local" && m_sink == "local")); //Only one of them is allowed to be local
  }

  int GetTimestamp() const { return m_timestamp; }
private:
  std::string m_fieldName;
    //field_value m_value;
  std::string m_tableName;

  std::string m_origin; //either 'local' or the Upnp GUID that send us these changesets
  std::string m_sink; //either 'local' or the UpnpGUID that we will send these changesets to.

  int m_timestamp;
};

class Changesets
{
public:
  Changesets()
  {}
  Changesets(/*From didl*/ bool b)
  {
  }
  
  static boost::shared_ptr<Changesets> GetChangesets(const std::string &upnpDevice)
  {
    int lastUpdate = 0; //TODO: SQL for that devices last update timestamp
    int lastUpdateID = 0; //TODO: sql for that devices last update id
    
    std::vector<std::string> changes; //TODO: sql for all changes later than lastUpdate
    
    boost::shared_ptr<Changesets> result(new Changesets());
    
    return result;
  }
  
  bool Empty() { return m_changesets.empty(); }
  size_t Size() {
    return m_changesets.size();
  }
  
private:
  std::vector<Changeset> m_changesets;
  int m_firstUpdateID;
};
