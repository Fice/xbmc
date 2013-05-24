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

#include "guilib/GUIDialog.h"
#include "GUIContextMenuManager.h"

class CMediaSource;

class CContextButtons : public std::vector< std::pair<unsigned int, CStdString> >
{
public:
  void Add(unsigned int, const CStdString &label);
  void Add(unsigned int, int label);
};

class ConvertFromContextItem 
{
  const CFileItem* m_item;
  const CFileItemList& m_container;
public:
  ConvertFromContextItem(const CFileItem* item, const CFileItemList& container) : m_item(item), m_container(container) {}
  std::pair<unsigned int, CStdString> operator()(ContextItemPtr& input);
  
};

class CGUIDialogContextMenu :
      public CGUIDialog
{
public:
  CGUIDialogContextMenu(void);
  virtual ~CGUIDialogContextMenu(void);
  virtual bool OnMessage(CGUIMessage &message);
  virtual bool OnAction(const CAction& action);
  virtual void SetPosition(float posX, float posY);

  static bool SourcesMenu(const CStdString &strType, const CFileItemPtr item, float posX, float posY);
  static void SwitchMedia(const CStdString& strType, const CStdString& strPath);

  static void GetContextButtons(const CStdString &type, const CFileItemPtr item, CContextButtons &buttons);
  static bool OnContextButton(const CStdString &type, const CFileItemPtr item, CONTEXT_BUTTON button);

  /*! \brief Show the context menu with the given choices
   \param choices the choices available for the user.
   \return -1 if no choice is made, else the chosen option.
   */
  static int ShowAndGetChoice(const CContextButtons &choices);

protected:
  void SetupButtons();

  /*! \brief Position the context menu in the middle of the focused control.
   If no control is available it is positioned in the middle of the screen.
   */
  void PositionAtCurrentFocus();

  virtual float GetWidth() const;
  virtual float GetHeight() const;
  virtual void OnInitWindow();
  virtual void OnWindowLoaded();
  virtual void OnDeinitWindow(int nextWindowID);
  static CStdString GetDefaultShareNameByType(const CStdString &strType);
  static void SetDefault(const CStdString &strType, const CStdString &strDefault);
  static void ClearDefault(const CStdString &strType);
  static CMediaSource *GetShare(const CStdString &type, const CFileItem *item);

private:
  float m_coordX, m_coordY;
  /// \brief Stored size of background image (height or width depending on grouplist orientation)
  float m_backgroundImageSize;
  int m_clickedButton;
  CContextButtons m_buttons;
};
