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

  //I assume that CMRename, CMRemove, CMSetThumb, CMSetFanart etc. are already implemented above



class CMManage : public CMNestedContextItem
{
  
  CManage() {
    sub_context_items.push_back(ContextItemPtr(new CMRename()));    
    sub_context_items.push_back(ContextItemPtr(new CMRemove()));
    sub_context_items.push_back(ContextItemPtr(new CMSetThumb()));
    sub_context_items.push_back(ContextItemPtr(new CMSetFanart()));
  }
  
  virtual unsigned int getMsgID() const { return MESSAGE_ID_FOR_MANAGE; }
  virtual CStdString getLabel() { return "Manage"; }
  
}