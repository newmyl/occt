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

#include <inspector/VInspector_ItemPresentableObject.hxx>

#include <AIS_Shape.hxx>
#include <AIS_ListOfInteractive.hxx>
#include <AIS_ListIteratorOfListOfInteractive.hxx>
#include <Aspect.hxx>

#include <inspector/VInspector_ItemContext.hxx>
#include <inspector/VInspector_ItemSelectBasicsEntityOwner.hxx>
#include <inspector/VInspector_ItemFolderObject.hxx>
#include <inspector/VInspector_ItemPresentations.hxx>
#include <inspector/VInspector_ItemSelectMgrSelection.hxx>
#include <inspector/ViewControl_Table.hxx>
#include <inspector/ViewControl_Tools.hxx>
#include <inspector/VInspector_Tools.hxx>
#include <inspector/VInspector_ViewModel.hxx>

#include <NCollection_List.hxx>
#include <Prs3d.hxx>
#include <Prs3d_Drawer.hxx>
#include <PrsMgr.hxx>
#include <SelectBasics_EntityOwner.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <Standard_Version.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QColor>
#include <QItemSelectionModel>
#include <Standard_WarningsRestore.hxx>

// =======================================================================
// function : initValue
// purpose :
// =======================================================================
QVariant VInspector_ItemPresentableObject::initValue (int theItemRole) const
{
  if (Column() == 20 && theItemRole == Qt::BackgroundRole) {
    Handle(AIS_InteractiveObject) anIO = GetInteractiveObject();
    if (!anIO.IsNull() && anIO->HasColor())
    {
      Quantity_Color aColor;
      anIO->Color(aColor);
      return QColor ((int)(aColor.Red()*255.), (int)(aColor.Green()*255.), (int)(aColor.Blue()*255.));
    }
  }

  if (theItemRole == Qt::DisplayRole || theItemRole == Qt::ToolTipRole)
  {
    Handle(AIS_InteractiveObject) anIO = GetInteractiveObject();
    bool aNullIO = anIO.IsNull();
    switch (Column())
    {
      case 0:
      {
        if (aNullIO)
          return theItemRole == Qt::ToolTipRole ? QVariant ("Owners where Selectable is empty")
                                                : QVariant ("Free Owners");
        else
          return theItemRole == Qt::ToolTipRole ? QVariant ("")
                                                : QVariant (anIO->DynamicType()->Name());
      }
      case 1:
        return rowCount();
      case 2:
      {
        if (!aNullIO)
          return VInspector_Tools::GetPointerInfo (anIO, true).ToCString();
        break;
      }
      case 3:
      {
        Handle(AIS_Shape) aShapeIO = Handle(AIS_Shape)::DownCast (anIO);
        if (!aShapeIO.IsNull())
        {
          const TopoDS_Shape& aShape = aShapeIO->Shape();
          if (!aShape.IsNull())
            return VInspector_Tools::GetShapeTypeInfo (aShape.ShapeType()).ToCString();
        }
        break;
      }
      case 4:
      {
        int aNbSelected = VInspector_Tools::SelectedOwners (GetContext(), anIO, false);
        return aNbSelected > 0 ? QString::number (aNbSelected) : "";
      }
      case 5:
      {
        TColStd_ListOfInteger aModes;
        Handle(AIS_InteractiveContext) aContext = GetContext();
        aContext->ActivatedModes(anIO, aModes);
        TCollection_AsciiString aModesInfo;
        for (TColStd_ListIteratorOfListOfInteger itr (aModes); itr.More(); itr.Next())
        {
          if (!aModesInfo.IsEmpty())
            aModesInfo += ", ";
          aModesInfo += VInspector_Tools::GetShapeTypeInfo (AIS_Shape::SelectionType(itr.Value()));
        }
        return aModesInfo.ToCString();
      }
      break;
      case 6:
      {
        double aDeviationCoefficient = 0;
        Handle(AIS_Shape) anAISShape = Handle(AIS_Shape)::DownCast (anIO);
        if (!anAISShape.IsNull())
        {
          Standard_Real aPreviousCoefficient;
          anAISShape->OwnDeviationCoefficient(aDeviationCoefficient, aPreviousCoefficient);
        }
        return QString::number(aDeviationCoefficient);
      }
      case 7:
      {
        double aShapeDeflection = 0;
        Handle(AIS_Shape) aShapeIO = Handle(AIS_Shape)::DownCast (anIO);
        if (!aShapeIO.IsNull())
        {
          const TopoDS_Shape& aShape = aShapeIO->Shape();
          if (!aShape.IsNull())
            aShapeDeflection = Prs3d::GetDeflection(aShape, anIO->Attributes());
        }
        return QString::number (aShapeDeflection);
      }
      case 8:
      {
        double aDeviationCoefficient = 0;
        Handle(AIS_Shape) anAISShape = Handle(AIS_Shape)::DownCast (anIO);
        if (!anAISShape.IsNull())
        {
          Standard_Real aPreviousCoefficient;
          anAISShape->OwnDeviationCoefficient(aDeviationCoefficient, aPreviousCoefficient);
        }
        Handle(AIS_Shape) aShapeIO = Handle(AIS_Shape)::DownCast (anIO);
        bool anIsAutoTriangulation = aNullIO ? false : anIO->Attributes()->IsAutoTriangulation();
        return anIsAutoTriangulation ? QString ("true") : QString ("false");
      }
      case 17:
      case 18:
      case 19:
        {
        Handle(AIS_Shape) aShapeIO = Handle(AIS_Shape)::DownCast (anIO);
        if (aShapeIO.IsNull())
          return QVariant();
        const TopoDS_Shape& aShape = aShapeIO->Shape();
        if (aShape.IsNull())
          return QVariant();

        return Column() == 17 ? VInspector_Tools::GetPointerInfo (aShape.TShape(), true).ToCString()
              : Column() == 18 ? VInspector_Tools::OrientationToName (aShape.Orientation()).ToCString()
              :           /*19*/ ViewControl_Tools::ToString (aShape.Location()).ToCString();
      }
      default: break;
    }
  }
  if (theItemRole == Qt::BackgroundRole || theItemRole == Qt::ForegroundRole)
  {
    Handle(AIS_InteractiveContext) aContext = GetContext();
    if (Column() == 2 && VInspector_Tools::SelectedOwners(aContext, GetInteractiveObject(), false) > 0)
    {
      return (theItemRole == Qt::BackgroundRole) ? QColor(Qt::darkBlue) : QColor(Qt::white);
    }
    else if (theItemRole == Qt::ForegroundRole)
    {
      Handle(AIS_InteractiveObject) anIO = GetInteractiveObject();
      if (anIO.IsNull())
        return QVariant();

      AIS_ListOfInteractive aListOfIO;
      GetContext()->ErasedObjects(aListOfIO);
      for (AIS_ListIteratorOfListOfInteractive anIOIt(aListOfIO); anIOIt.More(); anIOIt.Next())
      {
        if (anIO == anIOIt.Value())
          return QColor(Qt::darkGray);
      }
      return QColor(Qt::black);
    }
  }
  return QVariant();
}

