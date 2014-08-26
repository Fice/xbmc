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

#include "PltSyncPolicy.h"
#include "PltUtilities.h"

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
NPT_Result PLT_SyncPolicy::FromXml(NPT_XmlElementNode* root)
{
  NPT_CHECK(root->GetTag().Compare("policy", true));
  
  NPT_XmlElementNode* node;
  node = PLT_XmlHelper::GetChild(root, "syncType");
  if(!node)
  {
    m_policyType = "";
    return NPT_ERROR_INVALID_PARAMETERS;
  }
  m_policyType = *(node->GetText());
  if (m_policyType.IsEmpty())
    return NPT_ERROR_INVALID_PARAMETERS;
  
  //TODO: we could validate wheter m_policyType has an allowed value, but then we probably need a way so client
  // apps can add their own policies
  
  node = PLT_XmlHelper::GetChild(root, "priorityPartnerID");
  m_priorityPartnerID = 0; //this value is optional. 0 means: not set!
  if(node)
  {
    NPT_CHECK(node->GetText()->ToInteger(m_priorityPartnerID));
  }
  if((m_policyType.Compare("replace", true) || m_policyType.Compare("merge", true)) && !m_priorityPartnerID)
  { //When the policy is 'replace' or 'merge' than the priorityPartnerID is not optional!!!
    return NPT_ERROR_INVALID_PARAMETERS;
  }

  node = PLT_XmlHelper::GetChild(root, "delProtection");
  m_delProtection.Reset(); //this value is optional. so reset it first
  if(node)
  {
    m_delProtection.fromString(*(node->GetText()));
  }
  
  node = PLT_XmlHelper::GetChild(root, "autoObjAdd");
  m_autoObjAdd.Reset(); //this value is optional. so reset it first
  if(node)
  {
    m_autoObjAdd.fromString(*(node->GetText()));
  }

  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_SyncPolicy::ToXml(NPT_String& result) const
{
  result += "<policy>"
              "<syncType>"+m_policyType+"</syncType>";
  
  if(m_priorityPartnerID)
      result += "<priorityPartnerID>"+NPT_String::FromInteger(m_priorityPartnerID)+"</priorityPartnerID>";
  if (m_delProtection.IsValid())
      result += "<delProtection>"+m_delProtection.toString()+"</delProtection>";
  if (m_autoObjAdd.IsValid())
      result += "<autoObjAdd>"+m_autoObjAdd.toString()+"</autoObjAdd>";
  result += "</policy>";

  return NPT_SUCCESS;
}
