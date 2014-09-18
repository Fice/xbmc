#include "PltRelationshipStructures.h"
#include "PltUtilities.h"
#include "PltSyncUtils.h"

NPT_SET_LOCAL_LOGGER("platinum.media.server")

NPT_Result PLT_PairGroup::FromXml(NPT_XmlElementNode* root)
{
  NPT_CHECK(root->GetTag().Compare("pairGroup", true));

  ///////////////
  // Get attributes
  /////////////////
  const NPT_String *str = root->GetAttribute("id");
  if(!str)
  {
    m_id = "";
    return NPT_ERROR_INVALID_PARAMETERS;
  }
  m_id = *str;

  str = root->GetAttribute("updateID");
  if (!str || str->IsEmpty())
    m_updateID = 0;
  else
    NPT_CHECK(str->ToInteger(m_updateID));

  NPT_XmlElementNode* node;
  node = PLT_XmlHelper::GetChild(root, "policy");
  if (node)
    m_policy.FromXml(node);

  return NPT_SUCCESS;
}
NPT_Result PLT_PairGroup::ToXml(NPT_String& result) const
{
  result += "<pairGroup id=\""+m_id+ "\" ";
  if (m_active.IsValid())
    result += "active=\""+m_active.toString()+"\" ";
  result += "updateID=\"" + m_updateID;
  result += "\">";

  NPT_CHECK(m_policy.ToXml(result));

  result += "</pairGroup>";
  return NPT_SUCCESS;
}

NPT_Result PLT_Partner::FromXml(NPT_XmlElementNode* root)
{
  NPT_CHECK(root->GetTag().Compare("partner", true));

  ///////////////
  // Get attributes
  /////////////////
  const NPT_String *str = root->GetAttribute("id");
  if(!str)
  {
    m_id = 0;
    return NPT_ERROR_INVALID_PARAMETERS;
  }
  NPT_CHECK(str->ToInteger(m_id));

  /////////////////
  //Get subelements
  /////////////////
  NPT_XmlElementNode* node;
  node = PLT_XmlHelper::GetChild(root, "deviceUDN");
  if (node)
  {
    m_strDeviceUDN = *(node->GetText());
    if(m_strDeviceUDN.IsEmpty() || !IsValidUDN(m_strDeviceUDN))
      return NPT_ERROR_INVALID_PARAMETERS;
  }

  node = PLT_XmlHelper::GetChild(root, "serviceID");
  if (node)
  {
    m_strServiceID = *(node->GetText());
    if(m_strServiceID.IsEmpty() || !IsValidID(m_strServiceID))
      return NPT_ERROR_INVALID_PARAMETERS;
  }

  return NPT_SUCCESS;
}

NPT_Result PLT_Partner::ToXml(NPT_String& result) const
{
  result += "<partner id=\""+NPT_String::FromInteger(m_id)+"\">"
              "<deviceUDN>"+m_strDeviceUDN+"</deviceUDN>"
              "<serviceID>"+m_strServiceID+"</serviceID>"
            "</partner>";
  return NPT_SUCCESS;
}


