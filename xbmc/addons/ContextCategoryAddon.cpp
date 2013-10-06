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

#include "ContextCategoryAddon.h"
#include "AddonManager.h"
#include "GUIInfoManager.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "interfaces/generic/ScriptInvocationManager.h"
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include "dialogs/GUIDialogContextMenu.h"

using namespace std;


namespace ADDON
{

CContextCategoryAddon::CContextCategoryAddon(const cp_extension_t *ext)
  : CContextItemAddon(ext)
{ }


CContextCategoryAddon::CContextCategoryAddon(const AddonProps &props)
  : CContextItemAddon(props)
{ }
  
CContextCategoryAddon::~CContextCategoryAddon() 
{ }
  
bool CContextCategoryAddon::isVisible(const CFileItemPtr item) const 
{ 
  if (!Enabled())
    return false;
  
  typedef std::vector<ADDON::ContextAddonPtr>::const_iterator iter;
  
  iter end = m_vecContextMenus.end();
  for(iter i = m_vecContextMenus.begin(); i != end; ++i)
  {
    if((*i)->isVisible(item))
      return true;
  }
  return false;
} 
  
void CContextCategoryAddon::addIfVisible(const CFileItemPtr item, std::list<ContextAddonPtr> &visible)
{  
  if(m_vecContextMenus.size()==1)
    m_vecContextMenus[0]->addIfVisible(item, visible);
  else if(isVisible(item))
    visible.push_back(boost::static_pointer_cast<IContextItem>(shared_from_this()));
}

  
ADDON::ContextAddonPtr CContextCategoryAddon::GetChildWithID(const std::string& strID) 
{ 
  if (ID()==strID)
    return boost::static_pointer_cast<IContextItem>(shared_from_this());
  return GetContextItemByID(strID); 
}
  
bool CContextCategoryAddon::execute(const CFileItemPtr item)
{
  CContextButtons choices;  
  std::list<ContextAddonPtr> additional_context_items;
  
  GetVisibleContextItems(0, item, additional_context_items);
  std::transform(additional_context_items.begin(), additional_context_items.end(), back_inserter(choices), ConvertFromContextItem());      
  
  int button = CGUIDialogContextMenu::ShowAndGetChoice(choices);
  
  ADDON::ContextAddonPtr context_item = GetContextItemByID(button);
  if(context_item!=0)
    return (*context_item)(item); //execute our context item logic
  return false;
}


}
