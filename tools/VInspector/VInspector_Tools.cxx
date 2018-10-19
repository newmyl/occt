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

#include <inspector/VInspector_Tools.hxx>

#include <inspector/ViewControl_TableModelValues.hxx>
#include <inspector/VInspector_ItemFolderObject.hxx>
#include <inspector/VInspector_TableModelValues.hxx>

#include <AIS_ListIteratorOfListOfInteractive.hxx>
#include <AIS_ListOfInteractive.hxx>
#include <Standard_Version.hxx>
#if OCC_VERSION_HEX < 0x060901
#include <AIS_LocalContext.hxx>
#endif
#include <AIS_Selection.hxx>
#include <AIS_Shape.hxx>
#include <AIS_Trihedron.hxx>
#include <BRep_Builder.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepTools.hxx>
#include <Graphic3d.hxx>
#include <Graphic3d_IndexBuffer.hxx>
#include <Graphic3d_Buffer.hxx>
#include <Graphic3d_BoundBuffer.hxx>

#include <SelectMgr_StateOfSelection.hxx>
#include <SelectMgr_TypeOfUpdate.hxx>
#include <SelectMgr_TypeOfBVHUpdate.hxx>
#include <Standard_Version.hxx>
#include <StdSelect_BRepOwner.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QStringList>
#include <Standard_WarningsRestore.hxx>

#include <sstream>

// =======================================================================
// function : GetShapeTypeInfo
// purpose :
// =======================================================================
TCollection_AsciiString VInspector_Tools::GetShapeTypeInfo (const TopAbs_ShapeEnum& theType)
{
  Standard_SStream aSStream;
  TopAbs::Print (theType, aSStream);
  return aSStream.str().c_str();
}

// =======================================================================
// function : GetPointerInfo
// purpose :
// =======================================================================
TCollection_AsciiString VInspector_Tools::GetPointerInfo (const Handle(Standard_Transient)& thePointer, const bool isShortInfo)
{
  std::ostringstream aPtrStr;
  aPtrStr << thePointer.operator->();
  if (!isShortInfo)
    return aPtrStr.str().c_str();

  TCollection_AsciiString anInfoPtr (aPtrStr.str().c_str());
  for (int aSymbolId = 1; aSymbolId < anInfoPtr.Length(); aSymbolId++)
  {
    if (anInfoPtr.Value(aSymbolId) != '0')
    {
      anInfoPtr = anInfoPtr.SubString(aSymbolId, anInfoPtr.Length());
      anInfoPtr.Prepend("0x");
      return anInfoPtr;
    }
  }
  return aPtrStr.str().c_str();
}

// =======================================================================
// function : SelectedOwners
// purpose :
// =======================================================================
int VInspector_Tools::SelectedOwners (const Handle(AIS_InteractiveContext)& theContext,
                                      const Handle(AIS_InteractiveObject)& theObject,
                                      const bool theShapeInfoOnly)
{
  QStringList anObjects;
  if (theContext.IsNull())
    return 0;

  QList<size_t> aSelectedIds; // Remember of selected address in order to avoid duplicates
  for (theContext->InitSelected(); theContext->MoreSelected(); theContext->NextSelected())
  {
    Handle(SelectMgr_EntityOwner) anOwner = theContext->SelectedOwner();
    if (anOwner.IsNull()) // TODO: check why it is possible
      continue;

    if (!theObject.IsNull())
    {
      Handle(AIS_InteractiveObject) anOwnerPresentation =
                                    Handle(AIS_InteractiveObject)::DownCast (anOwner->Selectable());
      if (anOwnerPresentation != theObject)
        continue;
    }
    Handle(StdSelect_BRepOwner) BROwnr = Handle(StdSelect_BRepOwner)::DownCast (anOwner);
    if (theShapeInfoOnly && BROwnr.IsNull())
      continue;

    Standard_Transient* anOwnerPtr = anOwner.operator->();
    if (aSelectedIds.contains ((size_t)anOwnerPtr))
      continue;
    aSelectedIds.append ((size_t)anOwnerPtr);

    anObjects.append (VInspector_Tools::GetPointerInfo (anOwnerPtr, true).ToCString());
  }
  return anObjects.size();
}