NPT_Result PLT_Partnership::FromXml(NPT_XmlElementNode* root)
{
    NPT_CHECK(root->GetTag().Compare("partnership", true));

    ////////////////
    //Get attributes
    ////////////////
    const NPT_String *str = root->GetAttribute("id");
    if(!str)
    {
      m_id = "";
      return NPT_ERROR_INVALID_PARAMETERS;
    }
    m_id = *str;

    str = root->GetAttribute("active");
    if(!str)
      m_active.Reset(); //not specified, by setting it to undefined, we will use the value of our parent synchronization structure!
    else
      m_active.fromString(*str);

    /////////////////
    //Get subelements
    /////////////////
    NPT_XmlElementNode* node;
    node = PLT_XmlHelper::GetChild(root, "policy");
    if (node)
      m_policy.FromXml(node);

    NPT_Array<NPT_XmlElementNode*> nodes;
    NPT_CHECK(PLT_XmlHelper::GetChildren(root, nodes, "partner"));
    if (nodes.GetItemCount()!=2)
      return NPT_ERROR_INVALID_PARAMETERS;
    m_partner1.FromXml(nodes[0]); //TODO: the first partner could have partner ID 2... so we have to take that into account
    m_partner2.FromXml(nodes[1]);

    NPT_CHECK(LoadArrayFromXml("pairGroup", root, m_childs));

    return NPT_SUCCESS;
}
NPT_Result PLT_Partnership::ToXml(NPT_String& result) const
{
  result += "<partnership id=\""+m_id+"\" ";
  if (m_active.IsValid())
    result += "active=\""+m_active.toString()+"\"";
  result += ">";

    //TODO: m_updateID


  NPT_CHECK(m_partner1.ToXml(result));
  NPT_CHECK(m_partner2.ToXml(result));

  NPT_CHECK(m_policy.ToXml(result));
  NPT_CHECK(PrintArrayToXml(m_childs, result));

  result += "</partnership>";


  return NPT_SUCCESS;
}

NPT_Result PLT_Partnership::GetPartners(NPT_List<PLT_Partner>& result) const
{
  if (!result.Find(DeviceUDNFinder(m_partner1.m_strDeviceUDN)))
    NPT_CHECK(result.Add(m_partner1));
  if (!result.Find(DeviceUDNFinder(m_partner2.m_strDeviceUDN)))
    NPT_CHECK(result.Add(m_partner2));
  return NPT_SUCCESS;
}

NPT_Result PLT_SyncRelationship::FromXml(NPT_XmlElementNode* root)
{
  NPT_CHECK(root->GetTag().Compare("syncRelationship", true));

  ////////////////
  //Get attributes
  ////////////////
  const NPT_String *str = root->GetAttribute("id");
  if(!str)
  {
    m_id = "";
    return NPT_ERROR_INVALID_PARAMETERS;
  }
  m_id = *str;

  str = root->GetAttribute("active");
  if(!str)
    return NPT_ERROR_INVALID_PARAMETERS;
  else if ((str->Compare("0")==0) || (str->Compare("false", true)==0))
    m_active = false;
  else if ((str->Compare("1")==0) || (str->Compare("true", true)==0))
    m_active = true;
  else
    return NPT_ERROR_INVALID_PARAMETERS;

  str = root->GetAttribute("syncUpdateID");
  m_updateID = 0;
  if (str)
  {
    str->ToInteger(m_updateID);
    //TODO: the specs say this is not an optional attribute... but when creating a SyncRelationship for the first time,
    //      they didn't send it.... so... for now let's treat it as optional
  }
  str = root->GetAttribute("systemUpdateID");
  m_strSystemUpdateID = "";
  if (str)
    m_strSystemUpdateID = *str;
  NPT_XmlElementNode* node;
  node = PLT_XmlHelper::GetChild(root, "title");
  if(!node)
  {
    m_strTitle = "";
    return NPT_ERROR_INVALID_PARAMETERS;
  }
  m_strTitle = *(node->GetText());

  ///////////////
  // Sub elements
  ///////////////
  NPT_CHECK(LoadArrayFromXml("partnership", root, m_childs));

  return NPT_SUCCESS;
}

NPT_Result PLT_SyncRelationship::ToXml(NPT_String& result) const
{
  result += "<syncRelationship id=\""+m_id+"\" ";
  result += "active=\"" + m_active.toString() + "\" ";
  if (m_updateID)
    result += "syncUpdateID=\""+NPT_String::FromInteger(m_updateID)+"\" ";
  if (!m_strSystemUpdateID.IsEmpty())
    result += "systemUpdateID=\"" + m_strSystemUpdateID + "\"";

    //TODO: the following does belong to PLT_SyncData::ToXml()
  result += "xmlns=\"urn:schemas-upnp-org:cs\" "
            "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
            "xmlns:xsi=\"http://www.w4.org/2001/XMLSchema-instance\" "
            "xsi:schemaLocation=\"urn:schemas-upnp-org:cs http://www.upnp.org/schemas/cs/cs-v1-2007XXXX.xsd\">";

  result += "<title>"+m_strTitle+"</title>";

  NPT_CHECK(PrintArrayToXml(m_childs, result));

  result += "</syncRelationship>";

  return NPT_SUCCESS;
}

