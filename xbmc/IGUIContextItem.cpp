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

#include "IGUIContextItem.h"
#include "dialogs/GUIDialogContextMenu.h"
#include "PlayListPlayer.h"
#include "playlists/PlayList.h"

#include "guilib/GUIWindowManager.h"

bool IGUIContextItem::ContextVisiblePredicate::operator()(const ContextItemPtr& item, const CGUIListItem *listItem) const
{ 
  return item->isVisible(listItem); 
}


ContextItemNowPlaying::ContextItemNowPlaying() : CGUIBaseContextItem(CONTEXT_BUTTON_NOW_PLAYING, 13350) {}

///////////////////////////////
//Context Item: Now Playing
///////////////////////////////
bool ContextItemNowPlaying::isVisible(const CGUIListItem *item) const {
  return (g_playlistPlayer.GetPlaylist(PLAYLIST_VIDEO).size() > 0); // && container.content(VIDEO)
}
bool ContextItemNowPlaying::execute() 
{  
  g_windowManager.ActivateWindow(WINDOW_VIDEO_PLAYLIST); 
  return true; 
}









