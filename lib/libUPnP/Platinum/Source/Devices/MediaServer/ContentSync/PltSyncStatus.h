#pragma once
/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Neptune.h"

#define STU_STATUS_IN_PROGRESS            "IN_PROGRESS"
#define STU_STATUS_IN_PROGRESS_WITH_ERROR "IN_PROGRESS_WITH_ERROR"
#define STU_STATUS_COMPLETED              "COMPLETED"
#define STU_STATUS_COMPLETED_WITH_ERROR   "COMPLETED_WITH_ERROR"
#define STU_STATUS_STOPPED                "STOPPED"
#define STU_STATUS_TEMPORARILY_STOPPED    "TEMPORARILY_STOPPED"

struct PLT_SyncStatusUpdate
{
   //TODO:
};

struct PLT_SyncStatus
{
  PLT_SyncStatusUpdate m_ongoingSyncStatus;
  PLT_SyncStatusUpdate m_lastSyncStatus;

  NPT_Result FromXml(NPT_XmlElementNode*);
  NPT_Result ToXml(NPT_String& result) const;
};

struct PLT_SyncStatusInformation
{
	PLT_SyncStatusInformation(NPT_UInt32 total, NPT_UInt32 complete = 0, NPT_UInt32 failed = 0)
		: totalObjects(total), completeObjects(complete), failedObjects(failed)
	{}

	NPT_UInt32 totalObjects;
	NPT_UInt32 completeObjects;
	NPT_UInt32 failedObjects;
};

struct PLT_SyncLogEntry
{
	NPT_String localObjID;
	NPT_String remoteObjID;
	NPT_UInt32 statusCode;

	bool IsError() { return statusCode > 99; }
	bool IsSuccessfull() { return statusCode < 100; }
};

struct PLT_SyncStatusPairGroup
{
	NPT_List<PLT_SyncLogEntry> logEntries;
	PLT_SyncStatusInformation statusInformation;
	NPT_String id;
};

struct PLT_SyncStatusPartnership
{
	PLT_SyncStatusInformation statusInformaton;
	NPT_String id;
	PLT_SyncStatusPairGroup child;
};

struct PLT_SyncStatusRelationship
{
	PLT_SyncStatusInformation statusInformaton;
	NPT_String id;
	PLT_SyncStatusPartnership child;
};

