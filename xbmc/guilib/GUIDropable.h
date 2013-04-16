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
#include "boost/shared_ptr.hpp"

class CFileItem;
typedef boost::shared_ptr<CFileItem> CFileItemPtr;

/**
 * Class responsible to decide whether a CFileItem can be dropped on the current Directory
 */
struct IGUIDropable
{
  virtual bool IsDropable(const CFileItemPtr& item) const = 0;
  virtual IGUIDropable* Copy() const = 0;
};

struct CGUIAlwaysDropable : public IGUIDropable
{
  virtual bool IsDropable(const CFileItemPtr& item) const { return true; }
  virtual IGUIDropable* Copy() const { return new CGUIAlwaysDropable(); }
};

struct CGUIVideoDropable : public IGUIDropable
{
  virtual bool IsDropable(const CFileItemPtr& item) const;
  virtual IGUIDropable* Copy() const { return new CGUIVideoDropable(); }
};

struct CGUIMusicDropable : public IGUIDropable
{
  virtual bool IsDropable(const CFileItemPtr& item) const;
  virtual IGUIDropable*  Copy() const { return new CGUIMusicDropable(); }
};
