#pragma once

#include "Neptune.h"
#include "PltStateVariable.h"
#include "PltMediaItem.h"
#include "PltSyncPolicy.h"
#include "PltUtilities.h"

typedef NPT_List<NPT_String> PLT_StringList;

enum PLT_SyncStructureType
{
  UNDEFINED,
  PAIR_GROUP,
  PARTNERSHIP,
  SYNC_RELATIONSHIP
};

struct PLT_Partner
{
public:
  PLT_Partner() : m_id(0) {}

  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

  NPT_UInt32 m_id;
  NPT_String m_strDeviceUDN;
  NPT_String m_strServiceID;
protected:
};
typedef NPT_List<PLT_Partner> PLT_PartnerList;

struct PLT_UUIDGenerator
{
  template<typename T>
  void operator()(T& obj) const
  {
    obj.GenerateUUIDs();
  }

  template<typename T>
  void operator()(NPT_Reference<T> obj) const
  {
    obj->GenerateUUIDs();
  }
};

class PLT_PairGroup;
class PLT_Partnership;
class PLT_SyncRelationship;

class PLT_SyncStructureConstVisitor
{
public:
  virtual NPT_Result Visit(const PLT_PairGroup* const) = 0;
  virtual NPT_Result Visit(const PLT_Partnership* const) = 0;
  virtual NPT_Result Visit(const PLT_SyncRelationship* const) = 0;
};

class PLT_SyncStructure
{
public:
  PLT_SyncStructure() : m_updateID(0) {}
  virtual ~PLT_SyncStructure() {}
  virtual NPT_Result FromXml(NPT_XmlElementNode*) = 0;
  virtual NPT_Result ToXml(NPT_String& result) const = 0;
  virtual PLT_SyncStructureType GetType() const = 0;
  virtual NPT_Result AddChild(NPT_Reference<PLT_SyncStructure> child) = 0;
  virtual NPT_Result GetChilds(NPT_List<NPT_Reference<PLT_SyncStructure> >& childs) = 0;
  virtual NPT_Result GetPartners(NPT_List<PLT_Partner>& partners) const = 0;
  virtual bool Contains(const NPT_String &SyncID) const = 0;
  /*virtual NPT_Result visit(PLT_SyncStructureVisitor* visitor) = 0;*/
  virtual NPT_Result Visit(PLT_SyncStructureConstVisitor* visitor, bool recursive /* = true */ ) const = 0;
  const NPT_String& GetID() const { return m_id; }
  virtual NPT_Result AggreagateSyncPolicy(PLT_SyncPolicy& result) const = 0;
  NPT_Result SetID(const NPT_String& id)
  {
    if (!m_id.IsEmpty())
      return NPT_ERROR_INTERNAL;
    m_id = id;
    return NPT_SUCCESS;
  }

  virtual NPT_Result GenerateUUIDs()
  {
    if (!m_id.IsEmpty())
      return NPT_ERROR_INTERNAL;                   //TODO: is the ctrl point allowed to send us new sync structures WITH UUIDS?!?
    PLT_UPnPMessageHelper::GenerateUUID(16, m_id); //TODO:  find out what UPnP spec says about uuids
    return NPT_SUCCESS;
  }
  const PLT_SyncPolicy& GetPolicy() const { return m_policy; }
  virtual NPT_Result SetPolicy(const PLT_SyncPolicy& policy) { m_policy = policy; return NPT_SUCCESS;  }
  NPT_UInt32 GetUpdateID() const { return m_updateID; }
  void SetUpdateID(NPT_UInt32 updateID) { m_updateID = updateID; }
  const PLT_OptionalBool& GetActiveState() const { return m_active;  }
  PLT_OptionalBool& GetActiveState() { return m_active; }
  void IncreaseUpdateID() { ++m_updateID; }
protected:
  NPT_String m_id;
  PLT_SyncPolicy m_policy;
  PLT_OptionalBool m_active;
  NPT_UInt32 m_updateID;
};
typedef NPT_Reference<PLT_SyncStructure> PLT_SyncStructureRef;

