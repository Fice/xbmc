/*
 *      Copyright (C) 2005-2013 Team XBMC
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
#include "interfaces/generic/ScriptInvocationManager.h"
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include "utils/StringUtils.h"
#include "GUIContextMenuManager.h"
#include "addons/ContextCategoryAddon.h"
#include "video/dialogs/GUIDialogVideoInfo.h"

using namespace std;

namespace ADDON
{
  
IContextItem::IContextItem(const AddonProps &props)
  : CAddon(props)
{ }
  
IContextItem::~IContextItem() 
{ }

IContextItem::IContextItem(const cp_extension_t *ext)
  : CAddon(ext)
{
  //generate a unique ID for every context item entry
  m_id = CAddonMgr::Get().GetMsgIdForContextAddon(ID());
    
  CStdString labelStr = CAddonMgr::Get().GetExtValue(ext->configuration, "@label");
  if (labelStr.empty())
  {
    m_label = Name();
    CLog::Log(LOGDEBUG, "ADDON: %s - failed to load label attribute, falling back to addon name %s.", ID().c_str(), Name().c_str());
  } 
  else
  {
    if (StringUtils::IsNaturalNumber(labelStr))
    {
      int id = boost::lexical_cast<int>(labelStr);
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
}

void IContextItem::Register()
{
  ContextAddonPtr ptr = boost::dynamic_pointer_cast<IContextItem, IAddon>(shared_from_this());
  BaseContextMenuManager::Get().RegisterContextItem(ptr);
}

void IContextItem::Unregister()
{
  ContextAddonPtr ptr = boost::dynamic_pointer_cast<IContextItem, IAddon>(shared_from_this());
  BaseContextMenuManager::Get().UnregisterContextItem(ptr);
}
  
bool IContextItem::operator()(const CFileItemPtr item) 
{
  if (!isVisible(item))
    return false;
  return execute(item);
}

CContextItemAddon::CContextItemAddon(const cp_extension_t *ext)
  : IContextItem(ext)
{
  CStdString visible = CAddonMgr::Get().GetExtValue(ext->configuration, "@visible");
  
  if (visible.empty())
  {
    m_VisibleId = g_infoManager.Register("true", 0);
    CLog::Log(LOGDEBUG, "ADDON: %s - No visibility expression given. Context item will always be visible", ID().c_str());
    
  }
  else
  {
    m_VisibleId = g_infoManager.Register(visible, 0); 
    if (!m_VisibleId)
      CLog::Log(LOGDEBUG, "ADDON: %s - Failed to load visibility expression: %s. Context item will not be visible", ID().c_str(), visible.c_str());
  }
}

CContextItemAddon::CContextItemAddon(const AddonProps &props)
  : IContextItem(props)
{ }

CContextItemAddon::~CContextItemAddon() 
{ }

bool CContextItemAddon::isVisible(const CFileItemPtr item) const 
{ 
  if (!m_VisibleId)
    return false;

  return g_infoManager.GetBoolValue(m_VisibleId, &*item);
} 
  
void CContextItemAddon::addIfVisible(const CFileItemPtr item, std::list<ContextAddonPtr> &visible)
{
  if (isVisible(item))
    visible.push_back(boost::dynamic_pointer_cast<IContextItem>(shared_from_this()));
}
  
bool CContextItemAddon::execute(const CFileItemPtr item)
{
  return (CScriptInvocationManager::Get().Execute(LibPath(), this->shared_from_this(), std::vector<string>(), item) != -1);
}

}
