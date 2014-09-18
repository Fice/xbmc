#pragma once

#include "Neptune.h"
#include "PltSyncPolicy.h"


class PLT_SyncPair
{
public:
  enum STATUS
  {
    EMPTY = 0, //We shouldn't add the status to the xml... e.g. when creating a sync pair
    SYNCED,
    MODIFIED,
    NEW,
    DELETED,
    EXCLUDED //This syncPair has been removed but we didn't deal with it yet!
  };
  PLT_SyncPair() : m_status(EMPTY) {}

  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

  NPT_String     m_syncRelationshipID;
  NPT_String     m_partnershipID;
  NPT_String     m_pairGroupID;
  NPT_String     m_remoteObjectID;
  PLT_SyncPolicy m_syncPolicy;
  STATUS         m_status;
};

class PLT_SyncPairs
{
public:
  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;
private:
  NPT_List<PLT_SyncPair> m_pairs;
  NPT_UInt32 m_updateID;
};
