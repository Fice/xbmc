/*!
\file GUIContextMenuManager.h
\brief
*/
#pragma once

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

#include "boost/shared_ptr.hpp"
#include "utils/StringUtils.h"
#include "guilib/GUIListItem.h"
#include "guilib/GUIBaseContainer.h"
#include "guilib/LocalizeStrings.h"
#include <functional>
#include <boost/enable_shared_from_this.hpp>
#include <list>



enum CONTEXT_BUTTON { CONTEXT_BUTTON_CANCELLED = 0,
  CONTEXT_BUTTON_LAUNCH,
  CONTEXT_BUTTON_RENAME,
  CONTEXT_BUTTON_DELETE,
  CONTEXT_BUTTON_COPY,
  CONTEXT_BUTTON_MOVE,
  CONTEXT_BUTTON_ADD_FAVOURITE,
  CONTEXT_BUTTON_SETTINGS,
  CONTEXT_BUTTON_GOTO_ROOT,
  CONTEXT_BUTTON_PLAY_DISC,
  CONTEXT_BUTTON_RESUME_DISC,
  CONTEXT_BUTTON_RIP_CD,
  CONTEXT_BUTTON_CANCEL_RIP_CD,
  CONTEXT_BUTTON_RIP_TRACK,
  CONTEXT_BUTTON_EJECT_DISC,
  CONTEXT_BUTTON_EJECT_DRIVE,
  CONTEXT_BUTTON_ADD_SOURCE,
  CONTEXT_BUTTON_EDIT_SOURCE,
  CONTEXT_BUTTON_REMOVE_SOURCE,
  CONTEXT_BUTTON_SET_DEFAULT,
  CONTEXT_BUTTON_CLEAR_DEFAULT,
  CONTEXT_BUTTON_SET_THUMB,
  CONTEXT_BUTTON_ADD_LOCK,
  CONTEXT_BUTTON_REMOVE_LOCK,
  CONTEXT_BUTTON_CHANGE_LOCK,
  CONTEXT_BUTTON_RESET_LOCK,
  CONTEXT_BUTTON_REACTIVATE_LOCK,
  CONTEXT_BUTTON_VIEW_SLIDESHOW,
  CONTEXT_BUTTON_RECURSIVE_SLIDESHOW,
  CONTEXT_BUTTON_REFRESH_THUMBS,
  CONTEXT_BUTTON_SWITCH_MEDIA,
  CONTEXT_BUTTON_MOVE_ITEM,
  CONTEXT_BUTTON_MOVE_HERE,
  CONTEXT_BUTTON_CANCEL_MOVE,
  CONTEXT_BUTTON_MOVE_ITEM_UP,
  CONTEXT_BUTTON_MOVE_ITEM_DOWN,
  CONTEXT_BUTTON_SAVE,
  CONTEXT_BUTTON_LOAD,
  CONTEXT_BUTTON_CLEAR,
  CONTEXT_BUTTON_QUEUE_ITEM,
  CONTEXT_BUTTON_PLAY_ITEM,
  CONTEXT_BUTTON_PLAY_WITH,
  CONTEXT_BUTTON_PLAY_PARTYMODE,
  CONTEXT_BUTTON_PLAY_PART,
  CONTEXT_BUTTON_RESUME_ITEM,
  CONTEXT_BUTTON_RESTART_ITEM,
  CONTEXT_BUTTON_EDIT,
  CONTEXT_BUTTON_EDIT_SMART_PLAYLIST,
  CONTEXT_BUTTON_INFO,
  CONTEXT_BUTTON_INFO_ALL,
  CONTEXT_BUTTON_CDDB,
  CONTEXT_BUTTON_UPDATE_LIBRARY,
  CONTEXT_BUTTON_UPDATE_TVSHOW,
  CONTEXT_BUTTON_SCAN,
  CONTEXT_BUTTON_STOP_SCANNING,
  CONTEXT_BUTTON_SET_ARTIST_THUMB,
  CONTEXT_BUTTON_SET_SEASON_ART,
  CONTEXT_BUTTON_NOW_PLAYING,
  CONTEXT_BUTTON_CANCEL_PARTYMODE,
  CONTEXT_BUTTON_MARK_WATCHED,
  CONTEXT_BUTTON_MARK_UNWATCHED,
  CONTEXT_BUTTON_SET_CONTENT,
  CONTEXT_BUTTON_ADD_TO_LIBRARY,
  CONTEXT_BUTTON_SONG_INFO,
  CONTEXT_BUTTON_EDIT_PARTYMODE,
  CONTEXT_BUTTON_LINK_MOVIE,
  CONTEXT_BUTTON_UNLINK_MOVIE,
  CONTEXT_BUTTON_GO_TO_ARTIST,
  CONTEXT_BUTTON_GO_TO_ALBUM,
  CONTEXT_BUTTON_PLAY_OTHER,
  CONTEXT_BUTTON_SET_ACTOR_THUMB,
  CONTEXT_BUTTON_UNLINK_BOOKMARK,
  CONTEXT_BUTTON_PLUGIN_SETTINGS,
  CONTEXT_BUTTON_SCRIPT_SETTINGS,
  CONTEXT_BUTTON_LASTFM_UNLOVE_ITEM,
  CONTEXT_BUTTON_LASTFM_UNBAN_ITEM,
  CONTEXT_BUTTON_HIDE,
  CONTEXT_BUTTON_SHOW_HIDDEN,
  CONTEXT_BUTTON_ADD,
  CONTEXT_BUTTON_ACTIVATE,
  CONTEXT_BUTTON_START_RECORD,
  CONTEXT_BUTTON_STOP_RECORD,
  CONTEXT_BUTTON_GROUP_MANAGER,
  CONTEXT_BUTTON_CHANNEL_MANAGER,
  CONTEXT_BUTTON_FILTER,
  CONTEXT_BUTTON_SET_MOVIESET_ART,
  CONTEXT_BUTTON_BEGIN,
  CONTEXT_BUTTON_END,
  CONTEXT_BUTTON_FIND,
  CONTEXT_BUTTON_DELETE_PLUGIN,
  CONTEXT_BUTTON_SORTASC,
  CONTEXT_BUTTON_SORTBY,
  CONTEXT_BUTTON_SORTBY_CHANNEL,
  CONTEXT_BUTTON_SORTBY_NAME,
  CONTEXT_BUTTON_SORTBY_DATE,
  CONTEXT_BUTTON_MENU_HOOKS,
  CONTEXT_BUTTON_PLAY_AND_QUEUE,
  CONTEXT_BUTTON_PLAY_ONLY_THIS,
  CONTEXT_BUTTON_UPDATE_EPG,
  CONTEXT_BUTTON_RECORD_ITEM,
  CONTEXT_BUTTON_TAGS_ADD_ITEMS,
  CONTEXT_BUTTON_TAGS_REMOVE_ITEMS,
  CONTEXT_BUTTON_SET_MOVIESET,
  CONTEXT_BUTTON_MOVIESET_ADD_REMOVE_ITEMS,
  CONTEXT_BUTTON_FILE_DELETE,
  CONTEXT_BUTTON_FILE_RENAME,
  CONTEXT_BUTTON_FIRST_CONTEXT_PLUGIN //NOTE: this has to be the last in this enum,
                                      //because this one, and the ones higher will be used by context plugins
  
};


