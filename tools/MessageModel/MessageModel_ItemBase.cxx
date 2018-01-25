// Created on: 2017-06-16
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2017 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement. 


#include <inspector/MessageModel_ItemBase.hxx>
#include <inspector/MessageModel_ItemRoot.hxx>

// =======================================================================
// function : isReversed
// purpose :
// =======================================================================
Standard_Boolean MessageModel_ItemBase::isReversed() const
{
  TreeModel_ItemBasePtr aParentItem = GetRootItem();
  MessageModel_ItemRootPtr aRootItem = itemDynamicCast<MessageModel_ItemRoot> (aParentItem);
  return aRootItem ? aRootItem->IsReversed() : Standard_False;
}

// =======================================================================
// function : isUniteAlerts
// purpose :
// =======================================================================
Standard_Boolean MessageModel_ItemBase::isUniteAlerts() const
{
  TreeModel_ItemBasePtr aParentItem = GetRootItem();
  MessageModel_ItemRootPtr aRootItem = itemDynamicCast<MessageModel_ItemRoot> (aParentItem);
  return aRootItem ? aRootItem->IsUniteAlerts() : Standard_False;
}

// =======================================================================
// function : GetRootItem
// purpose :
// =======================================================================
TreeModel_ItemBasePtr MessageModel_ItemBase::GetRootItem() const
{
  TreeModel_ItemBasePtr anItem = Parent();
  while (anItem)
  {
    if (MessageModel_ItemRootPtr aThisRootItem = itemDynamicCast<MessageModel_ItemRoot> (anItem))
    {
      return aThisRootItem;
    }
    anItem = anItem->Parent();
  }
  return TreeModel_ItemBasePtr();
}
