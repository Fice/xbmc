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

#include "ContextItemAddon.h"
#include "../guilib/GUIListItem.h"
#include "GUIContextMenuManager.h"

class CFileItem;
typedef boost::shared_ptr<CFileItem> CFileItemPtr;

namespace ADDON
{

    //TODO: handle other addon types than python once that's possible!
class CContextCategoryAddon : public CContextItemAddon, public ContextMenuManager
{
  public:
    CContextCategoryAddon(const cp_extension_t *ext);
    CContextCategoryAddon(const AddonProps &props);
    virtual ~CContextCategoryAddon();

    virtual bool IsVisible(const CFileItemPtr item) const;
    virtual void AddIfVisible(const CFileItemPtr item, CContextButtons &visible);
    virtual ADDON::ContextAddonPtr GetChildWithID(const std::string& strID);
    virtual bool Execute(const CFileItemPtr itemPath);
    virtual bool IsLogicalType(TYPE type) const { return IsType(type) || type == ADDON_CONTEXT_ITEM; }
};
}