class IGUIContextItem;
typedef boost::shared_ptr<IGUIContextItem> ContextItemPtr;

class IGUIContextItem : public boost::enable_shared_from_this<IGUIContextItem>
{
public:
  virtual ContextItemPtr GetByID(unsigned int id) { return (getMsgID()==id) ? shared_from_this() : ContextItemPtr(); }
  virtual unsigned int getMsgID() const =0;
  virtual CStdString getLabel(const CFileItem *item, const CFileItemList& container) const =0;
  virtual void AddVisibleItems(const CFileItem* listItem, const CFileItemList& container, std::list<ContextItemPtr>& list);
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const=0;
  virtual ~IGUIContextItem() {}
  bool operator()(const CFileItem *item, CFileItemList& container) {
    if(!isVisible(item, container))
      return false;
    return execute(item, container);
  }
  
  struct ContextVisiblePredicate : std::unary_function<ContextItemPtr, bool>
  {
    ContextVisiblePredicate(const CFileItem * const listItem, const CFileItemList& container)
      : m_listItem(listItem), m_container(container) 
    {}
    bool operator()(const ContextItemPtr& item) const;
  protected:
    const CFileItem * const m_listItem;
    const CFileItemList& m_container;
  };
protected:
  virtual bool execute(const CFileItem *item, const CFileItemList& container)=0;
  
};
typedef boost::shared_ptr<IGUIContextItem> ContextItemPtr;

