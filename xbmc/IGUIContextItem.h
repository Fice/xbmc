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

class IGUIContextItem;
typedef boost::shared_ptr<IGUIContextItem> ContextItemPtr;

class IGUIContextItem : public boost::enable_shared_from_this<IGUIContextItem>
{
public:
  virtual ContextItemPtr GetByID(unsigned int id) { return (getMsgID()==id) ? shared_from_this() : ContextItemPtr(); }
  virtual unsigned int getMsgID() const =0;
  virtual CStdString getLabel() const =0;
  virtual void AddVisibleItems(const CGUIListItem* listItem, const CGUIBaseContainer& container, std::list<ContextItemPtr>& list);
  virtual bool isVisible(const CGUIListItem *item, const CGUIBaseContainer& container) const=0;
  virtual ~IGUIContextItem() {}
  bool operator()(const CGUIListItem *item) {
    if(!isVisible(item))
      return false;
    return execute();
  }
  
  struct ContextVisiblePredicate : std::binary_function<ContextItemPtr, const CGUIListItem* const, bool>
  {
    bool operator()(const ContextItemPtr& item, const CGUIListItem * const listItem) const;
  };
  
  /*
  struct IDFinder : std::binary_function<ContextItemPtr, unsigned int, bool>
  {
    bool operator()(const ContextItemPtr& item, unsigned int id) const;
  };*/
  
  
protected:
  virtual bool execute()=0;
  
};
typedef boost::shared_ptr<IGUIContextItem> ContextItemPtr;

class CGUIBaseContextItem : public IGUIContextItem
{
public:
  CGUIBaseContextItem(unsigned int msgId, int labelId) { m_msgId = msgId; m_labelId = labelId; }
  virtual unsigned int getMsgID() const { return m_msgId; }
  virtual CStdString getLabel() const { return g_localizeStrings.Get(m_labelId); }
protected:
  unsigned int m_msgId;
  int m_labelId;
private:
  CGUIBaseContextItem() {}
};

/* \brief This class represents a Context Menu with sub items */
class CGUIContextFolder : public CGUIBaseContextItem
{
  CGUIContextFolder(unsigned int msgId, int labelId) : CGUIBaseContextItem(msgId, labelId) {}
  
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
  
  virtual bool isVisible(const CGUIListItem *item) const
  {
    constContextIter it = find_if(m_subEntries.begin(), 
                                  m_subEntries.end(),
                                  std::bind2nd(IGUIContextItem::ContextVisiblePredicate(), item)
                                 );
    return it!=m_subEntries.end();
  }
  virtual void AddVisibleItems(const CGUIListItem* item, std::list<ContextItemPtr>& list)
  {
    contextIter it = find_if(m_subEntries.begin(), 
                             m_subEntries.end(),
                             std::bind2nd(IGUIContextItem::ContextVisiblePredicate(), item)
                            );
    if(it==m_subEntries.end())
      return;
    
    constContextIter it2 = find_if(it, 
                                  m_subEntries.end(),
                                  std::bind2nd(IGUIContextItem::ContextVisiblePredicate(), item)
                                  );
    if(it2!=m_subEntries.end())
    { //we have at least two visible sub-items -> Show the folder
      list.push_back(shared_from_this());
    }
    else 
    { //we only have one subitem visible... show that one directly!
      (*it)->AddVisibleItems(item, list);
    }
  }
  
  typedef std::vector<ContextItemPtr>::iterator contextIter;
  typedef std::vector<ContextItemPtr>::const_iterator constContextIter;
protected:
  virtual bool execute()=0; //TODO:
  
  std::vector<ContextItemPtr> m_subEntries;
  
  
};

  //Actual implementations of the Context items... should propably moved out into another file
class ContextItemNowPlaying : public CGUIBaseContextItem
{
public:
  ContextItemNowPlaying();
  
  virtual bool isVisible(const CGUIListItem *item) const;
  virtual bool execute();
};











