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

#include "GUIContextMenuManager.h"
#include "addons/Addon.h"
#include "addons/AddonManager.h"
#include "addons/ContextItemAddon.h"
#include "contextitems/FavoriteContextItems.h"
#include "Util.h"
#include "addons/IAddon.h"
#include <functional>
#include <boost/mem_fn.hpp>

#include <boost/bind.hpp>


GUIContextMenuManager& GUIContextMenuManager::Get() 
{
  static GUIContextMenuManager contextManager;
  return contextManager;
}

GUIContextMenuManager::GUIContextMenuManager() 
: m_vecContextMenus()
{
  ////////////
  //Core Context Items
  ////////////
  
    //general stuff
  m_vecContextMenus.push_back(CMAddRemoveFavorite());
  
  
  
    //Add context items from addons
  ADDON::VECADDONS addons;
  if(ADDON::CAddonMgr::Get().GetAddons(ADDON::ADDON_CONTEXT, addons)) 
  {
    m_vecContextMenus.reserve(addons.size());
  
    for(ADDON::IVECADDONS i = addons.begin(); i!=addons.end(); ++i) 
    {
      ContextItemPtr ptr(boost::shared_polymorphic_downcast<ADDON::CContextItemAddon>(*i));
      RegisterContextItem(ptr);
    }
  }
}


bool GUIContextMenuManager::RegisterContextItem(ContextItemPtr cm) 
{
  ContextItemPtr item = GetContextItemByID(cm->getMsgID());
  if(item==0) 
  {
    m_vecContextMenus.push_back(cm);
  } 
  else 
  {
    *item = *cm; //Item might be updated, so replace!
  }
  return true;
}

bool GUIContextMenuManager::UnregisterContextItem(ContextItemPtr cm) 
{
  contextIter it = GetContextItemIterator(cm->getMsgID());
  
  if(it!=m_vecContextMenus.end())
  {
    m_vecContextMenus.erase(it, m_vecContextMenus.end());
    return true;
  }
  return false;
}

GUIContextMenuManager::contextIter GUIContextMenuManager::GetContextItemIterator(const unsigned int ID)
{
  return find_if(m_vecContextMenus.begin(), 
                 m_vecContextMenus.end(), 
                 std::bind2nd(IGUIContextItem::IDFinder(), ID)
                 );
}

ContextItemPtr GUIContextMenuManager::GetContextItemByID(const unsigned int ID) 
{
  contextIter it = GetContextItemIterator(ID);
  if(it==m_vecContextMenus.end())
    return ContextItemPtr();
  return *it;
}

void GUIContextMenuManager::GetVisibleContextItems(int context/*TODO: */, const CFileItemPtr item, std::list<ContextItemPtr> &visible)
{  
  copy_if (m_vecContextMenus.begin(), 
           m_vecContextMenus.end(), 
           back_inserter(visible), 
           std::bind2nd(IGUIContextItem::ContextVisiblePredicate(), item)
          );
}
