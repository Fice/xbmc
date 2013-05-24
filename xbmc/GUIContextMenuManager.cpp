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
#include "utils/Variant.h"
#include <list>
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
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonVideoStopScanning()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonVideoUpdateLibrary()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonVideoScanLibrary()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonVideoSetContent()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonVideoGoArtist()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonVideoGoAlbum()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonVideoPlayOther()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonVideoInfo()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonUpdateTVShow()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonMarkUnwatched()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonMarkWatched()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonEditTitle()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonUnlinkMovie()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonLinkMovie()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonSetMovieset()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonSetSeasonArt()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonSetMoviesetArt()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonMoviesetAddRemove()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonTagAddItems()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonTagRemoveItems()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonDelete()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonSetArtistThumb()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonSetActortThumb()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonUnlinkBookmark()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonFileDelete()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonFileRename()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonSetContent()));
  m_vecContextMenus.push_back(ContextItemPtr(new ContextButtonPluginSettings()));
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

void GUIContextMenuManager::GetVisibleContextItems(int context/*TODO: */, const CFileItemList& list, const CFileItem * const item, std::list<ContextItemPtr> &visible)
{  
  if (item && item->GetProperty("pluginreplacecontextitems").asBoolean())
    return;
  
  contextIter end = m_vecContextMenus.end();
  for(contextIter i = m_vecContextMenus.begin(); i!=end; ++i)
  {
    (*i)->AddVisibleItems(item, list, visible);
  }
  
  /*
  copy_if (m_vecContextMenus.begin(), 
           m_vecContextMenus.end(), 
           back_inserter(visible), 
           std::bind2nd(IGUIContextItem::ContextVisiblePredicate(), item)
          );*/
}
