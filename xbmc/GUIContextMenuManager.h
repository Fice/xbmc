#pragma once
/*
 *      Copyright (C) 2013-2014 Team XBMC
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

#include <boost/shared_ptr.hpp>
#include <list>
#include <boost/noncopyable.hpp>
#include "dialogs/GUIDialogContextMenu.h"
#include "addons/ContextItemAddon.h"

#define CONTEXT_MENU_GROUP_MANAGE "xbmc.manage"

class CContextButtons;

class CContextMenuManager
{
public:
  static CContextMenuManager& Get();

  /*! Get a context menu item by its assigned message id.
   \param unsigned int - msg id of the context item
   \return the addon or NULL
   */
  ADDON::ContextAddonPtr GetContextItemByID(const unsigned int ID);

  /*!
   Adds all registered context item to the list
   \param context - the current window id
   \param item - the currently selected item
   \param out visible - appends all visible menu items to this list
   */
  void AppendVisibleContextItems(const CFileItemPtr item, CContextButtons& list, const std::string& parent = "");

  /*!
   \brief Adds a context item to this manager.
   NOTE: if a context item has changed, just register it again and it will overwrite the old one
   NOTE: only 'enabled' context addons should be added
   \param the context item to add
   \sa UnegisterContextItem
   */
  void Register(ADDON::ContextAddonPtr cm);

  /*!
   \brief Removes a context addon from this manager.
   \param the context item to remove
   \sa RegisterContextItem
   */
  bool Unregister(ADDON::ContextAddonPtr cm);

private:
  CContextMenuManager();
  CContextMenuManager(const CContextMenuManager&);
  CContextMenuManager const& operator=(CContextMenuManager const&);
  virtual ~CContextMenuManager() {};
  void Init();

  std::map<unsigned int, ADDON::ContextAddonPtr> m_contextAddons;
  unsigned int m_iCurrentContextId;
};
