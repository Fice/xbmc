/*!
\file GUIListContainer.h
\brief
*/

#pragma once

/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "GUIBaseContainer.h"

struct CGUIListDragHandler
{
  CGUIListDragHandler(bool internal, 
                      bool reorderable, 
                      bool dropable,
                      boost::shared_ptr<CGUIControl> dragHint, 
                      CGUIBaseContainer *container);
  ~CGUIListDragHandler();
      //Destructor
      //DragHint
      //Hide Drag Hint
      //Always: Stop Scrolling
    
      //Eigenschaften:
      //-intern/extern
      //-reorderable/nonreorderable
      //-DragHint/NotDragHint
    
  void Process(unsigned int currentTime, CDirtyRegionList &dirtyregions);
  void Render();
    
  void DragStart(const CPoint& point);
  EVENT_RESULT DragMove(const CPoint &point);
  /**
   Called when the user no longer hovers this item during drag&drop
   NOTE: this function will not be called, when the user droped on us!
   **/
  void DraggedAway();
    
  /**
   This functio will be called, when the user dropped the fileitem on us
   \returns EVENT_RESULT_HANDLED if it actually performed an action. EVENT_RESULT_UNHANDLED otherwise
   **/
  EVENT_RESULT OnDrop();
  
    //States that do not change during drag&drop
  const bool m_bInternal; //true: it is in-list-drag&drop, false: the users tries to drag an item from another list onto our list
  const bool m_bReorderable; 
  const bool m_bDropable;
protected:
  const CPoint m_dragHintOffset;
  
  void ClearDragHint();
  void ShowDragHint(const CPoint& insertPoint);
  
  const boost::shared_ptr<CGUIControl> m_dragHint;
  CGUIBaseContainer* const m_container;
  short m_draggedScrollDirection; 
  CPoint m_dragHintPosition; 
  int m_draggedNewPosition; 
  int m_draggedOrigPosition; 
};