template <class T>
class PLT_ParentSyncStructure : public PLT_SyncStructure
{
public:
  typedef T ChildType;
  typedef NPT_Reference<T> ChildReference;
  typedef NPT_List<ChildReference> ChildList;

  virtual ~PLT_ParentSyncStructure() {}
  virtual bool Contains(const NPT_String &SyncID) const
  {
    typename ChildList::Iterator iter = m_childs.GetFirstItem();
    while (iter)
    {
      if ((*iter)->GetID() == SyncID || (*iter)->Contains(SyncID))
        return true;
      ++iter;
    }
    return false;
  }

  virtual NPT_Result AddChild(PLT_SyncStructureRef child)
  {
    if (child->GetType() == GetChildType())
    {
      m_childs.Add((ChildReference)child);
    }
    return NPT_ERROR_INTERNAL;
  }
  virtual NPT_Result GetChilds(NPT_List<PLT_SyncStructureRef>& childs)
  {
    typename ChildList::Iterator iter = m_childs.GetFirstItem();
    while (iter)
    {
      NPT_CHECK(childs.Add((PLT_SyncStructureRef)*iter));
      ++iter;
    }
    return NPT_SUCCESS;
  }
  const NPT_List<ChildReference>& GetChilds() const { return m_childs; }
  NPT_List<ChildReference>& GetChilds() { return m_childs; }
  virtual PLT_SyncStructureType GetChildType() const = 0;
  NPT_Result VisitChilds(PLT_SyncStructureConstVisitor* visitor, bool recursive) const
  {
    typename ChildList::Iterator iter = m_childs.GetFirstItem();
    while (iter)
    {
      NPT_CHECK((*iter)->Visit(visitor, recursive));
      ++iter;
    }
    return NPT_SUCCESS;
  }
  NPT_Result GenerateUUIDs()
  {
    NPT_CHECK(PLT_SyncStructure::GenerateUUIDs());
    return m_childs.Apply(PLT_UUIDGenerator());
  }
  NPT_Result AggreagateSyncPolicy(PLT_SyncPolicy& result) const
  {
    if (m_childs.GetItemCount()==1)
    {
      PLT_SyncPolicy child;
      NPT_CHECK((*m_childs.GetFirstItem())->AggreagateSyncPolicy(child));
      NPT_CHECK(PLT_SyncPolicy::Merge(this->m_policy, child, result));
    }
    else if (m_childs.GetItemCount()==0){
      result == m_policy;
    }
    else
      return NPT_ERROR_INTERNAL;

    return NPT_SUCCESS;
  }protected:
  ChildList m_childs;
};

class PLT_PairGroup : public PLT_SyncStructure
{
public:
  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

  virtual PLT_SyncStructureType GetType() const { return PAIR_GROUP; }
  virtual NPT_Result AddChild(NPT_Reference<PLT_SyncStructure> child) { return NPT_ERROR_INTERNAL; } /* we don't have children */
  virtual NPT_Result GetChilds(NPT_List<NPT_Reference<PLT_SyncStructure> >& childs) { return NPT_ERROR_INTERNAL; }
  virtual NPT_Result GetPartners(NPT_List<PLT_Partner>& partners) const { return NPT_ERROR_NOT_IMPLEMENTED; } /* TODO: we need to return the partner of our parent structure*/
  virtual bool Contains(const NPT_String &SyncID) const { return false; }
  virtual NPT_Result Visit(PLT_SyncStructureConstVisitor* visitor, bool recursive) const { return visitor->Visit(this); }
  NPT_Result AggreagateSyncPolicy(PLT_SyncPolicy& result) const
  {
    result = m_policy;
    return NPT_SUCCESS;
  }
};
typedef NPT_Reference<PLT_PairGroup> PLT_PairGroupRef;

class PLT_Partnership : public PLT_ParentSyncStructure<PLT_PairGroup>
{
public:
  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

