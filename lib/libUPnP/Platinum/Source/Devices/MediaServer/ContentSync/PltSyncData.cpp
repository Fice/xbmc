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
#include "PltSyncData.h"
#include "PltUtilities.h"

/*
struct PLTErrorCode
{
  PLTErrorCode(NPT_UInt32 code, const char* title, const char* description)
	  : code(code), title(title), description(description)
	{}

	NPT_UInt32 code;
	const char* title;
	const char* description;
};

PLTErrorCode errorCodes[] = {
  { 1, "Success", "Synchronization of an object is succeeded" },
  { 2, "Partial Success", "Synchronization of an object is succeeded, but some DIDL-Lite properties are missing due to devie capabiliteis." },
  { 3, "Not Accepted", "the object is not accepted due to device capability." },
  { 100, "General Problem", "a problem is confirmed, but no specific reason can be identified." },
  { 101, "Disabled Sync Operation", "the synchronization operation is disabled by the user." },
  { 102, "Unknown Destination", "the destination for the new object is not specified" },
  { 200, "General Media Problem", "some trouble related to media is detected. Checking the media to resolbe it." },
  { 201, "Insufficient Disk Space", "storage of the sync device (i.e. HDD or Flash Memory etc...) does not have enough available space to complete the synhronization." },
  { 202, "Storage Low Space", "the storage of the sync device has low available space and the synchronization process may fail." },
  { 300, "General System Problem", "a problem related to the system is detected. It may affect all synchronization processes in the sync-enabled Content Directory service." },
  { 301, "Insufficient Memory", "the system does not have enough system meory to complete the synchronization processes." },
  { 302, "Insufficient Processing", "the system does not have the enough CPU power to execute the designated synchronization processes." },
  { 303, "Low Memory", "the system has low available memory and the designated synchronization may fail." },
  { 304, "Low Processing", "the system has low available CPU power and the designated synchronization process may fail." },
  { 400, "General Content Problem", "a problem related to the content is detected. It may be associated with the content that is being synchronized." },
  { 401, "No Sync Content", "the necessary content is missing from the sync device." },
  { 402, "Content Write Protect", "write access to the recording content is prohibited." },
  { 403, "Synchronization Loser", "there are other synchronizing processes with the same content at the same period, and the current synchronization process is superseeded by the conflicting synchronization process." },
  { 404, "Content Locked", "the originally sync content has been preempted by another synchronization process" },
  { 405, "Invalid XML", "xml document format for content metadata is not valid." },
  { 701, "No such sync data", "The request failed because the specified SyncID argument is invalid." },
  { 702, "Invalid XML", "The request failed because the xml for one of the arguments was invalid." },
  { 703, "Invalid action caller", "The request failed because the action caller is a part of the sync data." },
  { 704, "Partner timeout", "The request failed because some interaction with the partner timed out." },
  { 705, "Partner not online", "The request failed because partner device is not in the network." },
  { 706, "Update in-progress", "The request failed because another action request is still being processed." },
  { 707, "Stale data", "The request failed because the sync data is stale." },
  { 708, "Invalid object", "The request failed because the specified ObjectID argument is invalid." },
  { 709, "Invalid pair", "The request failed because the specified SyncPair argument is invalid." },
  { 710, "Inactive state", "The request failed because the specified SyncID argument is not active." },
  { 711, "Sync operation in-progress", "The request failed because the sync operation of the specified sync data is in-progress." },
  { 712, "Invalid Sync operation invocation", "The request failed because the relationship contains a non-CDS partner." },
  { 720, "Cannot process the request", "The request failed because the ContentSync service was unable to complete the necessary computations in the time allotted." }
};

*/
  //See Table B-1 in Appendix B.  of ContentSync:1 Service Template Version 1.01