class CGUIBaseContextItem : public IGUIContextItem
{
public:
  CGUIBaseContextItem(CONTEXT_BUTTON msgId, int labelId) { m_msgId = msgId; m_labelId = labelId; }
  virtual unsigned int getMsgID() const { return m_msgId; }
  virtual CStdString getLabel(const CFileItem *item, const CFileItemList& container) const { return g_localizeStrings.Get(m_labelId); }
protected:
  CONTEXT_BUTTON m_msgId;
  int m_labelId;
private:
  CGUIBaseContextItem() {}
};

/* \brief This class represents a Context Menu with sub items */
class CGUIContextFolder : public CGUIBaseContextItem
{
  CGUIContextFolder(CONTEXT_BUTTON msgId, int labelId) : CGUIBaseContextItem(msgId, labelId) {}
  
  virtual ContextItemPtr GetByID(unsigned int id) {
    if(getMsgID()==id)
      return shared_from_this();
      //todo
    for(contextIter it = m_subEntries.begin(); it!=m_subEntries.end(); ++it)
    {
      ContextItemPtr ptr = (*it)->GetByID(id);
      if(ptr)
        return ptr;
    }
    return ContextItemPtr();
  }
  
  virtual bool isVisible(const CFileItem *item,  const CFileItemList& container) const
  {
    constContextIter it = find_if(m_subEntries.begin(), 
                                  m_subEntries.end(),
                                  IGUIContextItem::ContextVisiblePredicate(item, container)
                                 );
    return it!=m_subEntries.end();
  }
  virtual void AddVisibleItems(const CFileItem* item, const CFileItemList& container, std::list<ContextItemPtr>& list)
  {
    contextIter it = find_if(m_subEntries.begin(), 
                             m_subEntries.end(),
                             IGUIContextItem::ContextVisiblePredicate(item, container)
                            );
    if(it==m_subEntries.end())
      return;
    
    constContextIter it2 = find_if(it, 
                                  m_subEntries.end(),
                                  IGUIContextItem::ContextVisiblePredicate(item, container)
                                  );
    if(it2!=m_subEntries.end())
    { //we have at least two visible sub-items -> Show the folder
      list.push_back(shared_from_this());
    }
    else 
    { //we only have one subitem visible... show that one directly!
      (*it)->AddVisibleItems(item, container, list);
    }
  }
  
  typedef std::vector<ContextItemPtr>::iterator contextIter;
  typedef std::vector<ContextItemPtr>::const_iterator constContextIter;
protected:
  virtual bool execute()=0; //TODO:
  
  std::vector<ContextItemPtr> m_subEntries;
  
  
};

  //Actual implementations of the Context items... should propably moved out into another file
struct ContextItemNowPlaying : public CGUIBaseContextItem
{
public:
  ContextItemNowPlaying();
  
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const;
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};



