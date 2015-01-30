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

#include "GUIContextMenuManager.h"
#include "addons/Addon.h"
#include "addons/AddonManager.h"
#include "addons/ContextItemAddon.h"
#include "Util.h"
#include "addons/IAddon.h"
#include "video/dialogs/GUIDialogVideoInfo.h"
#include "utils/log.h"

using namespace ADDON;
using namespace std;

typedef map<unsigned int, ContextAddonPtr>::const_iterator ContextAddonIter;
typedef map<unsigned int, ContextAddonPtr>::value_type ValueType;

struct AddonIdCmp : binary_function<string, ValueType, bool>
{
  bool operator()(const std::string& id, const ValueType& elem) const
  {
    return id == elem.second->ID();
  }
};

CContextMenuManager::CContextMenuManager()
  : m_iCurrentContextId(CONTEXT_BUTTON_FIRST_CONTEXT_PLUGIN)
{
  Init();
}

CContextMenuManager& CContextMenuManager::Get()
{
  static CContextMenuManager mgr;
  return mgr;
}

void CContextMenuManager::Init()
{
  //Make sure we load all context items on first usage...
  VECADDONS items;
  CAddonMgr::Get().GetAddons(ADDON_CONTEXT_ITEM, items);
  for (VECADDONS::const_iterator it = items.begin(); it != items.end(); ++it)
    Register(boost::static_pointer_cast<IContextItem>(*it));
}

void CContextMenuManager::Register(ContextAddonPtr cm)
{
  ContextAddonIter it = find_if(m_contextAddons.begin(),
                                m_contextAddons.end(),
                                bind1st(AddonIdCmp(), cm->ID()));
  // TODO: is this necessary?
  if (it != m_contextAddons.end())
    m_contextAddons[it->first] = cm;
  else
    m_contextAddons[m_iCurrentContextId++] = cm;
}

bool CContextMenuManager::Unregister(ContextAddonPtr cm)
{
  ContextAddonIter it = find_if(m_contextAddons.begin(),
                                m_contextAddons.end(),
                                bind1st(AddonIdCmp(), cm->ID()));
  if (it != m_contextAddons.end())
  {
    m_contextAddons.erase(it, m_contextAddons.end());
    return true;
  }
  return false;
}

ContextAddonPtr CContextMenuManager::GetContextItemByID(unsigned int id)
{
  ContextAddonIter it = m_contextAddons.find(id);
  if (it != m_contextAddons.end())
  {
    return it->second;
  }
  return ContextAddonPtr();
}

void CContextMenuManager::AppendVisibleContextItems(const CFileItemPtr item, CContextButtons& list, const std::string& parent)
{
  for (ContextAddonIter it = m_contextAddons.begin(); it != m_contextAddons.end(); ++it)
  {
    if (it->second->GetParent() == parent && it->second->IsVisible(item))
    {
      list.push_back(make_pair(it->first, it->second->GetLabel()));
    }
  }
}

