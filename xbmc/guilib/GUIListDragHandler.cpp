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
#include "GUIListDragHandler.h"
#include "GUIInfoManager.h"

CGUIListDragHandler::CGUIListDragHandler(bool internal, 
                                         bool reorderable,
                                         bool dropable, 
                                         boost::shared_ptr<CGUIControl> dragHint, 
                                         CGUIBaseContainer *container)
: m_bInternal(internal),
  m_bReorderable(reorderable),
  //if this is a in-list-dragging and we are not reorderable, 
  //then we shouldn't accept the item, no matter what everyone else says
  m_bDropable((internal && !reorderable) ? false : dropable), 
  m_dragHint(dragHint),
  m_container(container)
{
  ASSERT(m_container);
  
  m_draggedOrigPosition = m_draggedNewPosition = -1;
  m_draggedScrollDirection = 0;
}


CGUIListDragHandler::~CGUIListDragHandler()
{
  ClearDragHint();
}

void CGUIListDragHandler::Process(unsigned int currentTime, CDirtyRegionList &dirtyregions) //all public
{
    //Do wee need to scroll?
  if(m_draggedScrollDirection && !m_container->m_scroller.IsScrolling())
  {
    m_container->Scroll(m_draggedScrollDirection);
    if(m_container->m_scroller.IsScrolling())
      m_container->SetCursor(m_container->GetCursor()-m_draggedScrollDirection); //our dragged item should be kept focused
  }
  if(m_bDropable)
  {
    //If we have a drag handle, we need to process it
    if(m_dragHint && m_bDropable)
    {
      g_graphicsContext.SetOrigin(m_dragHintPosition.x1, m_dragHintPosition.x2);
      m_dragHint->Process(currentTime, dirtyregions);
      g_graphicsContext.RestoreOrigin();
    }
    dirtyregions.push_back(m_container->m_renderRegion); 
      //TODO: this can be done better
      //TODO: if hint: push region of old dragHint
      //TODO: if hint: push region of new hint
      //TODO: if !hint: 
  }
  
}


void CGUIListDragHandler::Render() //all public
{
    //if we have a draghint, we need to render it
  if(m_dragHint && m_bDropable) //TODO: only draw if it should be visible
  {
    CGUITexture::DrawQuad(m_dragHintPosition, 0x4c00ff00);
    g_graphicsContext.SetOrigin(m_dragHintPosition.x1, m_dragHintPosition.x2);
    m_dragHint->DoRender();
    g_graphicsContext.RestoreOrigin();
  }
}

void CGUIListDragHandler::DragStart(const CPoint& point) //m_items and m_focusedLayout is protected
{
  if(m_bInternal)
  {
    //Store the item that the user wants to drag for later use
    CGUIListItemPtr draggedItem;
    int selected = m_container->GetSelectedItem();
    if (selected >= 0 && selected < (int)m_container->m_items.size())
    {
      draggedItem = m_container->m_items[selected];
      m_draggedOrigPosition = m_draggedNewPosition = selected;
    }
      //Let the skinner have access to drag&drop info stuff
    g_infoManager.DraggingStart(draggedItem, m_container);

  }
  else //Dragging started earlier, but now we're finnaly hovered!
  {
    CGUIListItemPtr draggedItem = g_infoManager.GetDraggedFileItem();
    ASSERT(draggedItem);
    
    if(m_bReorderable)
    {
      CPoint insertPoint;
      m_draggedNewPosition = m_container->calculateDragInsertPosition(point, insertPoint);
      
      if(m_dragHint)
      { //set draghint position
        ShowDragHint(insertPoint);
      }
      else 
      { //insert the item at the correct position
        m_container->m_items.insert(m_container->m_items.begin()+m_draggedNewPosition, draggedItem);
      }
    }
    else {
        //add item to the end
      m_container->m_items.push_back(draggedItem);
        //now let the responsible entity sort the m_items vector
        //now find the position of our draggedItem
      m_draggedNewPosition = m_container->m_items.size()-1;
      
      if(m_dragHint)
      {
        m_container->m_items.erase(m_container->m_items.begin()+m_draggedNewPosition); //remove the item again... we only wanted to get the sorted position
        //TODO: get CPoint for dragged position
          //TODO:ShowDragHint(ShowDragHint)
      }
      
      
    }
    

  }
}

