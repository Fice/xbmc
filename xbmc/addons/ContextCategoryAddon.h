#pragma once
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
    
    virtual bool isVisible(const CFileItemPtr item) const;
    virtual void addIfVisible(const CFileItemPtr item, std::list<ContextAddonPtr> &visible);
    virtual ADDON::ContextAddonPtr GetChildWithID(const std::string& strID);
  protected:
    virtual bool execute(const CFileItemPtr itemPath);
  };
}