// =======================================================================
// function : initRowCount
// purpose :
// =======================================================================
int VInspector_ItemPresentableObject::initRowCount() const
{
  if (Column() != 0)
    return 0;

  int aNbProperties = 2; // "Properties", "Presentations"

  Handle(AIS_InteractiveObject) anIO = GetInteractiveObject();
#if OCC_VERSION_HEX < 0x070201
  int aRows = 0;
  if (anIO.IsNull())
    return aRows;
  // iteration through sensitive privitives
  for (anIO->Init(); anIO->More(); anIO->Next())
    aRows++;
  int aNbSelected = aRows;
#else
  int aNbSelected = !anIO.IsNull() ? anIO->Selections().Size() : 0;
#endif

  return aNbProperties + aNbSelected;
}

// =======================================================================
// function : createChild
// purpose :
// =======================================================================
TreeModel_ItemBasePtr VInspector_ItemPresentableObject::createChild (int theRow, int theColumn)
{
  if (theRow == 0)
    return VInspector_ItemFolderObject::CreateItem (currentItem(), theRow, theColumn);
  if (theRow == 1)
    return VInspector_ItemPresentations::CreateItem (currentItem(), theRow, theColumn);
  else
    return VInspector_ItemSelectMgrSelection::CreateItem(currentItem(), theRow, theColumn);
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void VInspector_ItemPresentableObject::Init()
{
  VInspector_ItemContextPtr aParentItem = itemDynamicCast<VInspector_ItemContext>(Parent());
  Handle(AIS_InteractiveContext) aContext = aParentItem->GetContext();
  SetContext (aContext);

  Handle(AIS_InteractiveObject) anIO;
  if (!GetContext().IsNull())
  {
    int aRowId = Row();
    AIS_ListOfInteractive aListOfIO;
    GetContext()->DisplayedObjects (aListOfIO); // the presentation is in displayed objects of Context
    GetContext()->ErasedObjects (aListOfIO); // the presentation is in erased objects of Context
    int aDeltaIndex = 1; // properties item
    int aCurrentIndex = 0;
    for (AIS_ListIteratorOfListOfInteractive anIOIt (aListOfIO); anIOIt.More(); anIOIt.Next(), aCurrentIndex++)
    {
      if (aCurrentIndex != aRowId - aDeltaIndex)
        continue;
      anIO = anIOIt.Value();
      break;
    }
  }

  setInteractiveObject (anIO);

  Handle(AIS_Shape) aShape = Handle(AIS_Shape)::DownCast (anIO);
  if (!aShape.IsNull())
    myPresentationShape = aShape->Shape();

  TreeModel_ItemBase::Init(); // to use getIO() without circling initialization
}

// =======================================================================
// function : Reset
// purpose :
// =======================================================================
void VInspector_ItemPresentableObject::Reset()
{
  VInspector_ItemBase::Reset();

  SetContext (NULL);
  setInteractiveObject (NULL);
}

// =======================================================================
// function : initItem
// purpose :
// =======================================================================
void VInspector_ItemPresentableObject::initItem() const
{
  if (IsInitialized())
    return;
  const_cast<VInspector_ItemPresentableObject*>(this)->Init();
}

// =======================================================================
// function : GetInteractiveObject
// purpose :
// =======================================================================
Handle(AIS_InteractiveObject) VInspector_ItemPresentableObject::GetInteractiveObject() const
{
  initItem();
  return myIO;
}

// =======================================================================
// function : PointerInfo
// purpose :
// =======================================================================
QString VInspector_ItemPresentableObject::PointerInfo() const
{
  return VInspector_Tools::GetPointerInfo (GetInteractiveObject(), true).ToCString();
}

// =======================================================================
// function : GetPresentations
// purpose :
// =======================================================================
void VInspector_ItemPresentableObject::GetPresentations (NCollection_List<Handle(Standard_Transient)>& thePresentations)
{
  if (Column() != 0)
    return;

  thePresentations.Append (GetInteractiveObject());
}

// =======================================================================
// function : GetTableRowCount
// purpose :
// =======================================================================
int VInspector_ItemPresentableObject::GetTableRowCount() const
{
  return 23;
}

// =======================================================================
// function : GetTableEditType
// purpose :
// =======================================================================
ViewControl_EditType VInspector_ItemPresentableObject::GetTableEditType (const int theRow, const int) const
{
  switch (theRow)
  {
    case 4: return ViewControl_EditType_Line;
    case 5: return ViewControl_EditType_Combo;
    case 6: return ViewControl_EditType_Bool;
    case 12: return ViewControl_EditType_Bool;
    case 17: return ViewControl_EditType_Combo;
    case 18: return ViewControl_EditType_Bool;
    case 22: return ViewControl_EditType_Bool;
    default: return ViewControl_EditType_None;
  }
}

// =======================================================================
// function : GetTableEnumValues
// purpose :
// =======================================================================
QList<QVariant> VInspector_ItemPresentableObject::GetTableEnumValues (const int theRow, const int) const
{
  QList<QVariant> aValues;
  switch (theRow)
  {
    case 5:
    {
      for (int i = 0; i <= Aspect_TOFM_FRONT_SIDE; i++)
        aValues.append (Aspect::TypeOfFacingModelToString((Aspect_TypeOfFacingModel)i));
    }
    break;
    case 17:
    {
      for (int i = 0; i <= PrsMgr_TOP_ProjectorDependant; i++)
        aValues.append (PrsMgr::TypeOfPresentation3dToString ((PrsMgr_TypeOfPresentation3d)i));
    }
    break;
    default: break;
  }
  return aValues;
}

// =======================================================================
// function : GetTableData
// purpose :
// =======================================================================
QVariant VInspector_ItemPresentableObject::GetTableData (const int theRow, const int theColumn, const int theRole) const
{
  if (theRole != Qt::DisplayRole)
    return QVariant();

  bool isFirstColumn = theColumn == 0;

  Handle(AIS_InteractiveObject) aPrs = GetInteractiveObject();
  switch (theRow)
  {
    case 0: return ViewControl_Table::SeparatorData();
    case 1: return isFirstColumn ? QVariant (STANDARD_TYPE (AIS_InteractiveObject)->Name())
                                 : ViewControl_Tools::GetPointerInfo (aPrs).ToCString();
    case 2: return ViewControl_Table::SeparatorData();
    case 3: return isFirstColumn ? QVariant ("HasWidth") : QVariant (aPrs->HasWidth());
    case 4: return isFirstColumn ? QVariant ("Width") : QVariant (aPrs->Width());
    case 5: return isFirstColumn ? QVariant ("CurrentFacingModel")
                                 : QVariant (Aspect::TypeOfFacingModelToString (aPrs->CurrentFacingModel()));
    case 6: return isFirstColumn ? QVariant ("IsInfinite") : QVariant (aPrs->IsInfinite());
    case 7: return isFirstColumn ? QVariant ("HasColor") : QVariant (aPrs->HasColor());
    case 8: return isFirstColumn ? QVariant ("HasMaterial") : QVariant (aPrs->HasMaterial());

    case 9: return ViewControl_Table::SeparatorData();
    case 10: return isFirstColumn ? QVariant (STANDARD_TYPE (SelectMgr_SelectableObject)->Name())
                                 : ViewControl_Tools::GetPointerInfo (aPrs).ToCString();
    case 11: return ViewControl_Table::SeparatorData();
    case 12: return isFirstColumn ? QVariant ("IsAutoHilight") : QVariant (aPrs->IsAutoHilight());
    case 13: return isFirstColumn ? QVariant ("GlobalSelectionMode") : QVariant (aPrs->GlobalSelectionMode());

    case 14: return ViewControl_Table::SeparatorData();
    case 15: return isFirstColumn ? QVariant (STANDARD_TYPE (PrsMgr_PresentableObject)->Name())
                                 : ViewControl_Tools::GetPointerInfo (aPrs).ToCString();
    case 16: return ViewControl_Table::SeparatorData();
    case 17: return isFirstColumn ? QVariant ("TypeOfPresentation3d")
                                  : QVariant (PrsMgr::TypeOfPresentation3dToString (aPrs->TypeOfPresentation3d()));
    case 18: return isFirstColumn ? QVariant ("IsMutable") : QVariant (aPrs->IsMutable());
    case 19: return isFirstColumn ? QVariant ("HasOwnPresentations") : QVariant (aPrs->HasOwnPresentations());
    case 20: return isFirstColumn ? QVariant ("TransformationGeom")
      : (!aPrs->TransformationGeom().IsNull()
        ? QVariant (ViewControl_Tools::ToString(aPrs->TransformationGeom()->Trsf()).ToCString()) : QVariant());
    case 21: return isFirstColumn ? QVariant ("LocalTransformationGeom")
      : (!aPrs->LocalTransformationGeom().IsNull()
        ? QVariant (ViewControl_Tools::ToString(aPrs->LocalTransformationGeom()->Trsf()).ToCString()) : QVariant());
    case 22: return isFirstColumn ? QVariant ("ResetTransformation") : (!aPrs->LocalTransformationGeom().IsNull());
    default: return QVariant();
  }
  return QVariant();
}

// =======================================================================
// function : SetTableData
// purpose :
// =======================================================================
bool VInspector_ItemPresentableObject::SetTableData (const int theRow, const int, const QVariant& theValue)
{
  Handle(AIS_InteractiveObject) aPrs = GetInteractiveObject();
  switch (theRow)
  {
    case 4:
    {
      double aValue = theValue.toDouble();
      if (aValue > 0) aPrs->SetWidth (aValue);
      else aPrs->UnsetWidth();
    }
    break;
    case 5: aPrs->SetCurrentFacingModel (Aspect::TypeOfFacingModelFromString (theValue.toString().toStdString().c_str()));
    case 6: aPrs->SetInfiniteState (theValue.toBool());
    case 12: aPrs->SetAutoHilight(theValue.toBool());
    case 17: aPrs->SetTypeOfPresentation (PrsMgr::TypeOfPresentation3dFromString (theValue.toString().toStdString().c_str()));
    case 18: aPrs->SetMutable (theValue.toBool());
    case 22: if (!theValue.toBool()) aPrs->ResetTransformation();
    default: return false;
  }
  return true;
}
