/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
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
#ifdef HAS_PYTHON
#include "interfaces/python/XBPython.h"
#endif
#include <boost/lexical_cast.hpp>

using namespace std;

namespace ADDON
{

CContextItemAddon::CContextItemAddon(const cp_extension_t *ext)
  : CAddon(ext), m_bTrueOnNullId(false)
{

  m_id = CAddonMgr::Get().GetMsgIdForContextAddon(ID());
  
  m_label = CAddonMgr::Get().GetTranslatedString(ext->configuration, "label");
  if(m_label.empty())
  {
    m_label = Name();
    CLog::Log(LOGDEBUG, "ADDON: %s - failed to load label attribute, falling back to addon name %s.", ID().c_str(), Name().c_str());
  }
  
  CStdString visible = CAddonMgr::Get().GetExtValue(ext->configuration, "@visible");
  if(visible.empty())
  {
    m_bTrueOnNullId = true;
    m_VisibleId = 0;
  }
  else 
  {
    m_VisibleId = g_infoManager.Register(visible, 0, this); 
    if(!m_VisibleId)
      CLog::Log(LOGDEBUG, "ADDON: %s - Failed to load visibility expression: %s. Context item will not be visible", ID().c_str(), visible.c_str());
  }
}


CContextItemAddon::CContextItemAddon(const AddonProps &props)
  : CAddon(props), m_bTrueOnNullId(false)
{
    //TODO: find out how to get the visible and label values!

}
  
CContextItemAddon::~CContextItemAddon() 
{ 
    //TODO:
    //either implement this unregister function
    //g_infoManager.Unregister(m_VisibleId); 
    //or make sure that the IAddon* member variable of the CInfoBool class isn't used
    //after this item is destroyed (e.g. use shared_ptr/weak_ptr!)
}

CStdString CContextItemAddon::getLabel() const 
{
  return m_label;
}
  
bool CContextItemAddon::isVisible(const CGUIListItem *item) const 
{ 
  if(!Enabled())
    return false;
  if(!m_VisibleId)
    return m_bTrueOnNullId;
  return g_infoManager.GetBoolValue(m_VisibleId, item);
} 
  
    //TODO: handle non-python addons
bool CContextItemAddon::execute()
{

#ifdef HAS_PYTHON
  return  (g_pythonParser.evalFile(LibPath(), this->shared_from_this()) != -1);
#endif
  return false;
}


}