NPT_Result PLT_SyncRelationship::GetPartners(NPT_List<PLT_Partner>& result) const
{
  NPT_List<PLT_PartnershipRef>::Iterator partnership = m_childs.GetFirstItem();
  while (partnership)
  {
    NPT_CHECK((*partnership)->GetPartners(result));
    ++partnership;
  }

  return NPT_SUCCESS;
}

bool PLT_SyncData::Contains(const NPT_String& SyncID) const
{
  NPT_List<PLT_SyncStructureRef>::Iterator relationship = m_syncData.GetFirstItem();
  while (relationship)
  {
    if ((*relationship)->Contains(SyncID))
      return true;
    ++relationship;
  }
  return false;
}

NPT_Result PLT_SyncData::GetPartners(NPT_List<PLT_Partner>& result) const
{
  NPT_List<PLT_SyncStructureRef>::Iterator relationship = m_syncData.GetFirstItem();
  while (relationship)
  {
    (*relationship)->GetPartners(result);
    ++relationship;
  }

  return NPT_SUCCESS;
}

NPT_Result PLT_SyncData::FromXml(NPT_XmlElementNode* root, bool singular)
{
  if (root == NULL)
    return NPT_ERROR_INTERNAL;

  if(singular)
  {
    PLT_SyncRelationshipRef syncRelationship(new PLT_SyncRelationship());
    PLT_PartnershipRef syncPartnership(new PLT_Partnership());
    PLT_PairGroupRef syncPairGroup(new PLT_PairGroup());
    if (NPT_SUCCEEDED(syncRelationship->FromXml(root)))
      NPT_CHECK(m_syncData.Add(syncRelationship));
    else if (NPT_SUCCEEDED(syncPartnership->FromXml(root)))
      NPT_CHECK(m_syncData.Add(syncPartnership));
    else if (NPT_SUCCEEDED(syncPairGroup->FromXml(root)))
      NPT_CHECK(m_syncData.Add(syncPairGroup));
    else
      return NPT_ERROR_INVALID_PARAMETERS;
  }
  else
  {
    NPT_List<NPT_XmlNode*>& allchildren = root->GetChildren();
    NPT_List<NPT_XmlNode*>::Iterator iter = allchildren.GetFirstItem();
    while (iter)
    {
      NPT_XmlElementNode* element = (*iter)->AsElementNode();
      if (element != NULL)
      {
        if (NPT_FAILED(FromXml((*iter)->AsElementNode(), true)))
          NPT_LOG_WARNING_1("Sync Structure of type %s could not be loaded", element->GetTag());
      }
      ++iter;
    }
  }
  return NPT_SUCCESS;
}

NPT_Result PLT_SyncData::ToXml(NPT_String& result, bool singular) const
{
  if (singular)
  {
    if (m_syncData.GetItemCount() > 1)
      return NPT_ERROR_INTERNAL;
      //TODO:
      //add
    /*
     result += "xmlns=\"urn:schemas-upnp-org:cs\" "
     "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
     "xsi:schemaLocation=\"urn:schemas-upnp-org:cs http://www.upnp.org/schemas/cs/cs-v1-2007XXXX.xsd\">";*/

  }
  else
    result += "<ContentSync>";

  NPT_CHECK(PrintArrayToXml(m_syncData, result));

  if (!singular)
    result += "</ContentSync>";

  return NPT_SUCCESS;
}
