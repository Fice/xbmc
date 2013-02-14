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
#include "IGUIContextItem.h"
#include "Favourites.h"

class CMAddRemoveFavorite : public IGUIContextItem
{
public:
  virtual CStdString getLabel(const CFileItemPtr item)
  {
    
    if (CFavourites::IsFavourite(item.get(), 0)
      return g_localizeStrings.Get(14077);     // Remove Favourite
    else
      return g_localizeStrings.Get(14076);     // Add To Favourites;
  }
  
  virtual unsigned int getMsgId() const { return CONTEXT_BUTTON_ADD_FAVOURITE; }
  
  
  virtual bool isVisible(const CFileItemPtr item) const
  {
    return !item->IsParentFolder() && !item->GetPath().Equals("add") && !item->GetPath().Equals("newplaylist://") &&
           !item->GetPath().Left(19).Equals("newsmartplaylist://") && !item->GetPath().Left(9).Equals("newtag://");
  }

protected:
  virtual bool execute(const CFileItemPtr item) { 
    CFavourites::AddOrRemove(item.get(), 0); 
    
    return true;
  }
  
};

