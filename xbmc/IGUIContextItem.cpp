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
#include "GUIUserMessages.h"
#include "Application.h"
#include "video/VideoDatabase.h"
#include "video/VideoInfoScanner.h"
#include "video/windows/GUIWindowVideoBase.h"
#include "video/windows/GUIWindowVideoNav.h"
#include "profiles/ProfilesManager.h"
#include "utils/URIUtils.h"
#include "music/MusicDatabase.h"
#include "settings/AdvancedSettings.h"
#include "filesystem/VideoDatabaseDirectory.h"
#include "Util.h"
#include "settings/GUISettings.h"
#include "ApplicationMessenger.h"
#include "addons/AddonManager.h"
#include "addons/GUIDialogAddonSettings.h"

bool IGUIContextItem::ContextVisiblePredicate::operator()(const ContextItemPtr& item) const
{ 
  return item->isVisible(m_listItem, m_container); 
}


void IGUIContextItem::AddVisibleItems(const CFileItem* listItem, const CFileItemList& container, std::list<ContextItemPtr>& list)
{
  if(isVisible(listItem, container))
    list.push_back(shared_from_this());
}


ContextItemNowPlaying::ContextItemNowPlaying() : CGUIBaseContextItem(CONTEXT_BUTTON_NOW_PLAYING, 13350) {}

  ///////////////////////////////
  //Context Item: Now Playing
  ///////////////////////////////
bool ContextItemNowPlaying::isVisible(const CFileItem *item, const CFileItemList& container) const 
{
  return (g_playlistPlayer.GetPlaylist(PLAYLIST_VIDEO).size() > 0); // && container.content(VIDEO)
}
bool ContextItemNowPlaying::execute(const CFileItem *item, const CFileItemList& container) 
{  
  g_windowManager.ActivateWindow(WINDOW_VIDEO_PLAYLIST); 
  return true; 
}

bool ContextButtonVideoStopScanning::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  return (container.IsVideoDb() || container.GetPath().Equals("sources://video/") )
  && !(item->IsPlugin() || item->IsScript() || container.IsPlugin())
  && g_application.IsVideoScanning();
}

bool ContextButtonVideoStopScanning::execute(const CFileItem *item, const CFileItemList& container) 
{  
  g_application.StopVideoScan();
  return true;
}

bool ContextButtonVideoUpdateLibrary::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  return (container.IsVideoDb() || container.GetPath().Equals("sources://video/"))
  && !g_application.IsVideoScanning();
}

bool ContextButtonVideoUpdateLibrary::execute(const CFileItem *item, const CFileItemList& container) 
{  
  g_application.StartVideoScan("", false);
  return true;
}

bool ContextButtonVideoScanLibrary::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  CVideoDatabase database;
  database.Open();
  ADDON::ScraperPtr info = database.GetScraperForPath(item->GetPath());
  database.Close();
  bool inPlaylists = container.GetPath().Equals(CUtil::VideoPlaylistsLocation()) ||
  container.GetPath().Equals("special://videoplaylists/");
  return (container.GetPath().Equals("sources://video/") &&
          !g_application.IsVideoScanning() &&
          info) || 
         (item && !item->IsParentFolder() && !container.IsVideoDb() && !container.IsVirtualDirectoryRoot() &&
         (g_guiSettings.GetBool("filelists.allowfiledeletion") &&
          CUtil::SupportsWriteFileOperations(item->GetPath())) || (inPlaylists && !URIUtils::GetFileName(item->GetPath()).Equals("PartyMode-Video.xsp") &&
         (item->IsPlayList() || item->IsSmartPlayList())));
}

bool ContextButtonVideoScanLibrary::execute(const CFileItem *item, const CFileItemList& container) 
{  
  if( !item)
    return false;
  ADDON::ScraperPtr info;
  VIDEO::SScanSettings settings;
  CGUIWindowVideoBase::GetScraperForItem(container, item, info, settings);
  CStdString strPath = item->GetPath();
  if (item->IsVideoDb() && (!item->m_bIsFolder || item->GetVideoInfoTag()->m_strPath.IsEmpty()))
    return false;
  
  if (item->IsVideoDb())
    strPath = item->GetVideoInfoTag()->m_strPath;
  
  if (!info || info->Content() == CONTENT_NONE)
    return false;
  
  if (item->m_bIsFolder)
  {
    CVideoDatabase database;
    database.SetPathHash(strPath, ""); // to force scan
    g_application.StartVideoScan(strPath, true);
  }
  else
    OnInfo(item, info);
  
  return true;
}

