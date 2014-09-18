#include "PltSyncPair.h"
#include "PltUtilities.h"
#include "PltSyncUtils.h"

NPT_SET_LOCAL_LOGGER("platinum.media.server")

NPT_Result PLT_SyncPair::FromXml(NPT_XmlElementNode* root)
{
  NPT_CHECK(root->GetTag().Compare("pair", true));
  m_syncRelationshipID = *root->GetAttribute("syncRelationship"); //TODO: do we need valiadtion for these
  m_partnershipID = *root->GetAttribute("partnershipID"); //are they allowed to be empty?!?
  m_pairGroupID = *root->GetAttribute("pairGroupID");

  NPT_XmlElementNode* node;
  node = PLT_XmlHelper::GetChild(root, "policy");
  if (node)
  {
    m_syncPolicy.FromXml(node);
  }
  else
  {
    //TODO: should we fail now?!? or is this allowed
  }

  const NPT_String* status = root->GetAttribute("pairGroupID");
  if (status)
  {
    if (status->Compare("NEW", true))
      m_status = NEW;
    else if (status->Compare("MODIFIED", true))
      m_status = MODIFIED;
    else if (status->Compare("DELETED", true))
      m_status = DELETED;
    else if (status->Compare("SYNCED", true))
      m_status = SYNCED;
    else if (status->Compare("Excluded", true))
      m_status = EXCLUDED;
    else if (status->IsEmpty())
      m_status = EMPTY;
    else
    { //TODO: what should we do with unknown values? fail probably
      return NPT_ERROR_INTERNAL;
    }
  }
  else
  {
    m_status = EMPTY;
  }

  return NPT_SUCCESS;
}

NPT_Result PLT_SyncPair::ToXml(NPT_String& result) const
{
  result += "<pair syncRelationship=\"" + m_syncRelationshipID + "\" "
                  "partnershipID=\"" + m_partnershipID + "\" "
                  "pairGroupID=\"" + m_pairGroupID + "\" >";
  result += "<remoteObjID>" + m_remoteObjectID + "</remoteObjID>";
  NPT_CHECK(m_syncPolicy.ToXml(result));

  if (m_status != EMPTY)
  {
    NPT_String status;
    switch (m_status)
    {
    case NEW:
      status = "NEW";
      break;
    case MODIFIED:
      status = "MODIFIED";
      break;
    case DELETED:
      status = "DELETED";
      break;
    case SYNCED:
      status = "SYNCED";
      break;
    case EXCLUDED:
      status = "EXCLUDED";
      break;
    default:
      return NPT_ERROR_INTERNAL;
    }
    result += "<status>" + status + "</status>";
  }
  result += "</pair>";

  return NPT_SUCCESS;
}

NPT_Result PLT_SyncPairs::FromXml(NPT_XmlElementNode* root)
{
  NPT_CHECK(root->GetTag().Compare("syncInfo", true));

  const NPT_String *str = root->GetAttribute("id");
  NPT_CHECK(str->ToInteger(m_updateID));
  return LoadArrayFromXml("pair", root, m_pairs);
}

NPT_Result PLT_SyncPairs::ToXml(NPT_String& result) const
{
  result += "<syncInfo updateID=" + NPT_String::FromInteger(m_updateID) + ">";
  NPT_CHECK(PrintArrayToXml(m_pairs, result));
  result += "</syncInfo>";
  return NPT_SUCCESS;
}