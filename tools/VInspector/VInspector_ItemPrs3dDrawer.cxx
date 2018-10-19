// Created on: 2018-08-10
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

#include <inspector/VInspector_ItemPrs3dDrawer.hxx>
#include <inspector/VInspector_ItemPrs3dAspect.hxx>
#include <inspector/VInspector_Tools.hxx>

#include <inspector/VInspector_ItemFolderObject.hxx>
#include <inspector/ViewControl_ColorSelector.hxx>
#include <inspector/ViewControl_Table.hxx>
#include <inspector/ViewControl_Tools.hxx>

#include <Aspect.hxx>
#include <Graphic3d.hxx>
#include <Prs3d.hxx>
#include <Prs3d_ArrowAspect.hxx>
#include <Prs3d_DatumAspect.hxx>
#include <Prs3d_DimensionAspect.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_IsoAspect.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_PlaneAspect.hxx>
#include <Prs3d_PointAspect.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Prs3d_TextAspect.hxx>
#include <TCollection_AsciiString.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QColor>
#include <Standard_WarningsRestore.hxx>

// =======================================================================
// function : initValue
// purpose :
// =======================================================================

QVariant VInspector_ItemPrs3dDrawer::initValue (int theItemRole) const
{
  Handle(Prs3d_Drawer) aDrawer = GetDrawer();
  bool aNullDrawer = aDrawer.IsNull();
  if (theItemRole == Qt::DisplayRole || theItemRole == Qt::ToolTipRole)
  {
    switch (Column())
    {
      case 0:
      {
        return theItemRole == Qt::ToolTipRole
          ? (aNullDrawer ? QVariant("Prs3d_Drawer is empty") : QVariant (aDrawer->DynamicType()->Name()))
          : QVariant (myName.ToCString());
      }
      case 1:
        return rowCount();
      case 2:
      {
        if (!aNullDrawer)
          return VInspector_Tools::GetPointerInfo (aDrawer, true).ToCString();
        break;
      }
      default: break;
    }
  }
  if (theItemRole == Qt::ForegroundRole)
    return !aNullDrawer ? QColor (Qt::black) : QColor (Qt::lightGray);

  return QVariant();
}

// =======================================================================
// function : initRowCount
// purpose :
// =======================================================================

int VInspector_ItemPrs3dDrawer::initRowCount() const
{
  VInspector_ItemFolderObjectPtr aParentItem = itemDynamicCast<VInspector_ItemFolderObject>(Parent());
  if (!aParentItem)
    return 0;

  Handle(Prs3d_Drawer) aDrawer = GetDrawer();
  if (aDrawer.IsNull())
    return 0;

  int aChildCount = 1; // Link
  aChildCount += 18; // Prs3d_Drawer aspects
  return aChildCount;
}

// =======================================================================
// function : createChild
// purpose :
// =======================================================================