bool ContextButtonVideoSetContent::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  return item
      && container.GetPath().Equals("sources://video/")
      && !item->IsDVD() 
      && item->GetPath() != "add" 
      && !item->IsParentFolder() 
      && (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser)
      && !g_application.IsVideoScanning()
      && !item->IsLiveTV() 
      && !item->IsPlugin() 
      && !item->IsAddonsPath() 
      && !URIUtils::IsUPnP(item->GetPath());
}

bool ContextButtonVideoSetContent::execute(const CFileItem *item, const CFileItemList& container) 
{  
  
}

CStdString ContextButtonVideoSetContent::getLabel(const CFileItem *item, const CFileItemList& container) const
{
  CVideoDatabase database;
  database.Open();
  ADDON::ScraperPtr info = database.GetScraperForPath(item->GetPath());
  database.Close();
  if(info && info->Content() != CONTENT_NONE)
    return g_localizeStrings.Get(20442);
  return g_localizeStrings.Get(20333);
}

bool ContextButtonVideoGoArtist::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  if (container.IsVideoDb() && item && item->HasVideoInfoTag() && !item->GetVideoInfoTag()->m_artist.empty())
  {
    CMusicDatabase database;
    database.Open();
    bool result = (database.GetArtistByName(StringUtils::Join(item->GetVideoInfoTag()->m_artist, g_advancedSettings.m_videoItemSeparator)) > -1);
    database.Close();
    return result;
  }
  return false;
}

bool ContextButtonVideoGoArtist::execute(const CFileItem *item, const CFileItemList& container) 
{  
  CStdString strPath;
  CMusicDatabase database;
  database.Open();
  strPath.Format("musicdb://artists/%ld/",database.GetArtistByName(StringUtils::Join(item->GetVideoInfoTag()->m_artist, g_advancedSettings.m_videoItemSeparator)));
  g_windowManager.ActivateWindow(WINDOW_MUSIC_NAV,strPath);
  return true;
}

bool ContextButtonVideoGoAlbum::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  if (container.IsVideoDb() && item && item->HasVideoInfoTag() && !item->GetVideoInfoTag()->m_strAlbum.empty())
  {
    CMusicDatabase database;
    database.Open();
    return (database.GetAlbumByName(item->GetVideoInfoTag()->m_strAlbum) > -1);
  }
  return false;
}

bool ContextButtonVideoGoAlbum::execute(const CFileItem *item, const CFileItemList& container) 
{  
  CStdString strPath;
  CMusicDatabase database;
  database.Open();
  strPath.Format("musicdb://albums/%ld/",database.GetAlbumByName(item->GetVideoInfoTag()->m_strAlbum));
  g_windowManager.ActivateWindow(WINDOW_MUSIC_NAV,strPath);
  return true;
}

bool ContextButtonVideoPlayOther::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  if (item &&
      container.IsVideoDb() &&
      item->HasVideoInfoTag() && !item->GetVideoInfoTag()->m_strAlbum.empty() &&
      !item->GetVideoInfoTag()->m_artist.empty()                              &&
      !item->GetVideoInfoTag()->m_strTitle.empty())
  {
    CMusicDatabase database;
    database.Open();
    return (database.GetSongByArtistAndAlbumAndTitle(StringUtils::Join(item->GetVideoInfoTag()->m_artist, g_advancedSettings.m_videoItemSeparator),
                                                     item->GetVideoInfoTag()->m_strAlbum,
                                                     item->GetVideoInfoTag()->m_strTitle) > -1);
  }
  return false;
}