EVENT_RESULT CGUIListDragHandler::DragMove(const CPoint &point)
{
  CPoint insertPoint;
  int newPosition = m_container->calculateDragInsertPosition(point, insertPoint);
  
  if(m_bDropable)
  {
      //Let the skinner know, that we are currently the drop target
    g_infoManager.DragHover(m_container);
    
    if (newPosition<-1)
    { //The user is currently only hovering our border
      DraggedAway(); //This will make sure, we remove all visual hints etc...
      m_draggedNewPosition = -1;
    }
    else if(!(!m_bReorderable && !m_bInternal))
    {
      if (newPosition < m_draggedOrigPosition)
        newPosition++;
      
      
      if (m_dragHint) //we have a drag hint, so let's calculate it's position and  make it visible
      {
        m_draggedNewPosition = newPosition;
        if(m_bInternal)
        {
          if (m_draggedNewPosition != m_draggedOrigPosition) //only if the drop position is different then the item's original position
            ShowDragHint(insertPoint);
          else //Don't show drag hint, if user drags on current item position
            ClearDragHint();
        }
        else
        { //Always show drag hint when sth. gets draged onto this list
          ShowDragHint(insertPoint);
        }
      } 
      else //we don't have a drag hint, so lets reorder immediately
      {
        m_container->MoveItemInternally(m_draggedNewPosition, newPosition);
        m_container->SetCursor(newPosition - m_container->GetOffset());
        m_draggedNewPosition=newPosition;
      }      
    }
  }
 
  
    //Take care of scrolling (even if we're not dropable)
  if (newPosition == m_container->m_offset) //are we hovering the first element?
  { //then move up
    m_draggedScrollDirection = -1;
  }
  else if (newPosition == m_container->m_offset + m_container->m_itemsPerPage - 1) //Are we hovering the last element?
  { //then move down
    m_draggedScrollDirection = 1;
  }
  else 
  { //stop scrolling
    m_draggedScrollDirection = 0;
  }
  
  return (m_bDropable) ? EVENT_RESULT_HANDLED : EVENT_RESULT_UNHANDLED;
} 

  //DragMove
  //if reorderable
    //Handle scrolling
  //internal && dropable
    //move item
  //external && dropable
    //add item
  //external && dropable && !reorderable
    //scroll to added item

void CGUIListDragHandler::DraggedAway()
{
  if(m_bDropable)
  {
    if(!m_dragHint) //no drag hint, so the item was added or moved to our list... revert that
    {
      if(m_bInternal)
      { //move item back to it's original position
        m_container->MoveItemInternally(m_draggedNewPosition, m_draggedOrigPosition);
        m_draggedNewPosition = m_draggedOrigPosition;
        m_container->SetCursor(m_draggedNewPosition - m_container->GetOffset()); //focus the dragged object again
      }
      else 
      {
        ASSERT(m_draggedNewPosition>=(int)m_container->m_items.size());
          //remove item from our list
        m_container->m_items.erase(m_container->m_items.begin()+m_draggedNewPosition);
          //set focus to previously focused item
      
          //set default values!
        m_draggedOrigPosition = m_draggedNewPosition = -1;
      }
    }
    else 
    {
      ClearDragHint();
    }
  }
  
    //Disable any scrolling
  m_draggedScrollDirection = 0;
}
  
EVENT_RESULT CGUIListDragHandler::OnDrop()
{
  
  //Disable any scrolling
  m_draggedScrollDirection = 0;
  
  if(!m_bDropable)
    return EVENT_RESULT_UNHANDLED; //Nothing left to do!
  
  if(m_bReorderable && m_bInternal)
  { //Notifie our window about the move
    //CPoint insertPoint;
    //int newPosition = calculateDragInsertPosition(point, insertPoint);
      //Valid positions from -1 (move item to the beginning of the list) to list.size
    if(m_draggedNewPosition>-2)  //make sure the item was dropped on our list and not on our border
    {
        //if(m_draggedObject < m_draggedOrigPosition)
        //newPosition++;
      
      if(m_draggedNewPosition!=m_draggedOrigPosition) //No notice necessary, if there was no actual movement
      {
        CGUIMessage msg2(GUI_MSG_IN_LIST_DRAGGED, 0, 
                         m_container->GetParentID(), 
                         m_draggedOrigPosition, 
                         m_draggedNewPosition-m_draggedOrigPosition);
        m_container->SendWindowMessage(msg2);
      }
        //for whatever reason, the focused item is set wrong after dragging is done...
        //so, set it correct
      m_container->SetCursor(m_draggedNewPosition - m_container->GetOffset());
    }
  }
  
  if(!m_bInternal && m_draggedNewPosition>0)
  { 
    if(!m_dragHint) //remove our temporarly added item
      m_container->m_items.erase(m_container->m_items.begin()+m_draggedNewPosition);
    
    //notifie our window that a item has been added
    CGUIMessage msg2(GUI_MSG_ON_LIST_DRAGGED, 0, 
                     m_container->GetParentID(), 
                     m_draggedNewPosition);
    m_container->SendWindowMessage(msg2);
  }
  
  ClearDragHint();
  
    //Notifie the skin, that dragging has stopped
  g_infoManager.DraggingStop();
  
  return EVENT_RESULT_HANDLED;
}
  

void CGUIListDragHandler::ClearDragHint()
{
  if(m_dragHint)
  {
      //TODO: reenable: m_dragHint->SetVisible(false);
  }
  m_dragHintPosition.SetRect(0,0,0,0);
}

void CGUIListDragHandler::ShowDragHint(const CPoint& insertPoint)
{
  if (m_container->m_orientation == VERTICAL)
  {
    int width = 10; //TODO;
    int yOffset = width/2;
    m_dragHintPosition.SetRect(m_container->GetXPosition(), 
                               insertPoint.y - yOffset, 
                               m_container->GetXPosition() + m_container->GetWidth(), 
                               insertPoint.y + yOffset);
  }
  else
  {
    int height = 10; //TODO;
    int xOffset = height/2;
    m_dragHintPosition.SetRect(insertPoint.x - xOffset, 
                               m_container->GetYPosition(), 
                               insertPoint.x + xOffset,
                               m_container->GetYPosition() + m_container->GetHeight());
  }
  m_dragHint->SetVisible(true);
}