TreeModel_ItemBasePtr VInspector_ItemPrs3dDrawer::createChild (int theRow, int theColumn)
{
  if (theRow == 0)
  {
    VInspector_ItemFolderObjectPtr aParentItem = itemDynamicCast<VInspector_ItemFolderObject>(Parent());
    if (aParentItem)
      return VInspector_ItemPrs3dDrawer::CreateItem (currentItem(), theRow, theColumn); // "Link"
  }
  else
    return VInspector_ItemPrs3dAspect::CreateItem (currentItem(), theRow, theColumn); // "Aspects"

  return TreeModel_ItemBasePtr();
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================

void VInspector_ItemPrs3dDrawer::Init()
{
  VInspector_ItemFolderObjectPtr aParentItem = itemDynamicCast<VInspector_ItemFolderObject>(Parent());
  Handle(Prs3d_Drawer) aDrawer;
  TCollection_AsciiString aName;
  if (aParentItem)
    aDrawer = aParentItem->GetPrs3dDrawer(Row(), aName);
  else
  {
    VInspector_ItemPrs3dDrawerPtr aParentDrawerItem = itemDynamicCast<VInspector_ItemPrs3dDrawer>(Parent());
    if (aParentDrawerItem)
    {
      aDrawer = aParentDrawerItem->GetDrawer()->Link();
      aName = "Link";
    }
  }
  setDrawer (aDrawer, aName);
  TreeModel_ItemBase::Init(); // to use getIO() without circling initialization
}

// =======================================================================
// function : Reset
// purpose :
// =======================================================================

void VInspector_ItemPrs3dDrawer::Reset()
{
  VInspector_ItemBase::Reset();

  setDrawer (NULL, "None");
}

// =======================================================================
// function : initItem
// purpose :
// =======================================================================

void VInspector_ItemPrs3dDrawer::initItem() const
{
  if (IsInitialized())
    return;
  const_cast<VInspector_ItemPrs3dDrawer*>(this)->Init();
}

// =======================================================================
// function : GetDrawer
// purpose :
// =======================================================================

Handle(Prs3d_Drawer) VInspector_ItemPrs3dDrawer::GetDrawer() const
{
  initItem();
  return myDrawer;
}

// =======================================================================
// function : GetPrs3dAspect
// purpose :
// =======================================================================

Handle(Prs3d_BasicAspect) VInspector_ItemPrs3dDrawer::GetPrs3dAspect (const int theRow,
                                                                      TCollection_AsciiString& theName,
                                                                      Standard_Boolean& theOwnAspect) const
{
  if (theRow == 0) // "Link" driver, there is no aspect
    return 0;

  Handle(Prs3d_Drawer) aDrawer = GetDrawer();
  if (aDrawer.IsNull())
    return 0;

  // do not create a new instance of the aspect
  switch (theRow)
  {
    case 1: theName = "UIsoAspect"; theOwnAspect = aDrawer->HasOwnUIsoAspect(); return aDrawer->UIsoAspect();
    case 2: theName = "VIsoAspect"; theOwnAspect = aDrawer->HasOwnVIsoAspect(); return aDrawer->VIsoAspect();
    case 3: theName = "WireAspect"; theOwnAspect = aDrawer->HasOwnWireAspect(); return aDrawer->WireAspect();
    case 4: theName = "PointAspect"; theOwnAspect = aDrawer->HasOwnPointAspect(); return aDrawer->PointAspect();
    case 5: theName = "LineAspect"; theOwnAspect = aDrawer->HasOwnLineAspect(); return aDrawer->LineAspect();
    case 6: theName = "TextAspect"; theOwnAspect = aDrawer->HasOwnTextAspect(); return aDrawer->TextAspect();
    case 7: theName = "ShadingAspect"; theOwnAspect = aDrawer->HasOwnShadingAspect(); return aDrawer->ShadingAspect();
    case 8: theName = "PlaneAspect"; theOwnAspect = aDrawer->HasOwnPlaneAspect(); return aDrawer->PlaneAspect();
    case 9: theName = "SeenLineAspect"; theOwnAspect = aDrawer->HasOwnSeenLineAspect(); return aDrawer->SeenLineAspect();
    case 10: theName = "ArrowAspect"; theOwnAspect = aDrawer->HasOwnArrowAspect(); return aDrawer->ArrowAspect();
    case 11: theName = "HiddenLineAspect"; theOwnAspect = aDrawer->HasOwnHiddenLineAspect(); return aDrawer->HiddenLineAspect();
    case 12: theName = "VectorAspect"; theOwnAspect = aDrawer->HasOwnVectorAspect(); return aDrawer->VectorAspect();
    case 13: theName = "DatumAspect"; theOwnAspect = aDrawer->HasOwnDatumAspect(); return aDrawer->DatumAspect();
    case 14: theName = "SectionAspect"; theOwnAspect = aDrawer->HasOwnSectionAspect(); return aDrawer->SectionAspect();
    case 15: theName = "FreeBoundaryAspect"; theOwnAspect = aDrawer->HasOwnFreeBoundaryAspect(); return aDrawer->FreeBoundaryAspect();
    case 16: theName = "UnFreeBoundaryAspect"; theOwnAspect = aDrawer->HasOwnUnFreeBoundaryAspect(); return aDrawer->UnFreeBoundaryAspect();
    case 17: theName = "FaceBoundaryAspect"; theOwnAspect = aDrawer->HasOwnFaceBoundaryAspect(); return aDrawer->FaceBoundaryAspect();
    case 18: theName = "DimensionAspect"; theOwnAspect = aDrawer->HasOwnDimensionAspect(); return aDrawer->DimensionAspect();
    default: break;
  }

  theName = "None";
  return Handle(Prs3d_BasicAspect)();
}

// =======================================================================
// function : GetTableRowCount
// purpose :
// =======================================================================
int VInspector_ItemPrs3dDrawer::GetTableRowCount() const
{
  Handle(Prs3d_Drawer) aDrawer = GetDrawer();
  if (aDrawer.IsNull())
    return 0;

  return 76;
}

// =======================================================================
// function : GetTableEditType
// purpose :
// =======================================================================
ViewControl_EditType VInspector_ItemPrs3dDrawer::GetTableEditType (const int theRow, const int) const
{
  Handle(Prs3d_Drawer) aDrawer = GetDrawer();
  if (aDrawer.IsNull())
    return ViewControl_EditType_None;

  switch (theRow)
  {
    case 4: return ViewControl_EditType_Spin;
    case 6: return ViewControl_EditType_Line;
    case 8: return ViewControl_EditType_Line;
    case 10: return ViewControl_EditType_Combo;
    case 12: return ViewControl_EditType_Combo;
    case 13: return ViewControl_EditType_Bool;
    case 14: return ViewControl_EditType_Line;
    case 15: return ViewControl_EditType_Bool;
    case 16: return ViewControl_EditType_Line;
    case 18: return ViewControl_EditType_Line;
    case 21: return ViewControl_EditType_Line;
    case 24: return ViewControl_EditType_Bool;
    case 26: return ViewControl_EditType_Bool;
    case 28: return ViewControl_EditType_Bool;
    // aspects
    case 30:
    case 31:
    case 32:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 43:
    case 44:
    case 46:
    case 47: return ViewControl_EditType_Bool;

    case 48: return ViewControl_EditType_Combo;

    case 49:
    case 50:
    case 51:
    case 53:
    case 54:
    case 56:
    case 57:
    case 59:
    case 60: return ViewControl_EditType_Bool;

    case 62:
    case 64:
    case 66:
    case 68: return ViewControl_EditType_Line;
    case 72: return ViewControl_EditType_Color;

    case 73: return ViewControl_EditType_Combo;
    case 74: return ViewControl_EditType_Combo;
    case 75: return ViewControl_EditType_Spin;

    default: return ViewControl_EditType_None;
  }
}

// =======================================================================
// function : GetTableEnumValues
// purpose :
// =======================================================================
QList<QVariant> VInspector_ItemPrs3dDrawer::GetTableEnumValues (const int theRow, const int) const
{
  Handle(Prs3d_Drawer) aDrawer = GetDrawer();
  if (aDrawer.IsNull())
    return QList<QVariant>();

  QList<QVariant> aValues;
  switch (theRow)
  {
    case 10:
    {
      for (int i = 0; i <= Aspect_TOD_ABSOLUTE; i++)
        aValues.append (Aspect::TypeOfDeflectionToString ((Aspect_TypeOfDeflection)i));
    }
    break;
    case 12:
    {
      for (int i = 0; i <= Prs3d_TOH_Algo; i++)
        aValues.append (Prs3d::TypeOfHLRToString ((Prs3d_TypeOfHLR)i));
    }
    break;
    case 48:
    {
      for (int i = 0; i <= Prs3d_VDM_Inherited; i++)
        aValues.append (Prs3d::VertexDrawModeToString ((Prs3d_VertexDrawMode)i));
    }
    break;
    case 73:
    {
      for (int i = 0; i <= Aspect_TOHM_BOUNDBOX; i++)
        aValues.append (Aspect::TypeOfHighlightMethodToString ((Aspect_TypeOfHighlightMethod)i));
    }
    break;
    case 74:
    {
      aValues.append (Graphic3d::ZLayerIdToString (Graphic3d_ZLayerId_UNKNOWN));
      aValues.append (Graphic3d::ZLayerIdToString (Graphic3d_ZLayerId_Default));
      aValues.append (Graphic3d::ZLayerIdToString (Graphic3d_ZLayerId_Top));
      aValues.append (Graphic3d::ZLayerIdToString (Graphic3d_ZLayerId_Topmost));
      aValues.append (Graphic3d::ZLayerIdToString (Graphic3d_ZLayerId_TopOSD));
      aValues.append (Graphic3d::ZLayerIdToString (Graphic3d_ZLayerId_BotOSD));
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
QVariant VInspector_ItemPrs3dDrawer::GetTableData (const int theRow, const int theColumn, const int theRole) const
{
  Handle(Prs3d_Drawer) aDrawer = GetDrawer();
  if (aDrawer.IsNull())
    return QVariant();

  bool isFirstColumn = theColumn == 0;
  if (theRole != Qt::DisplayRole && theRole != Qt::BackgroundRole ||
      (theRole == Qt::BackgroundRole && (isFirstColumn || theRow != 72)))
    return QVariant();

  switch (theRow)
  {
    case 0: return ViewControl_Table::SeparatorData();
    case 1: return isFirstColumn ? QVariant (STANDARD_TYPE (Prs3d_Drawer)->Name())
                                 : ViewControl_Tools::GetPointerInfo (aDrawer).ToCString();
    case 2: return ViewControl_Table::SeparatorData();
    case 3: return isFirstColumn ? QVariant ("HasOwnDiscretisation") : QVariant (aDrawer->HasOwnDiscretisation());
    case 4: return isFirstColumn ? QVariant ("Discretisation") : QVariant (aDrawer->Discretisation());
    case 5: return isFirstColumn ? QVariant ("HasOwnMaximalParameterValue") : QVariant (aDrawer->HasOwnMaximalParameterValue());
    case 6: return isFirstColumn ? QVariant ("MaximalParameterValue") : QVariant (aDrawer->MaximalParameterValue());
    case 7: return isFirstColumn ? QVariant ("HasOwnMaximalChordialDeviation") : QVariant (aDrawer->HasOwnMaximalChordialDeviation());
    case 8: return isFirstColumn ? QVariant ("MaximalChordialDeviation") : QVariant (aDrawer->MaximalChordialDeviation());
    case 9: return isFirstColumn ? QVariant ("HasOwnTypeOfDeflection") : QVariant (aDrawer->HasOwnTypeOfDeflection());
    case 10: return isFirstColumn ? QVariant ("TypeOfDeflection")
                                  : QVariant (Aspect::TypeOfDeflectionToString (aDrawer->TypeOfDeflection()));
    case 11: return isFirstColumn ? QVariant ("HasOwnTypeOfHLR") : QVariant (aDrawer->HasOwnTypeOfHLR());
    case 12: return isFirstColumn ? QVariant ("TypeOfHLR")
                                  : QVariant (Prs3d::TypeOfHLRToString (aDrawer->TypeOfHLR()));

    case 13: return isFirstColumn ? QVariant ("HasOwnDeviationCoefficient") : QVariant (aDrawer->HasOwnDeviationCoefficient());
    case 14: return isFirstColumn ? QVariant ("DeviationCoefficient") : QVariant (aDrawer->DeviationCoefficient());
    case 15: return isFirstColumn ? QVariant ("HasOwnHLRDeviationCoefficient") : QVariant (aDrawer->HasOwnHLRDeviationCoefficient());
    case 16: return isFirstColumn ? QVariant ("HLRDeviationCoefficient") : QVariant (aDrawer->HLRDeviationCoefficient());

    case 17: return isFirstColumn ? QVariant ("HasOwnDeviationAngle") : QVariant (aDrawer->HasOwnDeviationAngle());
    case 18: return isFirstColumn ? QVariant ("DeviationAngle") : QVariant (aDrawer->DeviationAngle());
    case 19: return isFirstColumn ? QVariant ("PreviousDeviationAngle") : QVariant (aDrawer->PreviousDeviationAngle());

    case 20: return isFirstColumn ? QVariant ("HasOwnHLRDeviationAngle") : QVariant (aDrawer->HasOwnHLRDeviationAngle());
    case 21: return isFirstColumn ? QVariant ("HLRAngle") : QVariant (aDrawer->HLRAngle());
    case 22: return isFirstColumn ? QVariant ("PreviousHLRDeviationAngle") : QVariant (aDrawer->PreviousHLRDeviationAngle());

    case 23: return isFirstColumn ? QVariant ("HasOwnIsoOnPlane") : QVariant (aDrawer->HasOwnIsoOnPlane());
    case 24: return isFirstColumn ? QVariant ("IsoOnPlane") : QVariant (aDrawer->IsoOnPlane());
    case 25: return isFirstColumn ? QVariant ("HasOwnIsoOnTriangulation") : QVariant (aDrawer->HasOwnIsoOnTriangulation());
    case 26: return isFirstColumn ? QVariant ("IsoOnTriangulation") : QVariant (aDrawer->IsoOnTriangulation());
    case 27: return isFirstColumn ? QVariant ("HasOwnIsAutoTriangulation") : QVariant (aDrawer->HasOwnIsAutoTriangulation());
    case 28: return isFirstColumn ? QVariant ("IsAutoTriangulation") : QVariant (aDrawer->IsAutoTriangulation());
    case 29: return ViewControl_Table::SeparatorData(); // aspects separation

    case 30: return isFirstColumn ? QVariant ("HasOwnUIsoAspect") : QVariant (aDrawer->HasOwnUIsoAspect());
    case 31: return isFirstColumn ? QVariant ("HasOwnVIsoAspect") : QVariant (aDrawer->HasOwnVIsoAspect());
    case 32: return isFirstColumn ? QVariant ("HasOwnWireAspect") : QVariant (aDrawer->HasOwnWireAspect());
    case 33: return isFirstColumn ? QVariant ("HasOwnWireDraw") : QVariant (aDrawer->HasOwnWireDraw());
    case 34: return isFirstColumn ? QVariant ("WireDraw") : QVariant (aDrawer->WireDraw());
    case 35: return isFirstColumn ? QVariant ("HasOwnPointAspect") : QVariant (aDrawer->HasOwnPointAspect());
    case 36: return isFirstColumn ? QVariant ("HasOwnLineAspect") : QVariant (aDrawer->HasOwnLineAspect());
    case 37: return isFirstColumn ? QVariant ("HasOwnTextAspect") : QVariant (aDrawer->HasOwnTextAspect());
    case 38: return isFirstColumn ? QVariant ("HasOwnShadingAspect") : QVariant (aDrawer->HasOwnShadingAspect());
    case 39: return isFirstColumn ? QVariant ("HasOwnPlaneAspect") : QVariant (aDrawer->HasOwnPlaneAspect());
    case 40: return isFirstColumn ? QVariant ("HasOwnSeenLineAspect") : QVariant (aDrawer->HasOwnSeenLineAspect());

    case 41: return isFirstColumn ? QVariant ("HasOwnArrowAspect") : QVariant (aDrawer->HasOwnArrowAspect());
    case 42: return isFirstColumn ? QVariant ("HasOwnLineArrowDraw") : QVariant (aDrawer->HasOwnLineArrowDraw());
    case 43: return isFirstColumn ? QVariant ("LineArrowDraw") : QVariant (aDrawer->LineArrowDraw());

    case 44: return isFirstColumn ? QVariant ("HasOwnHiddenLineAspect") : QVariant (aDrawer->HasOwnHiddenLineAspect());
    case 45: return isFirstColumn ? QVariant ("HasOwnDrawHiddenLine") : QVariant (aDrawer->HasOwnDrawHiddenLine());
    case 46: return isFirstColumn ? QVariant ("DrawHiddenLine") : QVariant (aDrawer->DrawHiddenLine());

    case 47: return isFirstColumn ? QVariant ("HasOwnVectorAspect") : QVariant (aDrawer->HasOwnVectorAspect());
    case 48: return isFirstColumn ? QVariant ("VertexDrawMode")
                                  : QVariant (Prs3d::VertexDrawModeToString (aDrawer->VertexDrawMode()));

    case 49: return isFirstColumn ? QVariant ("HasOwnDatumAspect") : QVariant (aDrawer->HasOwnDatumAspect());

    case 50: return isFirstColumn ? QVariant ("HasOwnSectionAspect") : QVariant (aDrawer->HasOwnSectionAspect());

    case 51: return isFirstColumn ? QVariant ("HasOwnFreeBoundaryAspect") : QVariant (aDrawer->HasOwnFreeBoundaryAspect());
    case 52: return isFirstColumn ? QVariant ("HasOwnFreeBoundaryDraw") : QVariant (aDrawer->HasOwnFreeBoundaryDraw());
    case 53: return isFirstColumn ? QVariant ("FreeBoundaryDraw") : QVariant (aDrawer->FreeBoundaryDraw());

    case 54: return isFirstColumn ? QVariant ("HasOwnUnFreeBoundaryAspect") : QVariant (aDrawer->HasOwnUnFreeBoundaryAspect());
    case 55: return isFirstColumn ? QVariant ("HasOwnUnFreeBoundaryDraw") : QVariant (aDrawer->HasOwnUnFreeBoundaryDraw());
    case 56: return isFirstColumn ? QVariant ("UnFreeBoundaryDraw") : QVariant (aDrawer->UnFreeBoundaryDraw());

    case 57: return isFirstColumn ? QVariant ("HasOwnFaceBoundaryAspect") : QVariant (aDrawer->HasOwnFaceBoundaryAspect());
    case 58: return isFirstColumn ? QVariant ("HasOwnFaceBoundaryDraw") : QVariant (aDrawer->HasOwnFaceBoundaryDraw());
    case 59: return isFirstColumn ? QVariant ("FaceBoundaryDraw") : QVariant (aDrawer->FaceBoundaryDraw());

    case 60: return isFirstColumn ? QVariant ("HasOwnDimensionAspect") : QVariant (aDrawer->HasOwnDimensionAspect());

    case 61: return isFirstColumn ? QVariant ("HasOwnDimLengthModelUnits") : QVariant (aDrawer->HasOwnDimLengthModelUnits());
    case 62: return isFirstColumn ? QVariant ("DimLengthModelUnits") : QVariant (aDrawer->DimLengthModelUnits().ToCString());
    case 63: return isFirstColumn ? QVariant ("HasOwnDimLengthModelUnits") : QVariant (aDrawer->HasOwnDimLengthModelUnits());
    case 64: return isFirstColumn ? QVariant ("DimAngleModelUnits") : QVariant (aDrawer->DimAngleModelUnits().ToCString());

    case 65: return isFirstColumn ? QVariant ("HasOwnDimLengthDisplayUnits") : QVariant (aDrawer->HasOwnDimLengthDisplayUnits());
    case 66: return isFirstColumn ? QVariant ("DimLengthDisplayUnits") : QVariant (aDrawer->DimLengthDisplayUnits().ToCString());
    case 67: return isFirstColumn ? QVariant ("HasOwnDimAngleDisplayUnits") : QVariant (aDrawer->HasOwnDimAngleDisplayUnits());
    case 68: return isFirstColumn ? QVariant ("DimAngleDisplayUnits") : QVariant (aDrawer->DimAngleDisplayUnits().ToCString());

    // Graphic3d_PresentationAttributes
    case 69: return ViewControl_Table::SeparatorData();
    case 70: return isFirstColumn ? QVariant (STANDARD_TYPE (Graphic3d_PresentationAttributes)->Name())
                                 : ViewControl_Tools::GetPointerInfo (aDrawer).ToCString();
    case 72:
    {
      if (isFirstColumn) return QVariant ("ColorRGBA");
      else if (theRole == Qt::BackgroundRole) return ViewControl_ColorSelector::ColorToQColor (aDrawer->ColorRGBA());
      else if (theRole == Qt::DisplayRole) return ViewControl_ColorSelector::ColorToString (aDrawer->ColorRGBA());
    }
    case 71: return ViewControl_Table::SeparatorData();
    case 73: return isFirstColumn ? QVariant ("Method")
                                  : QVariant (Aspect::TypeOfHighlightMethodToString (aDrawer->Method()));
    case 74: return isFirstColumn ? QVariant ("ZLayer")
                                  : QVariant (Graphic3d::ZLayerIdToString (aDrawer->ZLayer()));
    case 75: return isFirstColumn ? QVariant ("DisplayMode") : QVariant (aDrawer->DisplayMode());


    default: return QVariant();
  }
}

// =======================================================================
// function : SetTableData
// purpose :
// =======================================================================
bool VInspector_ItemPrs3dDrawer::SetTableData (const int theRow, const int, const QVariant& theValue)
{
  Handle(Prs3d_Drawer) aDrawer = GetDrawer();
  if (aDrawer.IsNull())
    return false;

  switch (theRow)
  {
    case 4: aDrawer->SetDiscretisation (theValue.toInt()); break;
    case 6: aDrawer->SetMaximalParameterValue (theValue.toDouble()); break;
    case 8: aDrawer->SetMaximalChordialDeviation (theValue.toDouble()); break;
    case 10: aDrawer->SetTypeOfDeflection (Aspect::TypeOfDeflectionFromString (theValue.toString().toStdString().c_str())); break;
    case 12: aDrawer->SetTypeOfHLR (Prs3d::TypeOfHLRFromString (theValue.toString().toStdString().c_str())); break;
    case 13: aDrawer->SetDeviationCoefficient(); break;
    case 14: aDrawer->SetDeviationCoefficient (theValue.toDouble()); break;
    case 15: aDrawer->SetHLRDeviationCoefficient(); break;
    case 16: aDrawer->SetHLRDeviationCoefficient (theValue.toDouble()); break;
    case 18: aDrawer->SetDeviationAngle (theValue.toDouble()); break;
    case 21: aDrawer->SetHLRAngle (theValue.toDouble()); break;
    case 24: aDrawer->SetIsoOnPlane (theValue.toBool()); break;
    case 26: aDrawer->SetIsoOnTriangulation (theValue.toBool()); break;
    case 28: aDrawer->SetAutoTriangulation (theValue.toBool()); break;
    // aspects
    case 30:
    case 31:
    case 32:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 44:
    case 47:
    case 49:
    case 50:
    case 51:
    case 54:
    case 57:
    case 60:
    {
      Standard_Boolean isOwnAspect = (Standard_Boolean)theValue.toBool();
      if (!isOwnAspect)
        aDrawer->SetUIsoAspect (NULL);
      Handle(Prs3d_Drawer) aLink = aDrawer->Link(); // copy link drawer to restore it
      aDrawer->SetLink(NULL);
      // creates and sets default aspect
       // TODO: suggest to fill myHasOwnUIsoAspect in UIsoAspect
      if (theRow == 30) aDrawer->SetUIsoAspect (isOwnAspect ? aDrawer->UIsoAspect() : NULL);
      if (theRow == 31) aDrawer->SetVIsoAspect (isOwnAspect ? aDrawer->VIsoAspect() : NULL);
      if (theRow == 32) aDrawer->SetWireAspect (isOwnAspect ? aDrawer->WireAspect() : NULL);
      if (theRow == 35) aDrawer->SetPointAspect (isOwnAspect ? aDrawer->PointAspect() : NULL);
      if (theRow == 36) aDrawer->SetLineAspect (isOwnAspect ? aDrawer->LineAspect() : NULL);
      if (theRow == 37) aDrawer->SetTextAspect (isOwnAspect ? aDrawer->TextAspect() : NULL);
      if (theRow == 38) aDrawer->SetShadingAspect (isOwnAspect ? aDrawer->ShadingAspect() : NULL);
      if (theRow == 39) aDrawer->SetPlaneAspect (isOwnAspect ? aDrawer->PlaneAspect() : NULL);
      if (theRow == 40) aDrawer->SetSeenLineAspect (isOwnAspect ? aDrawer->SeenLineAspect() : NULL);
      if (theRow == 41) aDrawer->SetArrowAspect (isOwnAspect ? aDrawer->ArrowAspect() : NULL);
      if (theRow == 44) aDrawer->SetHiddenLineAspect (isOwnAspect ? aDrawer->HiddenLineAspect() : NULL);
      if (theRow == 47) aDrawer->SetVectorAspect (isOwnAspect ? aDrawer->VectorAspect() : NULL);
      if (theRow == 49) aDrawer->SetDatumAspect (isOwnAspect ? aDrawer->DatumAspect() : NULL);
      if (theRow == 50) aDrawer->SetSectionAspect (isOwnAspect ? aDrawer->SectionAspect() : NULL);
      if (theRow == 51) aDrawer->SetFreeBoundaryAspect (isOwnAspect ? aDrawer->FreeBoundaryAspect() : NULL);
      if (theRow == 54) aDrawer->SetUnFreeBoundaryAspect (isOwnAspect ? aDrawer->UnFreeBoundaryAspect() : NULL);
      if (theRow == 57) aDrawer->SetFaceBoundaryAspect (isOwnAspect ? aDrawer->FaceBoundaryAspect() : NULL);
      if (theRow == 60) aDrawer->SetDimensionAspect (isOwnAspect ? aDrawer->DimensionAspect() : NULL);

      aDrawer->SetLink(aLink);
    }
    break;
    case 34: aDrawer->SetWireDraw (theValue.toBool()); break;
    case 43: aDrawer->SetLineArrowDraw (theValue.toBool()); break;
    case 46: { if (theValue.toBool()) aDrawer->EnableDrawHiddenLine(); else aDrawer->DisableDrawHiddenLine(); } break;
    case 48: aDrawer->SetVertexDrawMode (Prs3d::VertexDrawModeFromString (theValue.toString().toStdString().c_str())); break;
    case 53: aDrawer->SetFreeBoundaryDraw (theValue.toBool()); break;
    case 56: aDrawer->SetUnFreeBoundaryDraw (theValue.toBool()); break;
    case 59: aDrawer->SetFaceBoundaryDraw (theValue.toBool()); break;

    case 62: aDrawer->SetDimLengthModelUnits (theValue.toString().toStdString().c_str()); break;
    case 64: aDrawer->SetDimAngleModelUnits (theValue.toString().toStdString().c_str()); break;
    case 66: aDrawer->SetDimLengthDisplayUnits (theValue.toString().toStdString().c_str()); break;
    case 68: aDrawer->SetDimAngleDisplayUnits (theValue.toString().toStdString().c_str()); break;

    case 72:
    {
      float anAlpha;
      aDrawer->SetColor (ViewControl_ColorSelector::StringToColor (theValue.toString(), anAlpha));
      aDrawer->SetTransparency (anAlpha);
    }
    case 73: aDrawer->SetMethod (Aspect::TypeOfHighlightMethodFromString (theValue.toString().toStdString().c_str())); break;
    case 74: aDrawer->SetZLayer (Graphic3d::ZLayerIdFromString (theValue.toString().toStdString().c_str())); break;
    case 75: aDrawer->SetDisplayMode (theValue.toInt()); break;

    default: return false;
  }
  return true;
}
