#include "PltSyncValueObservers.h"
#include "PltUtilities.h"
#include "PltRelationshipStructures.h"

PLT_SyncChangeObserver::PLT_SyncChangeObserver(PLT_StateVariable* stateVariable)
	: m_stateVariable(stateVariable)
{
  NPT_String value;
  ToXml(value);
	m_stateVariable->SetValue(value);
	m_stateVariable->SetRate( NPT_TimeInterval(2.) );
}

void PLT_SyncChangeObserver::OnSyncDataChanged(const PLT_SyncStructure* const sync)
{
  OnSyncDataChanged(sync->GetID());
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

void PLT_SyncChangeObserver::OnSyncDataSynced(const PLT_SyncStructure* const sync)
{
  OnSyncDataSynced(sync->GetID());
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