// =======================================================================
// function : IsOwnerSelected
// purpose :
// =======================================================================
bool VInspector_Tools::IsOwnerSelected (const Handle(AIS_InteractiveContext)& theContext,
                                        const Handle(SelectBasics_EntityOwner)& theOwner)
{
  bool anIsSelected = false;
  Handle(SelectMgr_EntityOwner) anOwner = Handle(SelectMgr_EntityOwner)::DownCast (theOwner);
  for (theContext->InitSelected(); theContext->MoreSelected() && !anIsSelected; theContext->NextSelected())
    anIsSelected = theContext->SelectedOwner() == anOwner;
  return anIsSelected;
}

// =======================================================================
// function : ContextOwners
// purpose :
// =======================================================================
NCollection_List<Handle(SelectBasics_EntityOwner)> VInspector_Tools::ContextOwners (
                                               const Handle(AIS_InteractiveContext)& theContext)
{
  NCollection_List<Handle(SelectBasics_EntityOwner)> aResultOwners;
  if (theContext.IsNull())
    return aResultOwners;

  AIS_ListOfInteractive aListOfIO;
  theContext->DisplayedObjects (aListOfIO);
  QList<size_t> aSelectedIds; // Remember of selected address in order to avoid duplicates
  for (AIS_ListIteratorOfListOfInteractive aIt(aListOfIO); aIt.More(); aIt.Next())
  {
    Handle(AIS_InteractiveObject) anIO = aIt.Value();
    if (anIO.IsNull())
      continue;
#if OCC_VERSION_HEX < 0x070201
    for (anIO->Init(); anIO->More(); anIO->Next())
    {
      Handle(SelectMgr_Selection) aSelection = anIO->CurrentSelection();
      if (aSelection.IsNull())
        continue;
      for (aSelection->Init(); aSelection->More(); aSelection->Next())
      {
        Handle(SelectMgr_SensitiveEntity) anEntity = aSelection->Sensitive();
#else
    for (SelectMgr_SequenceOfSelection::Iterator aSelIter (anIO->Selections()); aSelIter.More(); aSelIter.Next())
    {
      Handle(SelectMgr_Selection) aSelection = aSelIter.Value();
      if (aSelection.IsNull())
        continue;
      for (NCollection_Vector<Handle(SelectMgr_SensitiveEntity)>::Iterator aSelEntIter (aSelection->Entities()); aSelEntIter.More(); aSelEntIter.Next())
      {
        Handle(SelectMgr_SensitiveEntity) anEntity = aSelEntIter.Value();
#endif
        if (anEntity.IsNull())
          continue;
        const Handle(SelectBasics_SensitiveEntity)& aBase = anEntity->BaseSensitive();
        Handle(SelectBasics_EntityOwner) anOwner = aBase->OwnerId();
        Standard_Transient* anOwnerPtr = anOwner.operator->();
        if (aSelectedIds.contains ((size_t)anOwnerPtr))
          continue;
        aSelectedIds.append ((size_t)anOwnerPtr);
        aResultOwners.Append (anOwner);
      }
    }
  }
  return aResultOwners;
}

// =======================================================================
// function : ActiveOwners
// purpose :
// =======================================================================
NCollection_List<Handle(SelectBasics_EntityOwner)> VInspector_Tools::ActiveOwners (
                                const Handle(AIS_InteractiveContext)& theContext,
                                NCollection_List<Handle(SelectBasics_EntityOwner)>& theEmptySelectableOwners)
{
  NCollection_List<Handle(SelectBasics_EntityOwner)> aResultOwners;

  // only local context is processed: TODO for global context
  Handle(AIS_InteractiveContext) aContext = theContext;
  if (aContext.IsNull())
    return aResultOwners;
  NCollection_List<Handle(SelectBasics_EntityOwner)> anActiveOwners;
  // OCCT BUG:1 - equal pointer owners are appears in the list
#if OCC_VERSION_HEX > 0x060901
  aContext->MainSelector()->ActiveOwners (anActiveOwners);
#else
  anActiveOwners = aContext->MainSelector()->ActiveOwners();
#endif
  QList<size_t> aSelectedIds; // Remember of selected address in order to avoid duplicates
  Handle(SelectMgr_EntityOwner) anOwner;
  for (NCollection_List<Handle(SelectBasics_EntityOwner)>::Iterator anOwnersIt (anActiveOwners);
       anOwnersIt.More(); anOwnersIt.Next())
  {
    anOwner = Handle(SelectMgr_EntityOwner)::DownCast (anOwnersIt.Value());
    if (anOwner.IsNull())
      continue;

    Standard_Transient* anOwnerPtr = anOwner.operator->();
    if (aSelectedIds.contains ((size_t)anOwnerPtr))
      continue;
    aSelectedIds.append ((size_t)anOwnerPtr);

    aResultOwners.Append (anOwner);
    Handle(SelectMgr_SelectableObject) aSelectable = anOwner->Selectable();
    if (aSelectable.IsNull() ||
        !theContext->IsDisplayed(Handle(AIS_InteractiveObject)::DownCast (aSelectable)))
      theEmptySelectableOwners.Append (anOwner);
  }
  return aResultOwners;
}

// =======================================================================
// function : AddOrRemoveSelectedShapes
// purpose :
// =======================================================================
void VInspector_Tools::AddOrRemoveSelectedShapes (const Handle(AIS_InteractiveContext)& theContext,
                                                  const NCollection_List<Handle(SelectBasics_EntityOwner)>& theOwners)
{
  // TODO: the next two rows are to be removed later
  theContext->UnhilightSelected(false);
  theContext->ClearSelected(false);

  theContext->UnhilightSelected(Standard_False);

  //TODO: processing in local context only
#if OCC_VERSION_HEX < 0x060901
  Handle(AIS_LocalContext) aLContext = theContext->LocalContext();
  TCollection_AsciiString aSelectionName = aLContext->SelectionName();
  aLContext->UnhilightPicked(Standard_False);
#endif

  for (NCollection_List<Handle(SelectBasics_EntityOwner)>::Iterator anOwnersIt(theOwners);
       anOwnersIt.More(); anOwnersIt.Next())
  {
    Handle(SelectMgr_EntityOwner) anOwner = Handle(SelectMgr_EntityOwner)::DownCast (anOwnersIt.Value());
#if OCC_VERSION_HEX > 0x060901
    theContext->AddOrRemoveSelected (anOwner, Standard_False);
#else
    AIS_Selection::Selection(aSelectionName.ToCString())->Select(anOwner);
    anOwner->SetSelected(Standard_True);
#endif
  }
  theContext->UpdateCurrentViewer();
}

// =======================================================================
// function : AddOrRemovePresentations
// purpose :
// =======================================================================
void VInspector_Tools::AddOrRemovePresentations (const Handle(AIS_InteractiveContext)& theContext,
                                                 const NCollection_List<Handle(AIS_InteractiveObject)>& thePresentations)
{
  // TODO: the next two rows are to be removed later
  theContext->UnhilightSelected(false);
  theContext->ClearSelected(false);

  for (NCollection_List<Handle(AIS_InteractiveObject)>::Iterator anIOIt (thePresentations); anIOIt.More(); anIOIt.Next())
    theContext->AddOrRemoveSelected (anIOIt.Value(), false);

  theContext->UpdateCurrentViewer();
}

// =======================================================================
// function : GetInfo
// purpose :
// =======================================================================
QList<QVariant> VInspector_Tools::GetInfo (Handle(AIS_InteractiveObject)& theObject)
{
  QList<QVariant> anInfo;
  anInfo.append (theObject->DynamicType()->Name());
  anInfo.append (VInspector_Tools::GetPointerInfo (theObject, true).ToCString());

  Handle(AIS_Shape) aShapeIO = Handle(AIS_Shape)::DownCast (theObject);
  if (aShapeIO.IsNull())
    return anInfo;

  const TopoDS_Shape& aShape = aShapeIO->Shape();
  if (!aShape.IsNull())
    anInfo.append (VInspector_Tools::GetShapeTypeInfo (aShape.ShapeType()).ToCString());

  return anInfo;
}

// =======================================================================
// function : GetHighlightInfo
// purpose :
// =======================================================================
QList<QVariant> VInspector_Tools::GetHighlightInfo (const Handle(AIS_InteractiveContext)& theContext)
{
  QList<QVariant> aValues;
  if (theContext.IsNull())
    return aValues;

  QStringList aSelectedNames;
  QStringList aSelectedPointers;
  QStringList aSelectedTypes;
  QStringList aSelectedOwners;
  QList<size_t> aSelectedIds; // Remember of selected address in order to avoid duplicates
  for (theContext->InitDetected(); theContext->MoreDetected(); theContext->NextDetected())
  {
    Handle(SelectMgr_EntityOwner) anOwner = theContext->DetectedOwner();
    if (anOwner.IsNull())
      continue;
    Standard_Transient* anOwnerPtr = anOwner.operator->();
    if (aSelectedIds.contains ((size_t)anOwnerPtr))
      continue;
    aSelectedIds.append ((size_t)anOwnerPtr);
    Handle(AIS_InteractiveObject) anIO = Handle(AIS_InteractiveObject)::DownCast (anOwner->Selectable());
    if (anIO.IsNull())
      continue;
    QList<QVariant> anIOInfo = VInspector_Tools::GetInfo (anIO);
    if (anIOInfo.size() == 3)
    {
      aSelectedNames.append (anIOInfo[0].toString());
      aSelectedPointers.append (anIOInfo[1].toString());
      aSelectedTypes.append (anIOInfo[2].toString());
    }
    aSelectedOwners.append (VInspector_Tools::GetPointerInfo (anOwnerPtr, true).ToCString());
  }
  aValues.append (aSelectedNames.join (", "));
  aValues.append (aSelectedPointers.join (", "));
  aValues.append (aSelectedTypes.join (", "));
  aValues.append (aSelectedOwners.join (", "));

  return aValues;
}

// =======================================================================
// function : GetSelectedInfo
// purpose :
// =======================================================================
QList<QVariant> VInspector_Tools::GetSelectedInfo (const Handle(AIS_InteractiveContext)& theContext)
{
  QList<QVariant> aValues;
  if (theContext.IsNull())
    return aValues;

  QStringList aSelectedNames;
  QStringList aSelectedPointers;
  QStringList aSelectedTypes;
  QStringList aSelectedOwners;
  QList<size_t> aSelectedIds; // Remember of selected address in order to avoid duplicates
  for (theContext->InitSelected(); theContext->MoreSelected(); theContext->NextSelected())
  {
    Handle(SelectMgr_EntityOwner) anOwner = theContext->SelectedOwner();
    if (anOwner.IsNull())
      continue;
    Standard_Transient* anOwnerPtr = anOwner.operator->();
    if (aSelectedIds.contains ((size_t)anOwnerPtr))
      continue;
    aSelectedIds.append ((size_t)anOwnerPtr);
    Handle(AIS_InteractiveObject) anIO = Handle(AIS_InteractiveObject)::DownCast (anOwner->Selectable());
    if (anIO.IsNull())
      continue;

    QList<QVariant> anIOInfo = VInspector_Tools::GetInfo (anIO);
    if (anIOInfo.size() == 3)
    {
      aSelectedNames.append (anIOInfo[0].toString());
      aSelectedPointers.append (anIOInfo[1].toString());
      aSelectedTypes.append (anIOInfo[2].toString());
    }
    aSelectedOwners.append (VInspector_Tools::GetPointerInfo (anOwnerPtr, true).ToCString());
  }
  aValues.append (aSelectedNames.join (", "));
  aValues.append (aSelectedPointers.join (", "));
  aValues.append (aSelectedTypes.join (", "));
  aValues.append (aSelectedOwners.join (", "));
  return aValues;
}

// =======================================================================
// function : GetSelectedInfoPointers
// purpose :
// =======================================================================
QString VInspector_Tools::GetSelectedInfoPointers (const Handle(AIS_InteractiveContext)& theContext)
{
  QList<QVariant> aSelectedInfo = VInspector_Tools::GetSelectedInfo (theContext);
  return aSelectedInfo.size() > 2 ? aSelectedInfo[1].toString() : QString();
}

// =======================================================================
// function : ToName
// purpose :
// =======================================================================
TCollection_AsciiString VInspector_Tools::ToName (const VInspector_SelectionType theType, const int theValue)
{
  switch (theType)
  {
    case VInspector_SelectionType_TypeOfUpdate:
    {
      switch (theValue)
      {
        case SelectMgr_TOU_Full:    return "Full";
        case SelectMgr_TOU_Partial: return "Partial";
        case SelectMgr_TOU_None:    return "None";
        default: break;
      }
    }
    break;
    case VInspector_SelectionType_StateOfSelection:
    {
      switch (theValue)
      {
        case SelectMgr_SOS_Activated:   return "Activated";
        case SelectMgr_SOS_Deactivated: return "Deactivated";
        case SelectMgr_SOS_Sleeping:    return "Sleeping";
        case SelectMgr_SOS_Any:         return "Any";
        case SelectMgr_SOS_Unknown:     return "Unknown";
        default: break;
      }
    }
    break;
    case VInspector_SelectionType_TypeOfBVHUpdate:
    {
      switch (theValue)
      {
        case SelectMgr_TBU_Add:        return "Add";
        case SelectMgr_TBU_Remove:     return "Remove";
        case SelectMgr_TBU_Renew:      return "Renew";
        case SelectMgr_TBU_Invalidate: return "Invalidate";
        case SelectMgr_TBU_None:       return "None";
        default: break;
      }
    }
    default: break;
  }
  return "";
}

// =======================================================================
// function : SelectionModeToName
// purpose :
// =======================================================================
TCollection_AsciiString VInspector_Tools::SelectionModeToName (int theMode, const Handle(AIS_InteractiveObject)& thePresentation)
{
  // types are obtained by comment of SelectMgr_Selection class;
  Handle(AIS_Shape) aShapePresentation = Handle(AIS_Shape)::DownCast (thePresentation);
  if (!aShapePresentation.IsNull())
    return VInspector_Tools::GetShapeTypeInfo (AIS_Shape::SelectionType (theMode)).ToCString();
  else
  {
    Handle(AIS_Trihedron) aTrihedronPresentation = Handle(AIS_Trihedron)::DownCast (thePresentation);
    if (!aTrihedronPresentation.IsNull())
    {
      switch (theMode)
      {
        case 0: return "Trihedron";
        case 1: return "Trihedron Origin";
        case 2: return "Trihedron Axes";
        case 3: return "Trihedron Planes";
        default: break;
      }
    }
  }
  return TCollection_AsciiString (theMode);
}

// =======================================================================
// function : OrientationToName
// purpose :
// =======================================================================
TCollection_AsciiString VInspector_Tools::OrientationToName (const TopAbs_Orientation& theOrientation)
{
  Standard_SStream aSStream;
  TopAbs::Print(theOrientation, aSStream);
  return aSStream.str().c_str();
}

// =======================================================================
// function : ReadShape
// purpose :
// =======================================================================
TopoDS_Shape VInspector_Tools::ReadShape (const TCollection_AsciiString& theFileName)
{
  TopoDS_Shape aShape;

  BRep_Builder aBuilder;
  BRepTools::Read (aShape, theFileName.ToCString(), aBuilder);

  return aShape;
}

// =======================================================================
// function : GetPropertyTableValues
// purpose :
// =======================================================================
void VInspector_Tools::GetPropertyTableValues (const TreeModel_ItemBasePtr& theItem,
                                               QList<ViewControl_TableModelValues*>& theTableValues)
{
  TreeModel_ItemBasePtr anItem = theItem;
  VInspector_ItemFolderObjectPtr aFolderItem = itemDynamicCast<VInspector_ItemFolderObject>(anItem);
  if (aFolderItem)
  {
    VInspector_ItemFolderObject::ParentKind aParentKind = aFolderItem->GetParentItemKind();
    if (aParentKind == VInspector_ItemFolderObject::ParentKind_ContextItem ||
        aParentKind == VInspector_ItemFolderObject::ParentKind_PresentationItem)
      anItem = theItem->Parent();
  }

  theTableValues.append (new VInspector_TableModelValues (anItem));
}

namespace
{
  static Standard_CString VInspector_Table_PrintDisplayActionType[5] =
  {
    "None", "Display", "Redisplay", "Erase", "Remove"
  };
}

//=======================================================================
//function : DisplayActionTypeToString
//purpose  :
//=======================================================================
Standard_CString VInspector_Tools::DisplayActionTypeToString (VInspector_DisplayActionType theType)
{
  return VInspector_Table_PrintDisplayActionType[theType];
}

//=======================================================================
//function : DisplayActionTypeFromString
//purpose  :
//=======================================================================
Standard_Boolean VInspector_Tools::DisplayActionTypeFromString (Standard_CString theTypeString,
                                                                VInspector_DisplayActionType& theType)
{
  TCollection_AsciiString aName (theTypeString);
  for (Standard_Integer aTypeIter = 0; aTypeIter <= VInspector_DisplayActionType_RemoveId; ++aTypeIter)
  {
    Standard_CString aTypeName = VInspector_Table_PrintDisplayActionType[aTypeIter];
    if (aName == aTypeName)
    {
      theType = VInspector_DisplayActionType (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : ToVariant
//purpose  :
//=======================================================================
QVariant VInspector_Tools::ToVariant (const Select3D_BndBox3d& theBoundingBox)
{
  return QString ("(%1, %2, %3), (%4, %5, %6)")
    .arg (theBoundingBox.CornerMin().x()).arg (theBoundingBox.CornerMin().y()).arg (theBoundingBox.CornerMin().z())
    .arg (theBoundingBox.CornerMax().x()).arg (theBoundingBox.CornerMax().y()).arg (theBoundingBox.CornerMax().z());
}

//=======================================================================
//function : CreateShape
//purpose  :
//=======================================================================
TopoDS_Shape VInspector_Tools::CreateShape (const Bnd_Box& theBoundingBox)
{
  if (theBoundingBox.IsVoid() || theBoundingBox.IsWhole() ||
      theBoundingBox.IsXThin (Precision::Confusion()) ||
      theBoundingBox.IsYThin (Precision::Confusion()) ||
      theBoundingBox.IsZThin (Precision::Confusion()))
    return TopoDS_Shape();

  BRepPrimAPI_MakeBox aBoxBuilder(theBoundingBox.CornerMin(), theBoundingBox.CornerMax());
  return aBoxBuilder.Shape();
}

//=======================================================================
//function : ToVariant
//purpose  :
//=======================================================================
QVariant VInspector_Tools::ToVariant (const Handle(Graphic3d_IndexBuffer)& theIndexBuffer)
{
  const Handle(Graphic3d_Buffer)& aBuffer = theIndexBuffer;
  return VInspector_Tools::ToVariant (aBuffer);
}

//=======================================================================
//function : ToVariant
//purpose  :
//=======================================================================
QVariant VInspector_Tools::ToVariant (const Handle(Graphic3d_Buffer)& theBuffer)
{
  if (theBuffer.IsNull())
    return QVariant();

  QString anInfo;
  anInfo = "NbElements = " + QString::number (theBuffer->NbElements) + ",";
  anInfo = "NbAttributes = " + QString::number (theBuffer->NbAttributes) + ",";
  anInfo = "Stride = " + QString::number (theBuffer->Stride) + ",";
  QStringList anAttributes;
  for (Standard_Integer anAttribIter = 0; anAttribIter < theBuffer->NbAttributes; ++anAttribIter)
  {
    const Graphic3d_Attribute& anAttrib = theBuffer->Attribute (anAttribIter);
    anAttributes.append(VInspector_Tools::ToString (anAttrib));
  }
  return anInfo + " (" + anAttributes.join(", ") + ")";
}

//=======================================================================
//function : ToVariant
//purpose  :
//=======================================================================
QVariant VInspector_Tools::ToVariant (const Handle(Graphic3d_BoundBuffer)& theBoundBuffer)
{
  //const Handle(Graphic3d_Buffer)& aBuffer = theBoundBuffer;
  //Handle(Graphic3d_Buffer) aBuffer = Handle(Graphic3d_Buffer)::DownCast (theBoundBuffer);
  //return VInspector_Tools::ToVariant (aBuffer);
  return QVariant();
}

//=======================================================================
//function : ToString
//purpose  :
//=======================================================================
QString VInspector_Tools::ToString (const Graphic3d_Attribute& theAttribute)
{
  Graphic3d_TypeOfAttribute anId = theAttribute.Id;
  Graphic3d_TypeOfData aDataType = theAttribute.DataType;

  QString anInfo = Graphic3d::TypeOfAttributeToString (anId);
  anInfo += ": ";
  anInfo += Graphic3d::TypeOfDataToString (aDataType);

  return anInfo;
}
