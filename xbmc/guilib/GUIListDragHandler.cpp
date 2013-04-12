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
  m_dragHintOffset((dragHint) ? CPoint(dragHint->GetXPosition(), dragHint->GetYPosition()) : CPoint()),
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
    //Do we need to scroll?
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
        //g_graphicsContext.SetOrigin(m_dragHintPosition.x, m_dragHintPosition.y);
        //g_graphicsContext.SetOrigin(m_container->GetXPosition(), m_container->GetXPosition());
      m_dragHint->UpdateVisibility();
        //m_dragHint->SetPosition(m_dragHintPosition.x+m_dragHintOffset.x, m_dragHintPosition.y+m_dragHintOffset.y);
      unsigned int oldDirty = dirtyregions.size();
      m_dragHint->DoProcess(currentTime, dirtyregions);
      if (m_dragHint->IsVisible() || (oldDirty != dirtyregions.size())) // visible or dirty (was visible?)
        m_container->m_renderRegion.Union(m_dragHint->GetRenderRegion());
      
        //g_graphicsContext.SetOrigin(m_dragHintPosition.x1, m_dragHintPosition.x2);
        //m_dragHint->Process(currentTime, dirtyregions);
        //g_graphicsContext.RestoreOrigin();
    }
  }
  
}


void CGUIListDragHandler::Render() //all public
{
    //if we have a draghint, we need to render it
  if(m_dragHint && m_bDropable)
  {
    CGUITexture::DrawQuad(CRect(m_dragHintPosition.x+m_dragHintOffset.x, 
                                m_dragHintPosition.y-5+m_dragHintOffset.y,
                                m_dragHintPosition.x+100+m_dragHintOffset.x, 
                                m_dragHintPosition.y+5+m_dragHintOffset.y), 0x4c00ff00);
      //g_graphicsContext.SetOrigin(m_dragHintPosition.x1, m_dragHintPosition.x2);
    if(m_dragHintPosition.x!=0) //TODO: remove
    {
        //g_graphicsContext.SetOrigin(m_dragHintPosition.x, m_dragHintPosition.y);
    m_dragHint->DoRender();
      
        //g_graphicsContext.RestoreOrigin();
    }
  }
}

void CGUIListDragHandler::DragStart(const CPoint& point) //m_items and m_focusedLayout is protected
{
  if(m_bInternal)
  {
    //Store the item that the user wants to drag for later use
    CFileItemPtr draggedItem;
    int selected = m_container->GetSelectedItem();
    if (selected >= 0 && selected < (int)m_container->m_items.Size())
    {
      draggedItem = m_container->m_items[selected];
      m_draggedOrigPosition = m_draggedNewPosition = selected;
    }
      //Let the skinner have access to drag&drop info stuff
    g_infoManager.DraggingStart(draggedItem, m_container);

  }
  else //Dragging started earlier, but now we're finnaly hovered!
  {
    CFileItemPtr draggedItem = g_infoManager.GetDraggedFileItem();
    ASSERT(draggedItem);
    
    if(m_bReorderable)
    {
      CRect insertRect;
      m_draggedNewPosition = m_container->calculateDragInsertPosition(point, insertRect);
      if(m_dragHint)
      { //set draghint position
        CalcDragHint(point, insertRect, m_draggedNewPosition);
        ShowDragHint();
      }
      else 
      { //insert the item at the correct position
        m_container->m_items.AddFront(draggedItem, m_draggedNewPosition);
      }
    }
    else {
        //add item to the end
      m_container->m_items.Add(draggedItem);
        //now let the responsible entity sort the m_items vector
        //now find the position of our draggedItem
      m_draggedNewPosition = m_container->m_items.Size()-1;
      
      if(m_dragHint)
      {
        m_container->m_items.Remove(m_draggedNewPosition); //remove the item again... we only wanted to get the sorted position
        //TODO: get CPoint for dragged position
          //TODO:ShowDragHint(ShowDragHint)
      }
      
      
    }
    

  }
}

