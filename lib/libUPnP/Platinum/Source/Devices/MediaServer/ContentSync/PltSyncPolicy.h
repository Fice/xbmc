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

#ifndef _PLT_SYNC_POLICY_H_
#define _PLT_SYNC_POLICY_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"
#include "PltMediaItem.h"

struct PLT_OptionalBool
{
  
  PLT_OptionalBool() : m_value(UNDEFINED_) {}
  PLT_OptionalBool(bool value) : m_value(value ? TRUE_ : FALSE_) {}
  
  bool IsValid() const { return m_value!=UNDEFINED_; }
  bool IsTrue() const { return m_value==TRUE_; }
  bool IsFalse() const { return m_value==FALSE_; }
  void Reset() { m_value = UNDEFINED_; }
  
  void SetValue(bool value) { m_value = value ? TRUE_ : FALSE_; }
  NPT_Result fromString(const NPT_String& str)
  {
    if (str=="0" || str.Compare("false", true))
      m_value = FALSE_;
    else if (str=="1" || str.Compare("true", true))
      m_value = TRUE_;
    else {
      m_value = UNDEFINED_;
      return NPT_ERROR_INVALID_PARAMETERS;
    }
    return NPT_SUCCESS;
  }
  NPT_String toString() const {
    switch (m_value) {
      case TRUE_:
        return "1";
      case FALSE_:
        return "0";
      case UNDEFINED_:
      default:
        return "";
    }
  }
  
protected:
  enum Values {
    UNDEFINED_ = -1,
    FALSE_ = 0,
    TRUE_ = 1
  };
  
  Values m_value;
};

class PLT_SyncPolicy
{
public:
  PLT_SyncPolicy() : m_priorityPartnerID(0) {}
  
  const NPT_String& GetPolicyType() const { return m_policyType; }
  bool IsPriorityParnter(const NPT_UInt32 id) const { return m_priorityPartnerID == id;  }

  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

  NPT_String m_policyType;
  NPT_UInt32 m_priorityPartnerID;
  PLT_OptionalBool m_delProtection;
  PLT_OptionalBool m_autoObjAdd;
};

#endif //_PLT_SYNC_POLICY_H_