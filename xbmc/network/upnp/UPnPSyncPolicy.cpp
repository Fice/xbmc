/*
 *      Copyright (C) 2014 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#include <Platinum/Source/Platinum/Platinum.h>
#include "UPnPSyncPolicy.h"

namespace UPNP
{


    NPT_Result CustomSyncPolicy::Handle(const PLT_SyncPolicy& policy,
      NPT_String& localChangesets,
      NPT_String& remoteChangesets)
    {
      if (policy.GetPolicyType() != "kodi_custom")
        return NPT_ERROR_INVALID_PARAMETERS;
      return NPT_ERROR_NOT_IMPLEMENTED;
    }

    NPT_Result ReplacePolicyHandler::Handle(const PLT_SyncPolicy& policy,
      NPT_String& localChangesets,
      NPT_String& remoteChangesets)
    {
      if (policy.GetPolicyType() != "replace")
        return NPT_ERROR_INVALID_PARAMETERS;

      return NPT_ERROR_NOT_IMPLEMENTED;
    }

    NPT_Result MergePolicyHandler::Handle(const PLT_SyncPolicy& policy,
      NPT_String& localChangesets,
      NPT_String& remoteChangesets)
    {
      if (policy.GetPolicyType() != "merge")
        return NPT_ERROR_INVALID_PARAMETERS;

      return NPT_ERROR_NOT_IMPLEMENTED;
    }

    NPT_Result BlendPolicyHandler::Handle(const PLT_SyncPolicy& policy,
      NPT_String& localChangesets,
      NPT_String& remoteChangesets)
    {
      if (policy.GetPolicyType() != "blend")
        return NPT_ERROR_INVALID_PARAMETERS;

      return NPT_ERROR_NOT_IMPLEMENTED;
    }

    NPT_Result TrackingPolicyHandler::Handle(const PLT_SyncPolicy& policy,
      NPT_String& localChangesets,
      NPT_String& remoteChangesets)
    {
      if (policy.GetPolicyType() != "tracking")
        return NPT_ERROR_INVALID_PARAMETERS;
      return NPT_ERROR_NOT_IMPLEMENTED;
    }

    SyncPolicyHandlerInterface* SyncPolicyFactory::Create(const NPT_String& policyType)
    {
      if (policyType == "replace")
        return new ReplacePolicyHandler();
      if (policyType == "merge")
        return new MergePolicyHandler();
      if (policyType == "blend")
        return new BlendPolicyHandler();
      if (policyType == "tracking")
        return new TrackingPolicyHandler();
      if (policyType == "kodi_custom")
        return new CustomSyncPolicy();
      return NULL;
    }

}
