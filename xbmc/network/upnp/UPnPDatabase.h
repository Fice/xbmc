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

struct PLT_SyncRelationship;
struct PLT_Partnership;

namespace UPNP
{

class CUPnPDatabase : public CDatabase
{
public:
  CUPnPDatabase();
  virtual ~CUPnPDatabase();
  virtual bool Open();

  bool HasSyncRelationshipWith(const char* deviceUDN, const char* serviceID);
  bool AddSyncRelationship(const PLT_SyncRelationship*);
  bool AddPartnerships(const NPT_List<PLT_Partnership>* const partnerships, const int relationshipGUID);
  bool AddPartnership(const PLT_Partnership* partnership, const int relationshipGUID);
  bool AddPairGroups(const NPT_List<PLT_PairGroup>* const pairGroups, const int partnershipGUID);
  bool AddPairGroup(const PLT_PairGroup&, const int partnershipGUID);

protected:
  virtual void CreateTables();
  virtual void CreateAnalytics();
  virtual void UpdateTables(int version);
  virtual int GetSchemaVersion() const { return 1; };
  const char *GetBaseDBName() const { return "UPnP"; };
};

}