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
#include "interfaces/legacy/ListItem.h"
#include "interfaces/python/swig.h"
#endif
#include <boost/lexical_cast.hpp>
#include <stdexcept>

using namespace std;


namespace PythonBindings
{
  extern PyTypeObject PyXBMCAddon_xbmcgui_ListItem_Type;
  extern TypeInfo TyXBMCAddon_xbmcgui_ListItem_Type;
}

namespace ADDON
{

CContextItemAddon::CContextItemAddon(const cp_extension_t *ext)
  : CAddon(ext), m_bTrueOnNullId(false)
{

  m_id = CAddonMgr::Get().GetMsgIdForContextAddon(ID());
  
  CStdString labelStr = CAddonMgr::Get().GetExtValue(ext->configuration, "@label");
  if(labelStr.empty())
  {
    m_label = Name();
    CLog::Log(LOGDEBUG, "ADDON: %s - failed to load label attribute, falling back to addon name %s.", ID().c_str(), Name().c_str());
  } else {
    if(StringUtils::IsNaturalNumber(labelStr)) {
      int id = boost::lexical_cast<int>(labelStr);
      m_label = GetString(id);
      if(m_label.empty()) {
        CLog::Log(LOGDEBUG, "ADDON: %s - label id %i not found using addon name %s", ID().c_str(), id, Name().c_str());
        m_label = Name();
      }
    } else {
      m_label = labelStr;
    }
    
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

CStdString CContextItemAddon::getLabel()
{
  return m_label;
}
  
bool CContextItemAddon::isVisible(const CFileItemPtr item) const 
{ 
  if(!Enabled())
    return false;
  if(!m_VisibleId)
    return m_bTrueOnNullId;
  return g_infoManager.GetBoolValue(m_VisibleId, &*item);
} 
  
    //TODO: handle non-python addons
bool CContextItemAddon::execute(const CFileItemPtr item)
{
  vector<CStdString> args;
  args.push_back(item->GetPath());
    
#ifdef HAS_PYTHON
  XBMCAddon::xbmcgui::ListItem* arg = new XBMCAddon::xbmcgui::ListItem(item);
  
  PyObject *py_arg = makePythonInstance(arg, 
                                        &PythonBindings::PyXBMCAddon_xbmcgui_ListItem_Type, 
                                        &PythonBindings::TyXBMCAddon_xbmcgui_ListItem_Type, 
                                        true); //TODO: i have no idea if this is supposed to work
  
  return  (g_pythonParser.evalFile(LibPath(), py_arg, "item", this->shared_from_this()) != -1);
  
    //TODO: check if arg and py_arg gets deleted when script is done... no need for a memory leak
#endif
  return false;
}


}