bool ContextButtonVideoPlayOther::execute(const CFileItem *item, const CFileItemList& container) 
{  
  CMusicDatabase database;
  database.Open();
  CSong song;
  if (database.GetSongById(database.GetSongByArtistAndAlbumAndTitle(StringUtils::Join(item->GetVideoInfoTag()->m_artist, g_advancedSettings.m_videoItemSeparator), 
                                                                    item->GetVideoInfoTag()->m_strAlbum,
                                                                    item->GetVideoInfoTag()->m_strTitle),
                           song))
  {
    CApplicationMessenger::Get().PlayFile(song);
  }
  return true;
}

bool ContextButtonVideoInfo::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  if (item && !item->IsParentFolder())
  {
    ADDON::ScraperPtr info;
    VIDEO::SScanSettings settings;
    CGUIWindowVideoBase::GetScraperForItem(container, item, info, settings);
    
    return (info);
  }
  return false;
}

bool ContextButtonVideoInfo::execute(const CFileItem *item, const CFileItemList& container) 
{  
  OnInfo(itemNumber);
  return true;
}

CStdString ContextButtonVideoInfo::getLabel(const CFileItem *item, const CFileItemList& container) const
{
  ADDON::ScraperPtr info;
  VIDEO::SScanSettings settings;
  CGUIWindowVideoBase::GetScraperForItem(container, item, info, settings);
  
  if (info && info->Content() == CONTENT_TVSHOWS)
    return g_localizeStrings.Get(item->m_bIsFolder ? 20351 : 20352);
  else if (info && info->Content() == CONTENT_MUSICVIDEOS)
    return g_localizeStrings.Get(20393);
  else if (info && info->Content() == CONTENT_MOVIES)
    return g_localizeStrings.Get(13346);
  return "";
}

bool ContextButtonUpdateTVShow::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  if (!g_application.IsVideoScanning() && 
      item &&
      !item->IsParentFolder() &&
      CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser)
  {
    XFILE::CVideoDatabaseDirectory dir;
    XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE node = dir.GetDirectoryChildType(container.GetPath());
    return (node == XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE_TITLE_TVSHOWS);
  }
  return false;
}

bool ContextButtonMarkUnwatched::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  return item &&
  (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser &&
   !item->IsParentFolder() &&
   !item->IsPlugin() && !item->IsScript() && !item->IsLiveTV() && !item->IsAddonsPath() &&
   item->GetPath() != "sources://video/" && item->GetPath() != "special://videoplaylists/" &&
   item->GetPath().Left(19) != "newsmartplaylist://" && item->GetPath().Left(14) != "newplaylist://" &&
   item->GetPath().Left(9) != "newtag://") &&
  (item->m_bIsFolder || item->GetOverlayImage().Equals("OverlayWatched.png"));
}

bool ContextButtonMarkUnwatched::execute(const CFileItem *item, const CFileItemList& container) 
{  
  MarkWatched(item,false);
  CUtil::DeleteVideoDatabaseDirectoryCache();
  CGUIMessage msg(GUI_MSG_UPDATE, g_windowManager.GetActiveWindow(), 0);
  g_windowManager.SendMessage(msg);
  return true;
}

bool ContextButtonMarkWatched::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  return (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser &&
          item &&
          !item->IsParentFolder() &&
          !item->IsPlugin() && !item->IsScript() && !item->IsLiveTV() && !item->IsAddonsPath() &&
          item->GetPath() != "sources://video/" && item->GetPath() != "special://videoplaylists/" &&
          item->GetPath().Left(19) != "newsmartplaylist://" && item->GetPath().Left(14) != "newplaylist://" &&
          item->GetPath().Left(9) != "newtag://") &&
  (item->m_bIsFolder || !item->GetOverlayImage().Equals("OverlayWatched.png"));
}

bool ContextButtonMarkWatched::execute(const CFileItem *item, const CFileItemList& container) 
{  
  int newSelection = m_viewControl.GetSelectedItem() + 1;
  MarkWatched(item,true);
  m_viewControl.SetSelectedItem(newSelection);
  
  CUtil::DeleteVideoDatabaseDirectoryCache();
  CGUIMessage msg(GUI_MSG_UPDATE, g_windowManager.GetActiveWindow(), 0);
  g_windowManager.SendMessage(msg);
  return true;
}

