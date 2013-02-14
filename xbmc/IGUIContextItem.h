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
#include "FileItem.h"
#include <functional>

class IGUIContextItem;
typedef boost::shared_ptr<IGUIContextItem> ContextItemPtr;

class IGUIContextItem
{
public:
  virtual unsigned int getMsgID() const =0;
  bool isMsgID(const unsigned int Id) {
    return getMsgID()==Id;
  }
  virtual CStdString getLabel() =0;
  virtual bool isVisible(const CFileItemPtr item) const=0;
  virtual ~IGUIContextItem() {}
  bool operator()(const CFileItemPtr item) {
    if(!isVisible(item))
      return false;
    return execute(item);
  }
  
  struct ContextVisiblePredicate : std::binary_function<ContextItemPtr, const CFileItemPtr, bool>
  {
    bool operator()(const ContextItemPtr& item, const CFileItemPtr listItem) const;
  };
  
  struct IDFinder : std::binary_function<ContextItemPtr, unsigned int, bool>
  {
    bool operator()(const ContextItemPtr& item, unsigned int id) const;
  };
  
  
protected:
  virtual bool execute(const CFileItemPtr item)=0;
  
};

typedef boost::shared_ptr<IGUIContextItem> ContextItemPtr;


template<unsigned int ID, unsigned int LABEL, typename PREDICATE, typename FUNCTION>
class CoreContextItem : public IGUIContextItem
{
  virtual CStdString getLabel() { return g_localizeStrings.Get(LABEL); }
  virtual unsigned int getMsgID() const { return ID; }
  virtual bool isVisible(const CFileItemPtr item) const { return PREDICATE(item); }
protected:
  virtual bool execute(const CFileItemPtr item) { return FUNCTION(item); }
};