const char* syncableObjects[]= {"object.item",
                                "object.item.imageItem",
                                "object.item.imageItem.photo",
                                "object.item.audioItem",
                                "object.item.audioItem.musicTrack",
                                "object.item.audioItem.audioBook",
                                "object.item.videoItem",
                                "object.item.videoItem.movie",
                                "object.item.videoItem.musicVideoClip",
                                "object.item.playlistItem",
                                "object.item.bookmarkItem",
                                "object.item.textItem",
                                "object.item.epgItem",
                                "object.item.epgItem.audioProgram",
                                "object.item.epgItem.videoProgram",
                                "object.container",
                                "object.container.person",
                                "object.container.person.musicArtist",
                                "object.container.playlistContainer",
                                "object.container.album",
                                "object.container.album.musicAlbum",
                                "object.container.album.photoAlbum",
                                "object.container.genre",
                                "object.container.genre.musicGenre",
                                "object.container.genre.movieGenre",
                                "object.container.epgContainer",
                                "object.container.bookmarkFolder"
};

bool IsSyncable(const NPT_String& object_class)
{
  return true; //TODO:
}

bool IsValidUDN(const NPT_String& udn) { return true; /*TODO: */}
bool IsValidID(const NPT_String& id) { return true; /*TODO: */}

template<class T>
struct ToXMLFunctor
{
  ToXMLFunctor(NPT_String* result) : resultStr(result), resultValue(NPT_SUCCESS) {}
  
  NPT_String* resultStr;
  mutable NPT_Result resultValue;
  
  void operator()(const T& obj) const
  {
    NPT_Result result;
    result = obj.ToXml(*resultStr);
    if(NPT_FAILED(result))
      resultValue = result;
  }
};

template<typename T,  template <typename> class C >
NPT_Result PrintArrayToXml(const C<T> array, NPT_String& xml)
{
  ToXMLFunctor<T> function(&xml);
  array.Apply(function);
  return function.resultValue;
}

template<typename T, template <typename> class C>
NPT_Result LoadArrayFromXml(const NPT_String& tag, NPT_XmlElementNode* elem, C<T>& array)
{
  NPT_Array<NPT_XmlElementNode*> nodes;
  NPT_CHECK(PLT_XmlHelper::GetChildren(elem, nodes, tag));
  if (nodes.GetItemCount()==0)
    return NPT_SUCCESS; 
  
  for (unsigned int i=0; i < nodes.GetItemCount(); ++i) //reverse through the list, so we cann Add() the items at front and still have the same order. 
  {
    T object;
    NPT_CHECK(object.FromXml(nodes[i]));
    array.Add(object);
  }
  return NPT_SUCCESS;
}

NPT_Result PLT_SyncStatus::FromXml(NPT_XmlElementNode* root)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}
NPT_Result PLT_SyncStatus::ToXml(NPT_String& result) const
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

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
  
  return NPT_ERROR_NOT_IMPLEMENTED;
}
NPT_Result PLT_PairGroup::ToXml(NPT_String& result) const
{
  result += "<pairGroup id=\""+m_id+ "\" ";
  if (m_bActive.IsValid())
    result += "active=\""+m_bActive.toString()+"\"";
  result += "></pairGroup>";
  return NPT_SUCCESS;
}

NPT_Result PLT_Partner::FromXml(NPT_XmlElementNode* root)
{
  NPT_String tag = root->GetTag();
  if(tag.IsEmpty())
    return NPT_ERROR_INVALID_PARAMETERS;
  NPT_CHECK(tag.Compare("partner", true));
  
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
      m_bActive.Reset(); //not specified, by setting it to undefined, we will use the value of our parent synchronization structure!
    else
      m_bActive.fromString(*str);
  
    /////////////////
    //Get subelements
    /////////////////
    NPT_XmlElementNode* node;
    node = PLT_XmlHelper::GetChild(root, "policy");
    if (node)
    {
      m_policy.FromXml(node);
    }

    NPT_Array<NPT_XmlElementNode*> nodes;
    NPT_CHECK(PLT_XmlHelper::GetChildren(root, nodes, "partner"));
    if (nodes.GetItemCount()!=2)
      return NPT_ERROR_INVALID_PARAMETERS; 
    m_partner1.FromXml(nodes[0]);
    m_partner2.FromXml(nodes[1]);
  
    NPT_CHECK(LoadArrayFromXml("pairGroups", root, m_pairGroups));
    
    return NPT_SUCCESS;
}
NPT_Result PLT_Partnership::ToXml(NPT_String& result) const
{
  result += "<partnership id=\""+m_id+"\" ";
  if (m_bActive.IsValid())
    result += "active=\""+m_bActive.toString()+"\"";
  result += ">";
  
    //TODO: m_updateID
  
  
  NPT_CHECK(m_partner1.ToXml(result));
  NPT_CHECK(m_partner2.ToXml(result));
  
  NPT_CHECK(m_policy.ToXml(result));
  NPT_CHECK(PrintArrayToXml(m_pairGroups, result));
  
  result += "</partnership>";
  
  
  return NPT_SUCCESS;
}

 //ID of the partnership... if NULL, check if the SyncRelationship is disabled
