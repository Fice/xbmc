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
  m_policyType = "";
  if(node != NULL)
  {
    const NPT_String* text = node->GetText();
    if (text)
      m_policyType = *text;
  }
  //TODO: we could validate wheter m_policyType has an allowed value, but then we probably need a way so client
  // apps can add their own policies

  node = PLT_XmlHelper::GetChild(root, "priorityPartnerID");
  m_priorityPartnerID = 0; //this value is optional. 0 means: not set!
  if(node)
    NPT_CHECK(node->GetText()->ToInteger(m_priorityPartnerID));
  if((m_policyType.Compare("replace", true)==0 || m_policyType.Compare("merge", true)==0) && !m_priorityPartnerID)
  { //When the policy is 'replace' or 'merge' than the priorityPartnerID is not optional!!!
    return NPT_ERROR_INVALID_PARAMETERS;
  }

  node = PLT_XmlHelper::GetChild(root, "delProtection");
  m_delProtection.Reset(); //this value is optional. so reset it first
  if(node)
    m_delProtection.fromString(*(node->GetText()));

  node = PLT_XmlHelper::GetChild(root, "autoObjAdd");
  m_autoObjAdd.Reset(); //this value is optional. so reset it first
  if(node)
    m_autoObjAdd.fromString(*(node->GetText()));

  return NPT_SUCCESS;
}

NPT_Result PLT_SyncPolicy::ToXml(NPT_String& result) const
{
  result += "<policy>";

  if (!m_policyType.IsEmpty())
      result += "<syncType>"+m_policyType+"</syncType>";
  if(m_priorityPartnerID)
      result += "<priorityPartnerID>"+NPT_String::FromInteger(m_priorityPartnerID)+"</priorityPartnerID>";
  if (m_delProtection.IsValid())
      result += "<delProtection>"+m_delProtection.toString()+"</delProtection>";
  if (m_autoObjAdd.IsValid())
      result += "<autoObjAdd>"+m_autoObjAdd.toString()+"</autoObjAdd>";

  result += "</policy>";

  return NPT_SUCCESS;
}