bool ContextButtonEditTitle::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  if (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser && item && !item->IsParentFolder())
  {
    if (!((item->IsVideoDb() && item->HasVideoInfoTag() && !item->m_bIsFolder) ||
          (StringUtils::StartsWith(item->GetPath(), "videodb://movies/sets/") && item->GetPath().size() > 22 && item->m_bIsFolder)))
    {
      XFILE::CVideoDatabaseDirectory dir;
      XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE node = dir.GetDirectoryChildType(container.GetPath());
      return (node == XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE_TITLE_TVSHOWS);
    }
    return true;
  }
  return false;
}

bool ContextButtonEditTitle::execute(const CFileItem *item, const CFileItemList& container) 
{  
  UpdateVideoTitle(item);
  CUtil::DeleteVideoDatabaseDirectoryCache();
  CGUIMessage msg(GUI_MSG_UPDATE, g_windowManager.GetActiveWindow(), 0);
  g_windowManager.SendMessage(msg);
  return true;
}

bool ContextButtonUnlinkMovie::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  CVideoDatabase database;
  database.Open();
  return (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser) &&
  item && !item->IsParentFolder() &&
  (database.HasContent(VIDEODB_CONTENT_TVSHOWS) && item->HasVideoInfoTag() &&
   !item->m_bIsFolder && item->GetVideoInfoTag()->m_iEpisode == -1 &&
   item->GetVideoInfoTag()->m_artist.empty() && item->GetVideoInfoTag()->m_iDbId >= 0) &&
  database.IsLinkedToTvshow(item->GetVideoInfoTag()->m_iDbId);
}

bool ContextButtonUnlinkMovie::execute(const CFileItem *item, const CFileItemList& container) 
{  
  OnLinkMovieToTvShow(itemNumber, true);
  CGUIMessage msg(GUI_MSG_UPDATE, g_windowManager.GetActiveWindow(), 0);
  g_windowManager.SendMessage(msg);
  return true;
}

bool ContextButtonLinkMovie::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  
  CVideoDatabase database;
  database.Open();
  return (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser) &&
          item && !item->IsParentFolder() &&
         (database.HasContent(VIDEODB_CONTENT_TVSHOWS) && item->HasVideoInfoTag() &&
         !item->m_bIsFolder && item->GetVideoInfoTag()->m_iEpisode == -1 &&
          item->GetVideoInfoTag()->m_artist.empty() && item->GetVideoInfoTag()->m_iDbId >= 0);
}

bool ContextButtonLinkMovie::execute(const CFileItem *item, const CFileItemList& container) 
{  
  OnLinkMovieToTvShow(itemNumber, false);
  return true;
}

bool ContextButtonSetMovieset::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  return (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser) &&
          item && !item->IsParentFolder() &&
         (item->HasVideoInfoTag() && item->GetVideoInfoTag()->m_type == "movie");
}

bool ContextButtonSetMovieset::execute(const CFileItem *item, const CFileItemList& container) 
{  
  CFileItemPtr selectedSet;
  if (!GetSetForMovie(item, selectedSet))
    return true;
  
  if (SetMovieSet(item, selectedSet))
  {
    CGUIMessage msg(GUI_MSG_UPDATE, g_windowManager.GetActiveWindow(), 0);
    g_windowManager.SendMessage(msg);
  }
  
  return true;
}

bool ContextButtonSetSeasonArt::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  XFILE::CVideoDatabaseDirectory dir;
  XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE node = dir.GetDirectoryChildType(container.GetPath());
  return (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser) &&
          item && !item->IsParentFolder() &&
         (node == XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE_SEASONS && item->m_bIsFolder);
}

bool ContextButtonSetSeasonArt::execute(const CFileItem *item, const CFileItemList& container) 
{  
  
}

bool ContextButtonSetMoviesetArt::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  return (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser) &&
          item && !item->IsParentFolder() &&
         (StringUtils::StartsWith(item->GetPath(), "videodb://movies/sets/") && item->GetPath().size() > 22 && item->m_bIsFolder);
}

bool ContextButtonSetMoviesetArt::execute(const CFileItem *item, const CFileItemList& container) 
{  
  
}