bool PLT_SyncRelationship::IsPartnershipActive(const char* ID) const
{
  if(!IsActive())
    //If syncRelationship is disabled, treat all children relationships as disabled
    return false;
    
  if(ID)
  { //find the partnership
    //TODO:
  }
  
  return false; //Partnership not found
}

bool PLT_SyncRelationship::IsPairGroupActive(const char* ID) const
{
  if(!IsActive())
    //If syncRelationship is disabled, treat all children relationships as disabled
    return false;
    
  if(ID)
  { //find the pair group
    //TODO:
  }
  
  
  return false; //PairGroup not found
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
  
  m_bActive.fromString(*str);
  
  str = root->GetAttribute("systemUpdateID");
  m_updateID = 0;
  if(str)
  {
    str->ToInteger(m_updateID);
    //TODO: the specs say this is not an optional attribute... but when creating a SyncRelationship for the first time,
    //      they didn't send it.... so... for now let's treat it as optional
  }
  
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
  NPT_CHECK(LoadArrayFromXml("partnership", root, m_partnerships));
  
  return NPT_SUCCESS;
}

NPT_Result PLT_SyncRelationship::ToXml(NPT_String& result) const
{
  result += "<syncRelationship id=\""+m_id+"\" ";
  if(m_bActive.IsValid())
    result += "active=\""+m_bActive.toString()+"\" ";
  if (m_updateID)
    result += "syncUpdateID=\""+NPT_String::FromInteger(m_updateID)+"\" ";
  
    //TODO: the following does belong to PLT_SyncData::ToXml()
  result += "xmlns=\"urn:schemas-upnp-org:cs\" "
            "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
            "xmlns:xsi=\"http://www.w4.org/2001/XMLSchema-instance\" "
            "xsi:schemaLocation=\"urn:schemas-upnp-org:cs http://www.upnp.org/schemas/cs/cs-v1-2007XXXX.xsd\">";

  result += "<title>"+m_strTitle+"</title>";

  
  
  NPT_CHECK(PrintArrayToXml(m_partnerships, result));

  result += "</syncRelationship>";

  return NPT_SUCCESS;
}


struct DeviceUDNFinder
{
  DeviceUDNFinder(const NPT_String& udn) : udn(udn) {}
  bool operator ()(const PLT_Partner& partner) const
  {
    return partner.m_strDeviceUDN == udn;
  }
  const NPT_String& udn;
};

NPT_Result PLT_SyncRelationship::GetPartners(const NPT_String& localDeviceID,
                                             NPT_List<PLT_Partner>& result) const
{
  
  NPT_List<PLT_Partnership>::Iterator partnership = m_partnerships.GetFirstItem();
  while (partnership) {
    if (partnership->m_partner1.m_strDeviceUDN == localDeviceID) 
    {
      if (!result.Find(DeviceUDNFinder(partnership->m_partner2.m_strDeviceUDN)))
        result.Add(partnership->m_partner2);
    }
    else if (partnership->m_partner2.m_strDeviceUDN == localDeviceID)
    {
      if (!result.Find(DeviceUDNFinder(partnership->m_partner1.m_strDeviceUDN)))
        result.Add(partnership->m_partner1);
    }
    else 
      return NPT_ERROR_INVALID_PARAMETERS;

    ++partnership;
  }
  
  return NPT_SUCCESS;
}

