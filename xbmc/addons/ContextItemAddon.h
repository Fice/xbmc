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
#include "Addon.h"
#include "../IGUIContextItem.h"
#include "../guilib/GUIListItem.h"

namespace ADDON
{

  class CContextItemAddon : public CAddon, public IGUIContextItem
  {
  public:


    CContextItemAddon(const cp_extension_t *ext);
    CContextItemAddon(const AddonProps &props);
    virtual ~CContextItemAddon();
    
    virtual unsigned int getMsgID() const { return m_id; }
    virtual CStdString getLabel();
    virtual bool isVisible(const CFileItemPtr item) const;
    
    
  protected:
    virtual bool execute(const CFileItemPtr itemPath);
    unsigned int m_id;
    unsigned int m_VisibleId;
    CStdString m_label;
    bool m_bTrueOnNullId;
  };
  typedef boost::shared_ptr<CContextItemAddon> ContextAddonPtr;
}