bool ContextButtonMoviesetAddRemove::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  return (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser) &&
          item && !item->IsParentFolder() &&
         (StringUtils::StartsWith(item->GetPath(), "videodb://movies/sets/") && item->GetPath().size() > 22 && item->m_bIsFolder);
}

bool ContextButtonMoviesetAddRemove::execute(const CFileItem *item, const CFileItemList& container) 
{  
  CFileItemList originalItems;
  CFileItemList selectedItems;
  
  if (!GetMoviesForSet(item, originalItems, selectedItems) || selectedItems.Size() == 0) // need at least one item selected
    return true;
  VECFILEITEMS original = originalItems.GetList();
  std::sort(original.begin(), original.end(), compFileItemsByDbId);
  VECFILEITEMS selected = selectedItems.GetList();
  std::sort(selected.begin(), selected.end(), compFileItemsByDbId);
  
  bool refreshNeeded = false;
    // update the "added" items
  VECFILEITEMS addedItems;
  set_difference(selected.begin(),selected.end(), original.begin(),original.end(), std::back_inserter(addedItems), compFileItemsByDbId);
  for (VECFILEITEMS::iterator it = addedItems.begin();  it != addedItems.end(); ++it)
  {
    if (SetMovieSet(*it, item))
      refreshNeeded = true;
  }
    // update the "deleted" items
  CFileItemPtr clearItem(new CFileItem());
  clearItem->GetVideoInfoTag()->m_iDbId = -1; // -1 will be used to clear set
  VECFILEITEMS deletedItems;
  set_difference(original.begin(),original.end(), selected.begin(),selected.end(), std::back_inserter(deletedItems), compFileItemsByDbId);
  for (VECFILEITEMS::iterator it = deletedItems.begin();  it != deletedItems.end(); ++it)
  {
    if (SetMovieSet(*it, clearItem))
      refreshNeeded = true;
  }
  
    // we need to clear any cached version of this tag's listing
  if (refreshNeeded) 
  {
    CGUIMessage msg(GUI_MSG_UPDATE, g_windowManager.GetActiveWindow(), 0);
    g_windowManager.SendMessage(msg);
  }
  return true;
}

bool ContextButtonTagAddItems::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  CVideoDbUrl videoUrl;
  return (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser) &&
  item && !item->IsParentFolder() &&
  container.GetContent() == "tags" && item->m_bIsFolder &&
  videoUrl.FromString(item->GetPath());
}

bool ContextButtonTagAddItems::execute(const CFileItem *item, const CFileItemList& container) 
{  
  
  CVideoDbUrl videoUrl;
  if (!videoUrl.FromString(item->GetPath()))
    return false;
  
  std::string mediaType = videoUrl.GetItemType();
  mediaType = mediaType.substr(0, mediaType.length() - 1);
  
  CFileItemList items;
  CStdString localizedType = CGUIWindowVideoNav::GetLocalizedType(mediaType);
  CStdString strLabel; strLabel.Format(g_localizeStrings.Get(20464), localizedType.c_str());
  if (!GetItemsForTag(strLabel, mediaType, items, item->GetVideoInfoTag()->m_iDbId))
    return true;
  
  CVideoDatabase videodb;
  if (!videodb.Open())
    return true;
  
  for (int index = 0; index < items.Size(); index++)
  {
    if (!items[index]->HasVideoInfoTag() || items[index]->GetVideoInfoTag()->m_iDbId <= 0)
      continue;
    
    videodb.AddTagToItem(items[index]->GetVideoInfoTag()->m_iDbId, item->GetVideoInfoTag()->m_iDbId, mediaType);
  }
  
    // we need to clear any cached version of this tag's listing
  items.SetPath(item->GetPath());
  items.RemoveDiscCache(g_windowManager.GetActiveWindow());
  return true;
}

bool ContextButtonTagRemoveItems::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  CVideoDbUrl videoUrl;
  return (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser) &&
  item && !item->IsParentFolder() &&
  container.GetContent() == "tags" && item->m_bIsFolder &&
  videoUrl.FromString(item->GetPath());
}