struct ContextButtonVideoStopScanning : public CGUIBaseContextItem
{
  ContextButtonVideoStopScanning() : CGUIBaseContextItem(CONTEXT_BUTTON_STOP_SCANNING, 13353) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const;
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonVideoUpdateLibrary : public CGUIBaseContextItem
{
  ContextButtonVideoUpdateLibrary() : CGUIBaseContextItem(CONTEXT_BUTTON_UPDATE_LIBRARY, 653) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const;
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonVideoScanLibrary : public CGUIBaseContextItem
{
  ContextButtonVideoScanLibrary() : CGUIBaseContextItem(CONTEXT_BUTTON_SCAN, 13349) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonVideoSetContent : public IGUIContextItem
{
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const;
  virtual CStdString getLabel(const CFileItem *item, const CFileItemList& container) const; 
  virtual unsigned int getMsgID() const { return CONTEXT_BUTTON_SET_CONTENT; }
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonVideoGoArtist : public CGUIBaseContextItem
{
  ContextButtonVideoGoArtist() : CGUIBaseContextItem(CONTEXT_BUTTON_GO_TO_ARTIST, 20396) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonVideoGoAlbum : public CGUIBaseContextItem
{
  
  ContextButtonVideoGoAlbum() : CGUIBaseContextItem(CONTEXT_BUTTON_GO_TO_ALBUM, 20397) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonVideoPlayOther : public CGUIBaseContextItem
{
  ContextButtonVideoPlayOther() : CGUIBaseContextItem(CONTEXT_BUTTON_PLAY_OTHER, 20398) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonVideoInfo : public IGUIContextItem
{
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const;
  virtual CStdString getLabel(const CFileItem *item, const CFileItemList& container) const; 
  virtual unsigned int getMsgID() const { return CONTEXT_BUTTON_INFO; }
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonUpdateTVShow : public CGUIBaseContextItem
{
  ContextButtonUpdateTVShow() : CGUIBaseContextItem(CONTEXT_BUTTON_UPDATE_TVSHOW, 13349) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonMarkUnwatched : public CGUIBaseContextItem
{
  ContextButtonMarkUnwatched() : CGUIBaseContextItem(CONTEXT_BUTTON_MARK_UNWATCHED, 16104) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonMarkWatched : public CGUIBaseContextItem
{
  ContextButtonMarkWatched() : CGUIBaseContextItem(CONTEXT_BUTTON_MARK_WATCHED, 16103) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};


struct ContextButtonEditTitle : public CGUIBaseContextItem
{
  ContextButtonEditTitle() : CGUIBaseContextItem(CONTEXT_BUTTON_EDIT, 16105) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonUnlinkMovie : public CGUIBaseContextItem
{
  ContextButtonUnlinkMovie() : CGUIBaseContextItem(CONTEXT_BUTTON_UNLINK_MOVIE, 20385) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonLinkMovie : public CGUIBaseContextItem
{
  ContextButtonLinkMovie() : CGUIBaseContextItem(CONTEXT_BUTTON_LINK_MOVIE, 20384) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};


struct ContextButtonSetMovieset : public CGUIBaseContextItem
{
  
  ContextButtonSetMovieset() : CGUIBaseContextItem(CONTEXT_BUTTON_SET_MOVIESET, 20465) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonSetSeasonArt : public CGUIBaseContextItem
{
  ContextButtonSetSeasonArt() : CGUIBaseContextItem(CONTEXT_BUTTON_SET_SEASON_ART, 13511) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonSetMoviesetArt : public CGUIBaseContextItem
{
  ContextButtonSetMoviesetArt() : CGUIBaseContextItem(CONTEXT_BUTTON_SET_MOVIESET_ART, 13511) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonMoviesetAddRemove : public CGUIBaseContextItem
{
  ContextButtonMoviesetAddRemove() : CGUIBaseContextItem(CONTEXT_BUTTON_MOVIESET_ADD_REMOVE_ITEMS, 20465) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonTagAddItems : public IGUIContextItem
{
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
  virtual CStdString getLabel(const CFileItem *item, const CFileItemList& container) const; //TODO:
  virtual unsigned int getMsgID() const { return CONTEXT_BUTTON_TAGS_ADD_ITEMS; }
};

struct ContextButtonTagRemoveItems : public IGUIContextItem
{
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
  virtual CStdString getLabel(const CFileItem *item, const CFileItemList& container) const; //TODO:
  virtual unsigned int getMsgID() const { return CONTEXT_BUTTON_TAGS_REMOVE_ITEMS; }
};

struct ContextButtonDelete : public CGUIBaseContextItem
{
  ContextButtonDelete() : CGUIBaseContextItem(CONTEXT_BUTTON_DELETE, 646) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonSetArtistThumb : public CGUIBaseContextItem
{
  ContextButtonSetArtistThumb() : CGUIBaseContextItem(CONTEXT_BUTTON_SET_ARTIST_THUMB, 13359) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonSetActortThumb : public CGUIBaseContextItem
{
  ContextButtonSetActortThumb() : CGUIBaseContextItem(CONTEXT_BUTTON_SET_ACTOR_THUMB, 20403) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonUnlinkBookmark : public CGUIBaseContextItem
{
  ContextButtonUnlinkBookmark() : CGUIBaseContextItem(CONTEXT_BUTTON_UNLINK_BOOKMARK, 20405) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonFileDelete : public CGUIBaseContextItem
{
  ContextButtonFileDelete() : CGUIBaseContextItem(CONTEXT_BUTTON_FILE_DELETE, 117) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonFileRename : public CGUIBaseContextItem
{
  ContextButtonFileRename() : CGUIBaseContextItem(CONTEXT_BUTTON_FILE_RENAME, 118) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonSetContent : public CGUIBaseContextItem
{
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};

struct ContextButtonPluginSettings : public CGUIBaseContextItem
{
  ContextButtonPluginSettings() : CGUIBaseContextItem(CONTEXT_BUTTON_PLUGIN_SETTINGS, 1045) {}
  virtual bool isVisible(const CFileItem *item, const CFileItemList& container) const; 
  virtual bool execute(const CFileItem *item, const CFileItemList& container);
};