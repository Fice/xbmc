/*****************************************************************
|
|   Platinum - AV Media Server Device
|
| Copyright (c) 2004-2010, Plutinosoft, LLC.
| All rights reserved.
| http://www.plutinosoft.com
|
| This program is free software; you can redistribute it and/or
| modify it under the terms of the GNU General Public License
| as published by the Free Software Foundation; either version 2
| of the License, or (at your option) any later version.
|
| OEMs, ISVs, VARs and other distributors that combine and 
| distribute commercially licensed software with Platinum software
| and do not wish to distribute the source code for the commercially
| licensed software under version 2, or (at your option) any later
| version, of the GNU General Public License (the "GPL") must enter
| into a commercial license agreement with Plutinosoft, LLC.
| licensing@plutinosoft.com
|  
| This program is distributed in the hope that it will be useful,
| but WITHOUT ANY WARRANTY; without even the implied warranty of
| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
| GNU General Public License for more details.
|
| You should have received a copy of the GNU General Public License
| along with this program; see the file LICENSE.txt. If not, write to
| the Free Software Foundation, Inc., 
| 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
| http://www.gnu.org/licenses/gpl-2.0.html
|
****************************************************************/

/** @file
 UPnP AV Media Server.
 */

#ifndef _PLT_SYNC_DATA_H_
#define _PLT_SYNC_DATA_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"
#include "PltMediaItem.h"

class PLT_PairGroup
{
  NPT_String m_id;
  bool m_bActive;
  NPT_UInt32 m_updateID;
  PLT_SyncPolicy m_policy;
  
  void GenerateID(void);
};

class PLT_Partner
{
  NPT_UInt32 m_id;
  NPT_String m_strDeviceUDN;
  NPT_String m_strServiceID;
};

class PLT_Partnership
{
  NPT_String m_strID;
  bool m_bActive;
  NPT_UInt32 m_updateID;
  NPT_List<PLT_Partner> m_partners;
  PLT_SyncPolicy m_policy;
  NPT_List<PLT_PairGroup> m_pairGroups;
  
  
};

class PLT_SyncRelationship
{
  bool IsPartnershipActive(const char* ID) const;
  bool IsPairGroupActive(const char* ID) const;
  bool IsActive() const { return m_bActive; }
protected:
  NPT_String m_strID;
  bool m_bActive;
  NPT_String m_strSystemUpdateID;
  NPT_String m_strTitle;
  NPT_List<PLT_Partnership> m_partnerships;
};

class PLT_SyncData
{
  NPT_List<PLT_SyncRelationship> m_syncPartnerships;
};

class PLT_Pair
{
  enum STATUS
  {
    NEW,
    MODIFIED,
    DELETED
  };
  
  NPT_String     m_syncRelationshipID;
  NPT_String     m_partnershipID;
  NPT_String     m_pairGroupID;
  NPT_String     m_remoteObjectID;
  PLT_SyncPolicy m_syncPolicy;
  STATUS         m_status;
};

class PLT_SyncPairs
{
  NPT_List<PLT_Pair> m_pairs;
  NPT_UInt32 m_updateID;
};

class PLT_ResetObjectList
{
    //hmm... not sure if ResetObjectList should actually contain a LIST of objectIDs...
    //specs only say that ObjectID is optional, not that it can occur multiple times
  NPT_String m_objectID_id;
  NPT_String m_objectID_remoteObjID;
  NPT_UInt32 m_objectID_updateID;
};

class PLT_Change
{
  PLT_MediaObjectReference mediaObject;
    //TODO: ChangeLog specific data
};

class PLT_ChangeLog
{
  NPT_List<PLT_Change> m_changes;
};

#endif /* _PLT_SYNC_DATA_H_ */