bool ContextButtonTagRemoveItems::execute(const CFileItem *item, const CFileItemList& container) 
{  
  CVideoDbUrl videoUrl;
  if (!videoUrl.FromString(item->GetPath()))
    return false;
  
  std::string mediaType = videoUrl.GetItemType();
  mediaType = mediaType.substr(0, mediaType.length() - 1);
  
  CFileItemList items;
  CStdString localizedType = CGUIWindowVideoNav::GetLocalizedType(mediaType);
  CStdString strLabel; strLabel.Format(g_localizeStrings.Get(20464), localizedType.c_str());
  if (!GetItemsForTag(strLabel, mediaType, items, item->GetVideoInfoTag()->m_iDbId, false))
    return true;
  
  CVideoDatabase videodb;
  if (!videodb.Open())
    return true;
  
  for (int index = 0; index < items.Size(); index++)
  {
    if (!items[index]->HasVideoInfoTag() || items[index]->GetVideoInfoTag()->m_iDbId <= 0)
      continue;
    
    videodb.RemoveTagFromItem(items[index]->GetVideoInfoTag()->m_iDbId, item->GetVideoInfoTag()->m_iDbId, mediaType);
  }
  
    // we need to clear any cached version of this tag's listing
  items.SetPath(item->GetPath());
  items.RemoveDiscCache(GetID());
  return true;  
}

bool ContextButtonDelete::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  XFILE::CVideoDatabaseDirectory dir;
  XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE node = dir.GetDirectoryChildType(container.GetPath());
  return (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser) &&
  item && !item->IsParentFolder() &&
  ((StringUtils::StartsWith(item->GetPath(), "videodb://movies/sets/") && item->GetPath().size() > 22 && item->m_bIsFolder) ||
   (item->IsVideoDb() && item->HasVideoInfoTag() && (!item->m_bIsFolder || node == XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE_TITLE_TVSHOWS)));
}

bool ContextButtonDelete::execute(const CFileItem *item, const CFileItemList& container) 
{  
  OnDeleteItem(itemNumber);
  return true;
}

bool ContextButtonSetArtistThumb::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  XFILE::CVideoDatabaseDirectory dir;
  XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE node = dir.GetDirectoryChildType(container.GetPath());
  return item && !item->IsParentFolder() &&
  (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser) &&
  (node == XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE_ACTOR && !dir.IsAllItem(item->GetPath()) && item->m_bIsFolder) &&
  (StringUtils::StartsWith(container.GetPath(), "videodb://musicvideos"));
}

bool ContextButtonSetArtistThumb::execute(const CFileItem *item, const CFileItemList& container) 
{  
  
}

bool ContextButtonSetActortThumb::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  XFILE::CVideoDatabaseDirectory dir;
  XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE node = dir.GetDirectoryChildType(container.GetPath());
  return item && !item->IsParentFolder() &&
  (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser) &&
  (node == XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE_ACTOR && !dir.IsAllItem(item->GetPath()) && item->m_bIsFolder) &&
  !(StringUtils::StartsWith(container.GetPath(), "videodb://musicvideos"));
}

bool ContextButtonSetActortThumb::execute(const CFileItem *item, const CFileItemList& container) 
{  
  
}

bool ContextButtonUnlinkBookmark::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  XFILE::CVideoDatabaseDirectory dir;
  XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE node = dir.GetDirectoryChildType(container.GetPath());
  CVideoDatabase database;
  database.Open();
  ADDON::ScraperPtr info = database.GetScraperForPath(item->GetPath());
  database.Close();
  return item && !item->IsParentFolder() &&
  (CProfilesManager::Get().GetCurrentProfile().canWriteDatabases() || g_passwordManager.bMasterUser) &&
  (item->IsVideoDb() && item->HasVideoInfoTag()) &&
  (!item->m_bIsFolder || node == XFILE::VIDEODATABASEDIRECTORY::NODE_TYPE_TITLE_TVSHOWS) &&
  info && info->Content() == CONTENT_TVSHOWS &&
  item->GetVideoInfoTag()->m_iBookmarkId != -1 &&
  item->GetVideoInfoTag()->m_iBookmarkId != 0;
}

