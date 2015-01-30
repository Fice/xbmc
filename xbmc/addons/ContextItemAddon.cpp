/*
 *      Copyright (C) 2013-2014 Team XBMC
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

#include "ContextItemAddon.h"
#include "AddonManager.h"
#include "GUIInfoManager.h"
#include "utils/log.h"
#include "interfaces/info/InfoBool.h"
#include "utils/StringUtils.h"
#include "GUIContextMenuManager.h"
#include "video/dialogs/GUIDialogVideoInfo.h"
#include "dialogs/GUIDialogContextMenu.h"
#include <boost/lexical_cast.hpp>

using namespace std;

namespace ADDON
{

CContextItemAddon::CContextItemAddon(const AddonProps &props)
  : CAddon(props)
{ }

CContextItemAddon::~CContextItemAddon()
{ }

CContextItemAddon::CContextItemAddon(const cp_extension_t *ext)
  : CAddon(ext)
{

  const std::string labelStr = CAddonMgr::Get().GetExtValue(ext->configuration, "@label");
  if (labelStr.empty())
  {
    m_label = Name();
    CLog::Log(LOGDEBUG, "ADDON: %s - failed to load label attribute, falling back to addon name %s.", ID().c_str(), Name().c_str());
  }
  else
  {
    if (StringUtils::IsNaturalNumber(labelStr))
    {
      int id = boost::lexical_cast<int>( labelStr.c_str() );
      m_label = GetString(id);
      ClearStrings();
      if (m_label.empty())
      {
        CLog::Log(LOGDEBUG, "ADDON: %s - label id %i not found, using addon name %s", ID().c_str(), id, Name().c_str());
        m_label = Name();
      }
    }
    else
      m_label = labelStr;
  }

  m_parent = CAddonMgr::Get().GetExtValue(ext->configuration, "@parent");

  string visible = CAddonMgr::Get().GetExtValue(ext->configuration, "@visible");
  if (visible.empty())
  {
    visible = "false";
    CLog::Log(LOGWARNING, "ADDON: %s - No visibility expression give. Context item will not be visible.", ID().c_str());
  }
  m_visCondition = g_infoManager.Register(visible, 0);
}

bool CContextItemAddon::OnPreInstall()
{
  return CContextMenuManager::Get().Unregister(boost::dynamic_pointer_cast<CContextItemAddon>(shared_from_this()));
}

void CContextItemAddon::OnPostInstall(bool restart, bool update)
{
  if (restart)
  {
    // need to grab the local addon so we have the correct library path to run
    AddonPtr localAddon;
    if (CAddonMgr::Get().GetAddon(ID(), localAddon, ADDON_CONTEXT_ITEM))
    {
      ContextAddonPtr contextItem = boost::dynamic_pointer_cast<CContextItemAddon>(localAddon);
      if (contextItem)
        CContextMenuManager::Get().Register(contextItem);
    }
  }
}

void CContextItemAddon::OnPreUnInstall()
{
  CContextMenuManager::Get().Unregister(boost::dynamic_pointer_cast<CContextItemAddon>(shared_from_this()));
}

void CContextItemAddon::OnDisabled()
{
  CContextMenuManager::Get().Unregister(boost::dynamic_pointer_cast<CContextItemAddon>(shared_from_this()));
}
void CContextItemAddon::OnEnabled()
{
  CContextMenuManager::Get().Register(boost::dynamic_pointer_cast<CContextItemAddon>(shared_from_this()));
}

bool CContextItemAddon::IsVisible(const CFileItemPtr item) const
{
  return m_visCondition->Get(item.get());
}

}