EVENT_RESULT CGUIListDragHandler::DragMove(const CPoint &point)
{
  CRect insertPoint;
  int newPosition = m_container->calculateDragInsertPosition(point, insertPoint);
  CalcDragHint(point, insertPoint, newPosition);

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
      if (m_dragHint) //we have a drag hint, so let's calculate it's position and  make it visible
      {
        if (newPosition < m_draggedOrigPosition)
          newPosition++;
        m_draggedNewPosition = newPosition;
        if(m_bInternal)
        {
          if (m_draggedNewPosition != m_draggedOrigPosition) //only if the drop position is different then the item's original position
            ShowDragHint();
          else //Don't show drag hint, if user drags on current item position
            ClearDragHint();
        }
        else
        { //Always show drag hint when sth. gets draged onto this list
          ShowDragHint();
        }
      } 
      else //we don't have a drag hint, so lets reorder immediately
      {
        m_container->MoveItemInternally(m_draggedNewPosition, newPosition);
        m_container->SetCursor(newPosition - m_container->GetOffset());
        m_container->m_items[newPosition]->Select(true);
        m_draggedNewPosition=newPosition;
      }      
    }
  }
 
  
  m_draggedScrollDirection = m_container->NeedsScrolling(point);
  
  return (m_bDropable) ? EVENT_RESULT_HANDLED : EVENT_RESULT_UNHANDLED;
} 

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
        m_container->m_items[m_draggedNewPosition]->Select(true);
      }
      else 
      {
        if(m_draggedNewPosition >= 0 && m_draggedNewPosition < m_container->m_items.Size());
        {
          //remove item from our list
          m_container->m_items.Remove(m_draggedNewPosition);
          //set focus to previously focused item
        }
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
      m_container->m_items.Remove(m_draggedNewPosition);
    
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
    m_dragHint->SetVisible(false);
    m_dragHint->SetPosition(m_dragHintOffset.x, m_dragHintOffset.y);
  }
}

void CGUIListDragHandler::CalcDragHint(const CPoint& mouse, const CRect& hoveredArea, int& Pos)
{
  if(!m_dragHint)
    return;
  
  if (m_container->m_orientation == VERTICAL)
  {
      //check if the drag hint is supposed to be above or under the item
    if(mouse.y < hoveredArea.y1 + (hoveredArea.y2-hoveredArea.y1) / 2) 
    {
      if(m_bReorderable)
        Pos--;
      m_dragHintPosition.y = hoveredArea.y1;
    }
    else 
    {
      m_dragHintPosition.y = hoveredArea.y2;
    }
    
    m_dragHintPosition.x = m_container->GetXPosition();
    
  }
  else
  {
      //check if the drag hint is supposed to be on the left, or right
    if(mouse.y < hoveredArea.y1 + (hoveredArea.y2-hoveredArea.y1) / 2) 
    {
      if(m_bReorderable)
        Pos--;
      m_dragHintPosition.x = hoveredArea.x1;
    }
    else 
    {
      m_dragHintPosition.x = hoveredArea.x2;
    }
    m_dragHintPosition.y = m_container->GetYPosition();
  }
  
  CLog::Log(LOGDEBUG, "adjusted drop position %i", m_draggedNewPosition);
}

void CGUIListDragHandler::ShowDragHint()
{
  if(!m_dragHint)
    return; //Nothing to do
  
/*  if (m_container->m_orientation == VERTICAL)
  {
    m_dragHintPosition.x = m_container->GetXPosition();
    m_dragHintPosition.y = insertPoint.y;
  }
  else
  {
    m_dragHintPosition.x = insertPoint.x;
    m_dragHintPosition.y = m_container->GetYPosition();
  }*/
  
  m_dragHint->SetVisible(true);
  m_dragHint->SetPosition(m_dragHintPosition.x+m_dragHintOffset.x, m_dragHintPosition.y+m_dragHintOffset.y);
  m_dragHint->SetInvalid();
  
}