bool ContextButtonUnlinkBookmark::execute(const CFileItem *item, const CFileItemList& container) 
{  
  CVideoDatabase database;
  database.Open();
  database.DeleteBookMarkForEpisode(*item->GetVideoInfoTag());
  database.Close();
  CUtil::DeleteVideoDatabaseDirectoryCache();
  
  CGUIMessage msg(GUI_MSG_UPDATE, g_windowManager.GetActiveWindow(), 0);
  g_windowManager.SendMessage(msg);
  
  return true;
}

bool ContextButtonFileDelete::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  bool inPlaylists = container.GetPath().Equals(CUtil::VideoPlaylistsLocation()) ||
  container.GetPath().Equals("special://videoplaylists/");
  return item && !item->IsParentFolder() && !container.IsVideoDb() && !container.IsVirtualDirectoryRoot() &&
  (g_guiSettings.GetBool("filelists.allowfiledeletion") &&
   CUtil::SupportsWriteFileOperations(item->GetPath())) || (inPlaylists && !URIUtils::GetFileName(item->GetPath()).Equals("PartyMode-Video.xsp") &&
                                                            (item->IsPlayList() || item->IsSmartPlayList()));
}

bool ContextButtonFileDelete::execute(const CFileItem *item, const CFileItemList& container) 
{  
  OnDeleteItem(itemNumber);
  return true; 
}

bool ContextButtonFileRename::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  bool inPlaylists = container.GetPath().Equals(CUtil::VideoPlaylistsLocation()) ||
  container.GetPath().Equals("special://videoplaylists/");
  return item && !item->IsParentFolder() && !container.IsVideoDb() && !container.IsVirtualDirectoryRoot() &&
  (g_guiSettings.GetBool("filelists.allowfiledeletion") &&
   CUtil::SupportsWriteFileOperations(item->GetPath())) || (inPlaylists && !URIUtils::GetFileName(item->GetPath()).Equals("PartyMode-Video.xsp") &&
                                                            (item->IsPlayList() || item->IsSmartPlayList()));
}

bool ContextButtonFileRename::execute(const CFileItem *item, const CFileItemList& container) 
{  
  OnRenameItem(itemNumber);
  return true;
}

bool ContextButtonSetContent::isVisible(const CFileItem *item, const CFileItemList& container) const
{ 
  return item && !item->IsParentFolder() &&
  !container.IsVideoDb() && !container.IsVirtualDirectoryRoot() &&
  item->m_bIsFolder && !item->IsPlayList() && !item->IsSmartPlayList() && 
  !item->IsLiveTV() && !item->IsPlugin() && !item->IsAddonsPath() && !URIUtils::IsUPnP(item->GetPath()) &&
  !g_application.IsVideoScanning();
}

bool ContextButtonSetContent::execute(const CFileItem *item, const CFileItemList& container) 
{  
  OnAssignContent(item->HasVideoInfoTag() && !item->GetVideoInfoTag()->m_strPath.IsEmpty() ? item->GetVideoInfoTag()->m_strPath : item->GetPath());
  return true;
}

bool ContextButtonPluginSettings::isVisible(const CFileItem *item, const CFileItemList& container) const
{
  return (item->IsPlugin() || item->IsScript() || container.IsPlugin());
}

bool ContextButtonPluginSettings::execute(const CFileItem *item, const CFileItemList& container) 
{
    // CONTEXT_BUTTON_PLUGIN_SETTINGS can be called for plugin item
    // or script item; or for the plugin directory current listing.
  bool isPluginOrScriptItem = (item && (item->IsPlugin() || item->IsScript()));
  CURL plugin(isPluginOrScriptItem ? item->GetPath() : container.GetPath());
  ADDON::AddonPtr addon;
  if (ADDON::CAddonMgr::Get().GetAddon(plugin.GetHostName(), addon))
  {
    if (CGUIDialogAddonSettings::ShowAndGetInput(addon))
    {
      CGUIMessage msg(GUI_MSG_UPDATE, g_windowManager.GetActiveWindow(), 0);
      g_windowManager.SendMessage(msg);
    }
  }
  return true;
}









