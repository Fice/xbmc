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

#include "Addon.h"
#include <list>


class CFileItem;
typedef boost::shared_ptr<CFileItem> CFileItemPtr;

namespace INFO
{
  class InfoBool;
  typedef boost::shared_ptr<InfoBool> InfoPtr;
}

namespace ADDON
{
  class IContextItem;
  typedef boost::shared_ptr<IContextItem> ContextAddonPtr;

  class IContextItem : public CAddon
  {
  public:
    IContextItem(const cp_extension_t *ext);
    IContextItem(const AddonProps &props);
    virtual ~IContextItem();

    const std::string& GetLabel() const { return m_label; }
    /*! \brief returns the parent category of this context item
        \return empty string if at root level or MANAGE_CATEGORY_NAME when it should be in the 'manage' submenu
             or the id of a ContextCategoryItem
     */
    const std::string& GetParent() const { return m_parent; }
    /*! \brief returns true if this item should be visible.
     NOTE:  defaults to true, if no visibility expression was set.
     \return true if this item should be visible
     */
    virtual bool IsVisible(const CFileItemPtr item) const = 0;

    virtual bool OnPreInstall();
    virtual void OnPostInstall(bool restart, bool update);
    virtual void OnPreUnInstall();
    virtual void OnDisabled();
    virtual void OnEnabled();
  protected:
    std::string m_label;
    std::string m_parent;
  };

  class CContextItemAddon : public IContextItem
  {
  public:
    CContextItemAddon(const cp_extension_t *ext);
    CContextItemAddon(const AddonProps &props);
    virtual ~CContextItemAddon();

    bool IsVisible(const CFileItemPtr item) const;
  protected:
    INFO::InfoPtr m_VisibleId;
  };

}