  PLT_SyncStructureType GetChildType() const { return PAIR_GROUP; }
  virtual PLT_SyncStructureType GetType() const { return PARTNERSHIP; }
  virtual NPT_Result GetPartners(NPT_List<PLT_Partner>& partners) const;
  const NPT_List<NPT_Reference<PLT_PairGroup> >& GetChildren() const { return m_childs; }
  virtual NPT_Result Visit(PLT_SyncStructureConstVisitor* visitor, bool recursive) const
  {
    if (recursive)
      VisitChilds(visitor, recursive);
    return visitor->Visit(this);
  }
  const PLT_Partner& GetPartner1() const { return m_partner1; }
  const PLT_Partner& GetPartner2() const { return m_partner2; }
  void SetPartner1(const PLT_Partner& partner) {
    m_partner1 = partner;
    m_partner1.m_id = 1;
  }
  void SetPartner2(const PLT_Partner& partner) {
    m_partner2 = partner;
    m_partner2.m_id = 2;
  }

  NPT_Result GenerateUUIDs()
  {
    NPT_CHECK(PLT_ParentSyncStructure<PLT_PairGroup>::GenerateUUIDs());

    m_partner1.m_id = 1;
    m_partner2.m_id = 2;

    return NPT_SUCCESS;
  }
protected:
  PLT_Partner m_partner1;
  PLT_Partner m_partner2;
};
typedef NPT_Reference<PLT_Partnership> PLT_PartnershipRef;

class PLT_SyncRelationship : public PLT_ParentSyncStructure<PLT_Partnership>
{
public:
  PLT_SyncRelationship() { m_active.SetValue(true); }

  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;
  virtual NPT_Result Visit(PLT_SyncStructureConstVisitor* visitor, bool recursive) const
  {
    if (recursive)
      VisitChilds(visitor, recursive);
    return visitor->Visit(this);
  }
  virtual PLT_SyncStructureType GetType() const { return SYNC_RELATIONSHIP; }
  virtual PLT_SyncStructureType GetChildType() const { return PARTNERSHIP; }
  virtual NPT_Result GetPartners(NPT_List<PLT_Partner>& partners) const;
  void SetTitle(const NPT_String& title) { m_strTitle = title; }
  const NPT_String& GetTitle() const { return m_strTitle; }
  virtual NPT_Result SetPolicy(const PLT_SyncPolicy& policy)
  {
    if (m_policy.IsComplete())
      return NPT_ERROR_INTERNAL;
    m_policy = policy;
    return NPT_SUCCESS;
  }
protected:
  NPT_String m_strSystemUpdateID;
  NPT_String m_strTitle;
};
typedef NPT_Reference<PLT_SyncRelationship> PLT_SyncRelationshipRef;

struct PLT_SyncData
{
  NPT_List<PLT_SyncStructureRef> m_syncData;
  NPT_String m_parentID;

  NPT_Result GenerateUUIDs() { return m_syncData.Apply(PLT_UUIDGenerator()); }
  bool Contains(const NPT_String &SyncID) const;

  NPT_Result Visit(PLT_SyncStructureConstVisitor* visitor, bool recursive) const
  {
    NPT_List<PLT_SyncStructureRef>::Iterator iter = m_syncData.GetFirstItem();
    while (iter)
    {
      NPT_CHECK((*iter)->Visit(visitor, recursive));
      ++iter;
    }
    return NPT_SUCCESS;
  }

  NPT_Result GetPartners(NPT_List<PLT_Partner>& result) const;
  NPT_Result FromXml(NPT_XmlElementNode*, bool singular = false);
  NPT_Result ToXml(NPT_String& result, bool singular = false) const;
};

class PLT_ActiveChecker : public PLT_SyncStructureConstVisitor
{
public:
  template<class T>
  NPT_Result Check(const T* const obj)
  {
    if (obj->GetActiveState().IsTrue())
      return NPT_SUCCESS;
    return NPT_ERROR_INTERNAL;
  }

  virtual NPT_Result Visit(const PLT_PairGroup* const obj) {return Check(obj);}
  virtual NPT_Result Visit(const PLT_Partnership* const obj) {return Check(obj);}
  virtual NPT_Result Visit(const PLT_SyncRelationship* const obj)
  {
    if (!obj->GetActiveState().IsTrue())
      return NPT_ERROR_INTERNAL;
    return NPT_SUCCESS;
  }
};
