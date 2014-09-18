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
#pragma once

namespace UPNP
{

class SyncPolicyHandlerInterface
{
public:
  virtual NPT_Result Handle(const PLT_SyncPolicy& policy,
                            NPT_String& localChangesets,
                            NPT_String& remoteChangesets) = 0;
};

class ReplacePolicyHandler : public SyncPolicyHandlerInterface
{
public:
  virtual NPT_Result Handle(const PLT_SyncPolicy& policy,
                            NPT_String& localChangesets,
                            NPT_String& remoteChangesets);
};

class MergePolicyHandler : public SyncPolicyHandlerInterface
{
public:
  virtual NPT_Result Handle(const PLT_SyncPolicy& policy,
                            NPT_String& localChangesets,
                            NPT_String& remoteChangesets);
};

class BlendPolicyHandler : public SyncPolicyHandlerInterface
{
public:
  virtual NPT_Result Handle(const PLT_SyncPolicy& policy,
                            NPT_String& localChangesets,
                            NPT_String& remoteChangesets);
};

class TrackingPolicyHandler : public SyncPolicyHandlerInterface
{
public:
  virtual NPT_Result Handle(const PLT_SyncPolicy& policy,
                            NPT_String& localChangesets,
                            NPT_String& remoteChangesets);
};

class CustomSyncPolicy : public SyncPolicyHandlerInterface
{
public:
  virtual NPT_Result Handle(const PLT_SyncPolicy& policy,
                            NPT_String& localChangesets,
                            NPT_String& remoteChangesets);
};

class SyncPolicyFactory
{
public:
  virtual SyncPolicyHandlerInterface* Create(const NPT_String& policyType);
};

} /* namespace UPNP */