NPT_Result PLT_SyncData::FromXml(NPT_XmlElementNode* root)
{
  return m_syncRelationship.FromXml(root);
}

NPT_Result PLT_SyncData::ToXml(NPT_String& result) const
{
  NPT_CHECK(m_syncRelationship.ToXml(result));
  
    //TODO:
    //add
  /*
  result += "xmlns=\"urn:schemas-upnp-org:cs\" "
  "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
  "xsi:schemaLocation=\"urn:schemas-upnp-org:cs http://www.upnp.org/schemas/cs/cs-v1-2007XXXX.xsd\">";*/
  
  return NPT_SUCCESS;
}

NPT_Result PLT_SyncPair::FromXml(NPT_XmlElementNode* str)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_SyncPairs::FromXml(NPT_XmlElementNode* str)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ResetObjectList::FromXml(NPT_XmlElementNode* str)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

NPT_Result PLT_ChangeLog::FromXml(NPT_XmlElementNode*)
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}
NPT_Result PLT_ChangeLog::ToXml(NPT_String& result) const
{
  return NPT_ERROR_NOT_IMPLEMENTED;
}

PLT_SyncChangeObserver::PLT_SyncChangeObserver(PLT_StateVariable* stateVariable) 
	: m_stateVariable(stateVariable) 
{
  NPT_String value;
  ToXml(value);
	m_stateVariable->SetValue(value);
	m_stateVariable->SetRate( NPT_TimeInterval(2.) );
}

void PLT_SyncChangeObserver::OnSyncDataChanged(const NPT_String& GUID)
{
	//check if already present!
	if (m_syncDataUpdate.Find(NPT_StringFinder(GUID)))
		return; //We already need to sync that data anyway...

	m_syncDataUpdate.Add(GUID);

	//update our state variable
  NPT_String value;
  ToXml(value);
	m_stateVariable->SetValue(value);
}

void PLT_SyncChangeObserver::OnSyncDataSynced(const NPT_String& GUID)
{
	//check if we have that guid
  if (NPT_SUCCESS == m_syncDataUpdate.Remove(GUID))
  {
    NPT_String value;
    ToXml(value);
    m_stateVariable->SetValue(value); //update state variable only if we actually removed the id
  }
}

NPT_Result PLT_SyncChangeObserver::ToXml(NPT_String& result) const {
	result += "<SyncChange xmlns=\"urn:schemas-upnp-org:cs\" "
		                    "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
	                    	"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
		                    "xsi:schemaLocation=\"urn:schemas-upnp-org:cs http://www.upnp.org/schemas/cs/cs-v1-20070XXXX.xsd\">";

  for(PLT_StringList::Iterator i = m_syncDataUpdate.GetFirstItem(); i; ++i)
  {
    result += "<syncDataUpdate syncID=\""+*i+"\"/>";
  }
  
  for(PLT_StringList::Iterator i = m_syncObjUpdate.GetFirstItem(); i; ++i)
  {
    result += "<syncObjUpdate objectID=\""+*i+"\"/>";
  }

  result += "</SyncChange>";
  return NPT_SUCCESS;
}

PLT_SyncStatusObserver::PLT_SyncStatusObserver(PLT_StateVariable* stateVariable)
	: m_stateVariable(stateVariable)
{
  NPT_String value;
  ToXml(value);
	m_stateVariable->SetValue(value);
	m_stateVariable->SetRate(NPT_TimeInterval(2.));
}

NPT_Result PLT_SyncStatusObserver::ToXml(NPT_String& result) const
{
	result += "<SyncStatusUpdate xmlns=\"urn:schemas-upnp-org:cs\" "
					  		              "xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
						  				        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
							  			        "xsi:schemaLocation=\"urn:schemas-upnp-org:cs http://www.upnp.org/schemas/cs/cs-v1-20070XXXX.xsd\">";

	//TODO: print actual stuff!
  result += "</SyncStatusUpdate>";
  return NPT_SUCCESS;
}


