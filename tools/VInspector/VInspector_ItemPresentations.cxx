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

#include <inspector/VInspector_ItemPresentations.hxx>

#include <inspector/VInspector_ItemPresentableObject.hxx>
#include <inspector/VInspector_ItemPrs3dPresentation.hxx>

#include <AIS_InteractiveObject.hxx>
#include <PrsMgr_PresentationManager3d.hxx>

// =======================================================================
// function : initValue
// purpose :
// =======================================================================
QVariant VInspector_ItemPresentations::initValue (int theItemRole) const
{
  if (Column() != 0 || (theItemRole != Qt::DisplayRole && theItemRole != Qt::ToolTipRole))
    return QVariant();

  return "Presentations";
}

// =======================================================================
// function : initRowCount
// purpose :
// =======================================================================
int VInspector_ItemPresentations::initRowCount() const
{
  if (Column() != 0)
    return 0;

  VInspector_ItemPresentableObjectPtr aParentPrsItem = itemDynamicCast<VInspector_ItemPresentableObject>(Parent());
  if (!aParentPrsItem)
    return 0;

  Handle(AIS_InteractiveObject) aPresentation = aParentPrsItem->GetInteractiveObject();

  int aNbProperties = 0;
  if (!aPresentation->GetSelectPresentation (NULL).IsNull())
    aNbProperties++;

  if (!aPresentation->GetHilightPresentation (NULL).IsNull())
    aNbProperties++;

  for (PrsMgr_Presentations::Iterator aPrsIter (aPresentation->Presentations()); aPrsIter.More(); aPrsIter.Next())
  {
    const Handle(PrsMgr_Presentation)& aPrs3d = aPrsIter.ChangeValue().Presentation();
    if (!aPrs3d.IsNull() && !aPrs3d->Presentation().IsNull())
      aNbProperties++;
  }
  return aNbProperties;
}

// =======================================================================
// function : createChild
// purpose :
// =======================================================================
TreeModel_ItemBasePtr VInspector_ItemPresentations::createChild (int theRow, int theColumn)
{
  return VInspector_ItemPrs3dPresentation::CreateItem (currentItem(), theRow, theColumn);
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void VInspector_ItemPresentations::Init()
{
  TreeModel_ItemBase::Init(); // to use getIO() without circling initialization
}

// =======================================================================
// function : Reset
// purpose :
// =======================================================================
void VInspector_ItemPresentations::Reset()
{
  VInspector_ItemBase::Reset();
}

// =======================================================================
// function : initItem
// purpose :
// =======================================================================
void VInspector_ItemPresentations::initItem() const
{
  if (IsInitialized())
    return;
  const_cast<VInspector_ItemPresentations*>(this)->Init();
}

// =======================================================================
// function : GetPresentation
// purpose :
// =======================================================================
Handle(Prs3d_Presentation) VInspector_ItemPresentations::GetPresentation (const int theRowId,
                                                                           TCollection_AsciiString& theName) const
{
  VInspector_ItemPresentableObjectPtr aParentPrsItem = itemDynamicCast<VInspector_ItemPresentableObject>(Parent());
  if (!aParentPrsItem)
    return 0;

  Handle(AIS_InteractiveObject) aPresentation = aParentPrsItem->GetInteractiveObject();

  bool aHasSelectedPresentation = !aPresentation->GetSelectPresentation (NULL).IsNull();
  if (aHasSelectedPresentation && theRowId == 0)
  {
    theName = "SelectPresentation";
    return aPresentation->GetSelectPresentation (NULL);
  }

  bool aHasHilightPresentation = !aPresentation->GetHilightPresentation (NULL).IsNull();
  if (aHasHilightPresentation && ((aHasSelectedPresentation && theRowId == 1) || !aHasSelectedPresentation && theRowId == 0))
  {
    theName = "HilightPresentation";
    return aPresentation->GetHilightPresentation (NULL);
  }

  int aNextPresentationIndex = aHasSelectedPresentation ? 1 : 0;
  if (aHasHilightPresentation)
    aNextPresentationIndex++;

  int aCurrentIndex = 0;
  for (PrsMgr_Presentations::Iterator aPrsIter (aPresentation->Presentations()); aPrsIter.More(); aPrsIter.Next())
  {
    const Handle(PrsMgr_Presentation)& aPrs3d = aPrsIter.ChangeValue().Presentation();
    if (aPrs3d.IsNull() || aPrs3d->Presentation().IsNull())
      continue;
    if (theRowId - aNextPresentationIndex == aCurrentIndex)
    {
      theName = QString ("Prs3d_Presentation (mode = %1)").arg (aPrsIter.ChangeValue().Mode()).toStdString().c_str();
      return aPrs3d->Presentation();
    }
    aCurrentIndex++;
  }
  return Handle(Prs3d_Presentation)();
}
