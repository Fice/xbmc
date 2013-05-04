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
#include "Util.h"
#include "List.h"
#include <functional>


GUIContextMenuManager& GUIContextMenuManager::Get() 
{
  static GUIContextMenuManager contextManager;
  return contextManager;
}

GUIContextMenuManager::GUIContextMenuManager() 
: m_vecContextMenus()
{
  
    //Add core context items
  
  
  m_vecContextMenus.push_back(ContextItemPtr(new ContextItemNowPlaying()));

}

bool GUIContextMenuManager::RegisterContextItem(ContextItemPtr cm) 
{
  ContextItemPtr item = GetContextItemByID(cm->getMsgID());
  if (item==0) 
  {
    m_vecContextMenus.push_back(cm);
  } 
  else 
  {
    *item = *cm; //Item might be updated, so replace!
  }
  return true;
}

ContextItemPtr GUIContextMenuManager::GetContextItemByID(const unsigned int ID) 
{
  for(contextIter it = m_vecContextMenus.begin(); it!=m_vecContextMenus.end(); ++it)
  {
    ContextItemPtr ptr = (*it)->GetByID(ID);
    if(ptr)
      return ptr;
  }
  return ContextItemPtr();
}

void GUIContextMenuManager::GetVisibleContextItems(int context/*TODO: */, const CGUIListItem * const item, std::list<ContextItemPtr> &visible)
{  
  contextIter end = m_vecContextMenus.end();
  for(contextIter i = m_vecContextMenus.begin(); i!=end; ++i)
  {
    (*i)->AddVisibleItems(item, visible);
  }
  
  /*
  copy_if (m_vecContextMenus.begin(), 
           m_vecContextMenus.end(), 
           back_inserter(visible), 
           std::bind2nd(IGUIContextItem::ContextVisiblePredicate(), item)
          );*/
}
