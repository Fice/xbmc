#pragma once
/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"
#include "PltMediaItem.h"

class PLT_ResetObjectList
{
public:
  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

private:
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
public:
  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;

protected:
  NPT_List<PLT_Change> m_changes;
};
