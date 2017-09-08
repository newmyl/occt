// Created on: 2017-07-25
// Created by: Anastasia BOBYLEVA
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

#include <AIS_ViewCube.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_ManipulatorOwner.hxx>
#include <BRepLib_MakeVertex.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <BRepPrim_Wedge.hxx>
#include <BRepTools.hxx>
#include <ElCLib.hxx>
#include <gce_MakeDir.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <GeomAPI_IntCS.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_Transformation.hxx>
#include <NCollection_DataMap.hxx>
#include <Prs3d.hxx>
#include <Prs3d_Arrow.hxx>
#include <Prs3d_ArrowAspect.hxx>
#include <Prs3d_Root.hxx>
#include <Prs3d_ShadingAspect.hxx>
#include <Prs3d_Text.hxx>
#include <Prs3d_ToolDisk.hxx>
#include <Prs3d_ToolSphere.hxx>
#include <Select3D_SensitiveCircle.hxx>
#include <Select3D_SensitivePoint.hxx>
#include <Select3D_SensitiveSegment.hxx>
#include <Select3D_SensitiveTriangulation.hxx>
#include <Select3D_SensitivePrimitiveArray.hxx>
#include <SelectMgr_SequenceOfOwner.hxx>
#include <StdSelect_BRepOwner.hxx>
#include <StdSelect_BRepSelectionTool.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <V3d_View.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <StdPrs_ShadedShape.hxx>
#include <Graphic3d_ArrayOfPoints.hxx>

IMPLEMENT_STANDARD_RTTIEXT(AIS_ViewCube, AIS_InteractiveObject)
IMPLEMENT_STANDARD_RTTIEXT(AIS_ViewCubeFlat, AIS_ViewCube)

namespace
{
  const Standard_Real ROUND_INTERVAL = 0.2;

  const Standard_Integer SIDE_INDEX = 7;
  const Standard_Integer EDGE_INDEX = 19;
  const Standard_Integer VERTEX_INDEX = 27;
  const Standard_Integer ARROW_INDEX = 31;
  const Standard_Integer ROUND_ARROW_INDEX = 33;


}

//=======================================================================
//function : Constructor
//purpose  :
//=======================================================================
AIS_ViewCube::AIS_ViewCube()
: myPosition (Aspect_TOTP_LEFT_LOWER),
  myOffset (Graphic3d_Vec2i (150, 150)),
  mySize (100.0),
  myBoxPadding (20.0),
  myAxisPadding (10.0),
  myCornerRadius (0.1),
  myArrowPadding (0.0),
  myToDisplayEdges (Standard_True),
  myToDisplayVertices (Standard_True),
  myStartState (new Graphic3d_Camera),
  myEndState (new Graphic3d_Camera),
  myDuration (0.5),
  myIsAutoTransform (Standard_False)
{
  SetInfiniteState();
  SetMutable (Standard_True);

  myDrawer->SetArrowAspect (new Prs3d_ArrowAspect());
  myDrawer->SetTextAspect (new Prs3d_TextAspect());
  myDrawer->SetShadingAspect (new Prs3d_ShadingAspect());
  myArrowAspect = new Prs3d_ShadingAspect;
  myDynHilightDrawer = new Prs3d_Drawer;
  myDynHilightDrawer->SetShadingAspect (new Prs3d_ShadingAspect());


  setDefaultAttributes();
  setDefaultHighlightAttributes();

  // Set default size parameters that are stored in Drawer
  myDrawer->ArrowAspect()->SetLength (mySize * 0.25);
  myDrawer->TextAspect()->SetHeight (mySize * 0.2);


  // Create parts
  addPart (new Side (this, BRepPrim_YMin, TCollection_ExtendedString ("FRONT")),  gp::DY(), gp::DZ());
  addPart (new Side (this, BRepPrim_YMax, TCollection_ExtendedString ("BACK")),  -gp::DY(), gp::DZ());
  addPart (new Side (this, BRepPrim_XMin, TCollection_ExtendedString ("LEFT")),   gp::DX(), gp::DZ());
  addPart (new Side (this, BRepPrim_XMax, TCollection_ExtendedString ("RIGHT")), -gp::DX(), gp::DZ());
  addPart (new Side (this, BRepPrim_ZMin, TCollection_ExtendedString ("BOTTOM")), gp::DZ(), -gp::DY());
  addPart (new Side (this, BRepPrim_ZMax, TCollection_ExtendedString ("TOP")),   -gp::DZ(), gp::DY());

  for (Standard_Integer anX = 0; anX < 4; anX++)
    for (Standard_Integer anY = anX < 2 ? 2 : 4 ; anY < 6; anY++)
    {
      gp_Dir aDir (direction (anX) + direction (anY));
      gp_Dir anUp = Abs (aDir.Z()) < Precision::Confusion()
                    ? gp_Dir (0.0, 0.0, 1.0)
                    : (Abs (aDir.Y()) < Precision::Confusion()
                       ? gp_Dir (0.0, -sign (aDir.Z()), 0.0)
                       : gp_Dir (0.0, -1.0 * sign (aDir.Y() * aDir.Z()), 1.0) );
      addPart (new Edge (this, (BRepPrim_Direction) anX, (BRepPrim_Direction) anY), aDir, anUp);
    }

  for (Standard_Integer anX = 0; anX < 2; anX++)
    for (Standard_Integer anY = 2; anY < 4; anY++)
      for (Standard_Integer aZ = 4; aZ < 6; aZ++)
      {
        gp_Dir aDir (direction (anX) + direction (anY) + direction (aZ));
        gp_Dir anUp(-sign(aDir.X()) * sign (aDir.Z()), -sign (aDir.Y()) * sign (aDir.Z()), 2.0);
        addPart (new Vertex (this, (BRepPrim_Direction) anX, (BRepPrim_Direction) anY, (BRepPrim_Direction) aZ), aDir, anUp);
      }

  addPart (new FlatArrow (this), -M_PI_4, 0.0, 0.0, 9);
  addPart (new FlatArrow (this), 0.0, M_PI_4, 0.0, 9);
  addPart (new FlatArrow (this), M_PI_4, 0.0, 0.0, 9);
  addPart (new FlatArrow (this), 0.0, -M_PI_4, 0.0, 9);
  addPart (new RoundedArrow (this), 0.0, 0.0, M_PI_4, 9);
  addPart (new RoundedArrow (this), 0.0, 0.0, -M_PI_4, 9);

  SetZLayer (Graphic3d_ZLayerId_Topmost);
}

//=======================================================================
//function : Constructor
//purpose  :
//=======================================================================
AIS_ViewCube::AIS_ViewCube (const Handle(PrsMgr_PresentableObject)& theParent)
: myPosition (Aspect_TOTP_LEFT_LOWER),
  myOffset (Graphic3d_Vec2i (150, 150)),
  mySize (100.0),
  myBoxPadding (20.0),
  myAxisPadding (10.0),
  myCornerRadius (0.1),
  myArrowPadding (0.0),
  myToDisplayEdges (Standard_True),
  myToDisplayVertices (Standard_True),
  myDuration (0.5),
  myIsAutoTransform (Standard_False)
{
  // Do not add parts and camera state here

  SetInfiniteState();
  SetMutable (Standard_True);

  // Inherit aspects
  myDrawer = new Prs3d_Drawer;
  myDrawer->SetLink (theParent->Attributes());
  myDynHilightDrawer = new Prs3d_Drawer;
  myDynHilightDrawer->SetLink (theParent->DynamicHilightAttributes());
}

//=======================================================================
//function : setDefaultAttributes
//purpose  :
//=======================================================================
void AIS_ViewCube::setDefaultAttributes()
{
  myDrawer->TextAspect()->SetHorizontalJustification (Graphic3d_HTA_CENTER);
  myDrawer->TextAspect()->SetVerticalJustification (Graphic3d_VTA_CENTER);
  myDrawer->TextAspect()->SetColor (Quantity_NOC_BLACK);
  myDrawer->TextAspect()->SetFont (Font_NOF_GREEK_MONO);
  myDrawer->TextAspect()->SetHeight (16.0);

  myDrawer->ArrowAspect()->SetAngle (50.0 * M_PI / 180.0);
  myDrawer->ArrowAspect()->SetLength (10.0);

  Graphic3d_MaterialAspect aShadingMaterial;
  aShadingMaterial.SetMaterialType (Graphic3d_MATERIAL_ASPECT);
  aShadingMaterial.SetReflectionModeOff (Graphic3d_TOR_SPECULAR);
  aShadingMaterial.SetReflectionModeOff (Graphic3d_TOR_EMISSION);
  aShadingMaterial.SetReflectionModeOff (Graphic3d_TOR_DIFFUSE);
  aShadingMaterial.SetTransparency (0.0);

  Graphic3d_MaterialAspect aBackMaterial;
  aBackMaterial.SetReflectionModeOff (Graphic3d_TOR_SPECULAR);
  aBackMaterial.SetReflectionModeOff (Graphic3d_TOR_AMBIENT);
  aBackMaterial.SetReflectionModeOff (Graphic3d_TOR_EMISSION);
  aBackMaterial.SetReflectionModeOff (Graphic3d_TOR_DIFFUSE);
  aBackMaterial.SetMaterialType (Graphic3d_MATERIAL_ASPECT);
  aBackMaterial.SetTransparency (1.0);

  myDrawer->ShadingAspect()->Aspect()->SetInteriorStyle (Aspect_IS_SOLID);
  myDrawer->ShadingAspect()->SetMaterial (aShadingMaterial, Aspect_TOFM_FRONT_SIDE);
  myDrawer->ShadingAspect()->SetMaterial (aBackMaterial, Aspect_TOFM_BACK_SIDE);
  myDrawer->ShadingAspect()->SetColor (Quantity_NOC_WHITE, Aspect_TOFM_FRONT_SIDE);
  myDrawer->ShadingAspect()->SetColor (Quantity_NOC_GRAY40, Aspect_TOFM_BACK_SIDE);
  myDrawer->ShadingAspect()->Aspect()->SetPolygonOffsets (Aspect_POM_Fill, 5.0, 10.0);
  myDrawer->SetFaceBoundaryDraw (Standard_False);

  myArrowAspect->Aspect()->SetInteriorStyle (Aspect_IS_SOLID);
  myArrowAspect->SetMaterial (aShadingMaterial);
  myArrowAspect->SetColor (Quantity_NOC_WHITE);
  myArrowAspect->SetTransparency (0.5);
}

//=======================================================================
//function : setDefaultHighlightAttributes
//purpose  :
//=======================================================================
void AIS_ViewCube::setDefaultHighlightAttributes()
{
  Graphic3d_MaterialAspect aHighlightMaterial;
  aHighlightMaterial.SetReflectionModeOff (Graphic3d_TOR_AMBIENT);
  aHighlightMaterial.SetReflectionModeOff (Graphic3d_TOR_DIFFUSE);
  aHighlightMaterial.SetReflectionModeOff (Graphic3d_TOR_SPECULAR);
  aHighlightMaterial.SetReflectionModeOff (Graphic3d_TOR_EMISSION);
  aHighlightMaterial.SetMaterialType (Graphic3d_MATERIAL_ASPECT);
  myDynHilightDrawer->SetShadingAspect (new Prs3d_ShadingAspect);
  myDynHilightDrawer->ShadingAspect()->Aspect()->SetInteriorStyle (Aspect_IS_SOLID);
  myDynHilightDrawer->ShadingAspect()->SetMaterial (aHighlightMaterial);
  myDynHilightDrawer->ShadingAspect()->SetColor (Quantity_NOC_CYAN1);
  myDynHilightDrawer->SetZLayer (Graphic3d_ZLayerId_Topmost);
  myDynHilightDrawer->SetColor (Quantity_NOC_CYAN1);
}

//=======================================================================
//function : UnsetAttributes
//purpose  :
//=======================================================================
void AIS_ViewCube::UnsetAttributes()
{
  setDefaultAttributes();
  SetToUpdate();
}

//=======================================================================
//function : UnsetHilightAttributes
//purpose  :
//=======================================================================
void AIS_ViewCube::UnsetHilightAttributes()
{
  myHilightDrawer.Nullify();
  setDefaultHighlightAttributes();
  SetToUpdate();
}

//=======================================================================
//function : Reset
//purpose  :
//=======================================================================
void AIS_ViewCube::Reset()
{
  UnsetAttributes();
  UnsetHilightAttributes();

  mySize = 100.0;
  myBoxPadding = 20.0;
  myAxisPadding = 10.0;
  myCornerRadius = 0.1;
  myArrowPadding = 0.0;
  myToDisplayEdges = Standard_True;
  myToDisplayVertices = Standard_True;
  myDrawer->ArrowAspect()->SetLength (mySize * 0.25);
  myDrawer->TextAspect()->SetHeight (mySize * 0.16);
}

//=======================================================================
//function : SetPosition
//purpose  :
//=======================================================================
void AIS_ViewCube::Position (Aspect_TypeOfTriedronPosition& thePosition,
                             Graphic3d_Vec2i& theOffset)
{
  thePosition = myPosition;
  theOffset = myOffset;
}

//=======================================================================
//function : Position
//purpose  :
//=======================================================================
Graphic3d_Vec2i AIS_ViewCube::Position() const
{
  if (myView.IsNull())
  {
    return Graphic3d_Vec2i();
  }

  Standard_Integer aWidth = 0;
  Standard_Integer aHeight = 0;
  myView->Window()->Size (aWidth, aHeight);

  Graphic3d_Vec2i aPosition (aWidth / 2, aHeight / 2);
  if (myPosition & Aspect_TOTP_TOP)
  {
    aPosition.y() = myOffset.y();
  }
  else if (myPosition & Aspect_TOTP_BOTTOM)
  {
    aPosition.y() = aHeight - myOffset.y();
  }

  if (myPosition & Aspect_TOTP_LEFT)
  {
    aPosition.x() = myOffset.x();
  }
  else if (myPosition & Aspect_TOTP_RIGHT)
  {
    aPosition.x() = aWidth - myOffset.x();
  }
  return aPosition;
}

//=======================================================================
//function : SetPosition
//purpose  :
//=======================================================================
void AIS_ViewCube::SetPosition (const Aspect_TypeOfTriedronPosition thePosition,
                                const Standard_Integer theXOffset,
                                const Standard_Integer theYOffset)
{
  myPosition = thePosition;
  myOffset = Graphic3d_Vec2i (theXOffset, theYOffset);
}


//=======================================================================
//function : SetPosition
//purpose  :
//=======================================================================
Standard_Boolean AIS_ViewCube::SetPosition (const Graphic3d_Vec2i& thePosition,
                                            const Handle(V3d_View)& theView)
{
  if (theView.IsNull() || thePosition.x() < 0 || thePosition.y() < 0)
  {
    return Standard_False;
  }

  Standard_Integer aWidth = 0;
  Standard_Integer aHeight = 0;
  theView->Window()->Size (aWidth, aHeight);
  if (thePosition.x() > aWidth || thePosition.y() > aHeight)
  {
    return Standard_False;
  }

  Standard_Integer aMask = 0;
  Graphic3d_Vec2i anOffset (0, 0);

  if (thePosition.x() < aWidth / 2)
  {
    aMask |= Aspect_TOTP_LEFT;
    anOffset.x() = thePosition.x();
  }
  else if (thePosition.x() > aWidth / 2)
  {
    aMask |= Aspect_TOTP_RIGHT;
    anOffset.x() = aWidth - thePosition.x();
  }

  if (thePosition.y() > aHeight / 2)
  {
    aMask |= Aspect_TOTP_BOTTOM;
    anOffset.y() = aHeight - thePosition.y();
  }
  else if (thePosition.y() < aHeight / 2)
  {
    aMask |= Aspect_TOTP_TOP;
    anOffset.y() = thePosition.y();
  }

  myPosition = Aspect_TypeOfTriedronPosition (aMask);
  myOffset = anOffset;

  SetToUpdate();

  return Standard_True;
}

//=======================================================================
//function : SetPosition
//purpose  :
//=======================================================================
Standard_Boolean AIS_ViewCube::SetPosition (const Graphic3d_Vec2i& thePosition)
{
  return SetPosition (thePosition, myView);
}

//=======================================================================
//function : Size
//purpose  :
//=======================================================================
Standard_Real AIS_ViewCube::Size() const
{
  return mySize;
}

//=======================================================================
//function : SetSize
//purpose  :
//=======================================================================
void AIS_ViewCube::SetSize (const Standard_Real theValue, const Standard_Boolean theToAdaptAnother)
{
  if (Abs (mySize - theValue) < Precision::Confusion())
  {
    return;
  }
  mySize = theValue;
  if (theToAdaptAnother)
  {
    myBoxPadding = mySize * 0.2;
    myAxisPadding = mySize * 0.1;
    myDrawer->ArrowAspect()->SetLength (mySize * 0.25f);
    myDrawer->TextAspect()->SetHeight (mySize * 0.16);
  }
  SetToUpdate();
}

//=======================================================================
//function : SetBoxPadding
//purpose  :
//=======================================================================
void AIS_ViewCube::SetBoxPadding (const Standard_Real theValue)
{
  if (Abs (myBoxPadding - theValue) < Precision::Confusion())
  {
    return;
  }
  myBoxPadding = theValue;
  SetToUpdate();
}

//=======================================================================
//function : BoxPadding
//purpose  :
//=======================================================================
Standard_Real AIS_ViewCube::BoxPadding() const
{
  return myBoxPadding;
}

//=======================================================================
//function : SetAxisPadding
//purpose  :
//=======================================================================
void AIS_ViewCube::SetAxisPadding (const Standard_Real theValue)
{
  if (Abs (myAxisPadding - theValue) < Precision::Confusion())
  {
    return;
  }
  myAxisPadding = theValue;
  SetToUpdate();
}

//=======================================================================
//function : AxisPadding
//purpose  :
//=======================================================================
Standard_Real AIS_ViewCube::AxisPadding() const
{
  return myAxisPadding;
}

//=======================================================================
//function : SetCornerRadius
//purpose  :
//=======================================================================
void AIS_ViewCube::SetCornerRadius (const Standard_Real theValue)
{
  Standard_OutOfRange_Raise_if (theValue < 0.0 || theValue > 0.5,
                                "AIS_ViewCube::SetCornerRadius(): theValue should be in [0; 0.5]");
  if (Abs (myCornerRadius - theValue) < Precision::Confusion())
  {
    return;
  }
  myCornerRadius = theValue;
  SetToUpdate();
}

//=======================================================================
//function : CornerRadius
//purpose  :
//=======================================================================
Standard_Real AIS_ViewCube::CornerRadius() const
{
  return myCornerRadius;
}

//=======================================================================
//function : ToDrawVertices
//purpose  :
//=======================================================================
Standard_Boolean AIS_ViewCube::ToDrawVertices() const
{
  return myToDisplayVertices;
}

//=======================================================================
//function : SetDrawVertices
//purpose  :
//=======================================================================
void AIS_ViewCube::SetDrawVertices (const Standard_Boolean theValue)
{
  if (myToDisplayVertices == theValue)
  {
    return;
  }
  myToDisplayVertices = theValue;
  SetToUpdate();
}

//=======================================================================
//function : ToDrawEdges
//purpose  :
//=======================================================================
Standard_Boolean AIS_ViewCube::ToDrawEdges() const
{
  return myToDisplayEdges;
}

//=======================================================================
//function : SetDrawEdges
//purpose  :
//=======================================================================
void AIS_ViewCube::SetDrawEdges (const Standard_Boolean theValue)
{
  if (myToDisplayEdges == theValue)
  {
    return;
  }
  myToDisplayEdges = theValue;
  SetToUpdate();
}

//=======================================================================
//function : SetArrowAngle
//purpose  :
//=======================================================================
void AIS_ViewCube::SetArrowAngle (const Standard_Real theValue)
{
  if (Abs (myDrawer->ArrowAspect()->Angle() - theValue) < Precision::Angular())
  {
    return;
  }
  myDrawer->ArrowAspect()->SetAngle (theValue);
  SetToUpdate();
}

//=======================================================================
//function : ArrowAngle
//purpose  :
//=======================================================================
Standard_Real AIS_ViewCube::ArrowAngle() const
{
  return myDrawer->ArrowAspect()->Angle();
}

//=======================================================================
//function : SetArrowLength
//purpose  :
//=======================================================================
void AIS_ViewCube::SetArrowLength (const Standard_Real theValue)
{
  if (Abs (myDrawer->ArrowAspect()->Length() - theValue) < Precision::Confusion())
  {
    return;
  }
  myDrawer->ArrowAspect()->SetLength (theValue);
  SetToUpdate();
}

//=======================================================================
//function : ArrowLength
//purpose  :
//=======================================================================
Standard_Real AIS_ViewCube::ArrowLength() const
{
  return myDrawer->ArrowAspect()->Length();
}

//=======================================================================
//function : SetArrowPadding
//purpose  :
//=======================================================================
void AIS_ViewCube::SetArrowPadding (const Standard_Real theValue)
{
  if (Abs (myArrowPadding - theValue) < Precision::Confusion())
  {
    return;
  }
  myArrowPadding = theValue;
  SetToUpdate();
}

//=======================================================================
//function : ArrowPadding
//purpose  :
//=======================================================================
Standard_Real AIS_ViewCube::ArrowPadding() const
{
  return myArrowPadding;
}

//=======================================================================
//function : SetDuration
//purpose  :
//=======================================================================
void AIS_ViewCube::SetDuration (const Standard_Real theValue)
{
  if (Abs (myDuration - theValue) <Precision::Confusion())
  {
    return;
  }

  myDuration = theValue;
  SetToUpdate();
}

//=======================================================================
//function : Duration
//purpose  :
//=======================================================================
Standard_Real AIS_ViewCube::Duration() const
{
  return myDuration;
}

//=======================================================================
//function : SetTextColor
//purpose  :
//=======================================================================
void AIS_ViewCube::SetTextColor (const Quantity_Color& theColor)
{
  myDrawer->TextAspect()->SetColor (theColor);
  SetToUpdate();
}

//=======================================================================
//function : TextColor
//purpose  :
//=======================================================================
const Quantity_Color& AIS_ViewCube::TextColor() const
{
  return myDrawer->TextAspect()->Aspect()->Color();
}

//=======================================================================
//function : SetFont
//purpose  :
//=======================================================================
void AIS_ViewCube::SetFont (const TCollection_AsciiString& theFont)
{
  myDrawer->TextAspect()->Aspect()->SetFont (theFont);
  SetToUpdate();
}

//=======================================================================
//function : Font
//purpose  :
//=======================================================================
const TCollection_AsciiString& AIS_ViewCube::Font() const
{
  return myDrawer->TextAspect()->Aspect()->Font();
}

//=======================================================================
//function : SetFontHeight
//purpose  :
//=======================================================================
void AIS_ViewCube::SetFontHeight (const Standard_Real theValue)
{
  myDrawer->TextAspect()->SetHeight (theValue);
  SetToUpdate();
}

//=======================================================================
//function : FontHeight
//purpose  :
//=======================================================================
Standard_Real AIS_ViewCube::FontHeight() const
{
  return myDrawer->TextAspect()->Height();
}

//=======================================================================
//function : SetArrowColor
//purpose  :
//=======================================================================
void AIS_ViewCube::SetArrowColor (const Quantity_Color& theColor)
{
  if (myArrowAspect->Color().IsEqual (theColor))
  {
    return;
  }
  myArrowAspect->SetColor (theColor);
  SetToUpdate();
}

//=======================================================================
//function : ArrowColor
//purpose  :
//=======================================================================
const Quantity_Color& AIS_ViewCube::ArrowColor() const
{
  return myArrowAspect->Color();
}

//=======================================================================
//function : SetArrowTransparency
//purpose  :
//=======================================================================
void AIS_ViewCube::SetArrowTransparency (const Standard_Real theValue)
{
  if (Abs (myArrowAspect->Transparency() - theValue) < Precision::Confusion())
  {
    return;
  }
  myArrowAspect->SetTransparency (theValue);
  SetToUpdate();
}

//=======================================================================
//function : SetArrowTransparency
//purpose  :
//=======================================================================
Standard_Real AIS_ViewCube::ArrowTransparency() const
{
  return myArrowAspect->Transparency();
}

//=======================================================================
//function : SetInnerColor
//purpose  :
//=======================================================================
void AIS_ViewCube::SetInnerColor (const Quantity_Color& theColor)
{
  myDrawer->ShadingAspect()->SetColor (theColor, Aspect_TOFM_BACK_SIDE);
  SetToUpdate();
}

//=======================================================================
//function : InsideBoxColor
//purpose  :
//=======================================================================
const Quantity_Color& AIS_ViewCube::InnerColor() const
{
  return myDrawer->ShadingAspect()->Color (Aspect_TOFM_BACK_SIDE);
}

//=======================================================================
//function : SetBoxColor
//purpose  :
//=======================================================================
void AIS_ViewCube::SetBoxColor (const Quantity_Color& theColor)
{
  if (myDrawer->ShadingAspect()->Color().IsEqual (theColor))
  {
    return;
  }
  myDrawer->ShadingAspect()->SetColor (theColor, Aspect_TOFM_FRONT_SIDE);
  SetToUpdate();
}

//=======================================================================
//function : BoxColor
//purpose  :
//=======================================================================
const Quantity_Color& AIS_ViewCube::BoxColor() const
{
  return myDrawer->ShadingAspect()->Color (Aspect_TOFM_FRONT_SIDE);
}

//=======================================================================
//function : SetBoxTransparency
//purpose  :
//=======================================================================
void AIS_ViewCube::SetBoxTransparency (const Standard_Real theValue)
{
  if (Abs (myDrawer->ShadingAspect()->Transparency() - theValue) < Precision::Confusion())
  {
    return;
  }
  myDrawer->ShadingAspect()->SetTransparency (theValue);
  SetToUpdate();
}

//=======================================================================
//function : BoxTransparency
//purpose  :
//=======================================================================
Standard_Real AIS_ViewCube::BoxTransparency() const
{
  return myDrawer->ShadingAspect()->Transparency();
}

//=======================================================================
//function : SetColor
//purpose  :
//=======================================================================
void AIS_ViewCube::SetColor (const Quantity_Color& theColor)
{
  SetArrowColor (theColor);
  SetBoxColor (theColor);
}

//=======================================================================
//function : SetTransparency
//purpose  :
//=======================================================================
void AIS_ViewCube::SetTransparency (const Standard_Real theValue)
{
  SetArrowTransparency (theValue);
  SetBoxTransparency (theValue);
}

//=======================================================================
//function : SetTransformPersistence
//purpose  :
//=======================================================================
void AIS_ViewCube::SetTransformPersistence (const Handle(Graphic3d_TransformPers)& theTrsfPers)
{
  (void) theTrsfPers;
}

//=======================================================================
//function : setTransformPersistence
//purpose  : 
//=======================================================================
void AIS_ViewCube::setTransformPersistence (const Handle(Graphic3d_TransformPers)& theTrsfPers)
{
  AIS_InteractiveObject::SetTransformPersistence (theTrsfPers);
}

//=======================================================================
//function : setTransformPersistence
//purpose  : 
//=======================================================================
void AIS_ViewCube::AddChild (const Handle(PrsMgr_PresentableObject)& theObject)
{
  (void) theObject;
}

//=======================================================================
//function : setTransformPersistence
//purpose  : 
//=======================================================================
void AIS_ViewCube::RemoveChild (const Handle(PrsMgr_PresentableObject)& theObject)
{
  (void) theObject;
}

//=======================================================================
//function : addChild
//purpose  : 
//=======================================================================
void AIS_ViewCube::addChild (const Handle(PrsMgr_PresentableObject)& theObject)
{
  AIS_InteractiveObject::AddChild (theObject);
}

//=======================================================================
//function : addChild
//purpose  : 
//=======================================================================
void AIS_ViewCube::removeChild (const Handle(PrsMgr_PresentableObject)& theObject)
{
  AIS_InteractiveObject::RemoveChild (theObject);
}

//=======================================================================
//function : Compute
//purpose  :
//=======================================================================
void AIS_ViewCube::Compute (const Handle(PrsMgr_PresentationManager3d)& thePrsMgr,
                            const Handle(Prs3d_Presentation)& thePrs,
                            const Standard_Integer theMode)
{
  if (theMode != 0)
  {
    return;
  }

  // View cube is to be attached to view, otherwise it isn't displayed
  Standard_ProgramError_Raise_if (myView.IsNull(),
                                  "AIS_ViewCube::Compute() - view cube isn't attached to any view.");

  if (myFlatPart.IsNull())
  {
    myFlatPart = new AIS_ViewCubeFlat (this);
    addChild (myFlatPart);
    myFlatPart->SetContext (GetContext());
  }

  thePrs->SetMutable (Standard_True);
  thePrs->SetZLayer (ZLayer());
  Handle(Graphic3d_Group) aGroup;
  Handle(Prs3d_ShadingAspect) anAspect = new Prs3d_ShadingAspect();
  anAspect->Aspect()->SetInteriorStyle (Aspect_IS_SOLID);
  gp_Pnt aLocation = (mySize * 0.5 + myBoxPadding + myAxisPadding) * gp_XYZ (-1.0, -1.0, -1.0);

  setTransformPersistence (new Graphic3d_TransformPers (Graphic3d_TMF_TriedronPers, myPosition, myOffset));

  // Display axes
  // Create axis in the default coordinate system. The custom position is applied in local transformation.
  Axis anAxes[3];
  const Standard_Real aSize = mySize + 2 * myBoxPadding + myAxisPadding;
  anAxes[0] = Axis (gp_Ax1 (aLocation, gp::DX()), Quantity_NOC_RED, L'X', aSize);
  anAxes[1] = Axis (gp_Ax1 (aLocation, gp::DY()), Quantity_NOC_GREEN, L'Y', aSize);
  anAxes[2] = Axis (gp_Ax1 (aLocation, gp::DZ()), Quantity_NOC_BLUE1, L'Z', aSize);
  for (Standard_Integer anIt = 0; anIt < 3; ++anIt)
  {
    aGroup = Prs3d_Root::NewGroup (thePrs);
    Handle(Prs3d_ShadingAspect) anAspectAx = new Prs3d_ShadingAspect (new Graphic3d_AspectFillArea3d (*anAspect->Aspect()));
    anAspectAx->SetColor (anAxes[anIt].Color());
    aGroup->SetGroupPrimitivesAspect (anAspectAx->Aspect());
    Handle(Prs3d_TextAspect) aTextAspectAx = new Prs3d_TextAspect (new Graphic3d_AspectText3d (*myDrawer->TextAspect()->Aspect()));
    aTextAspectAx->SetColor (anAxes[anIt].Color());
    aTextAspectAx->SetHeight (myDrawer->TextAspect()->Height());
    anAxes[anIt].Compute (aGroup, anAspectAx, aTextAspectAx);
  }

  // Display center
  aGroup = Prs3d_Root::NewGroup (thePrs);
  Handle(Prs3d_ShadingAspect) anAspectCen = new Prs3d_ShadingAspect (new Graphic3d_AspectFillArea3d (*anAspect->Aspect()));
  anAspectCen->SetColor (Quantity_NOC_WHITE);
  aGroup->SetGroupPrimitivesAspect (anAspectCen->Aspect());
  Prs3d_ToolSphere aTool (4.0, 20, 20);
  gp_Trsf aTrsf;
  aTrsf.SetTranslation (gp_Vec (gp::Origin(), aLocation));
  Handle(Graphic3d_ArrayOfTriangles) aCenterArray;
  aTool.FillArray (aCenterArray, aTrsf);
  aGroup->AddPrimitiveArray (aCenterArray);

  // Display box
  aGroup = Prs3d_Root::NewGroup (thePrs);
  aGroup->SetGroupPrimitivesAspect (myDrawer->ShadingAspect()->Aspect());
  Standard_Integer anIt = 1;
  for (; anIt < SIDE_INDEX; anIt++)
  {
    Handle(Side) aPart = Handle(Side)::DownCast (myParts.ChangeFromIndex (anIt));
    aPart->Display (thePrsMgr, aGroup, myDrawer->TextAspect());
    aPart->SetTransformPersistence (TransformPersistence());
  }

  if (myToDisplayEdges)
  {
    const Standard_Real aThickness = myBoxPadding > 0 ? Sqrt (myBoxPadding * myBoxPadding + myBoxPadding * myBoxPadding) - 2.0 : 2.0;
    for (anIt = SIDE_INDEX; anIt < EDGE_INDEX; anIt++)
    {
      Handle(Edge) aPart = Handle(Edge)::DownCast (myParts.ChangeFromIndex (anIt));
      aPart->Display (thePrsMgr, aGroup, aThickness);
      aPart->SetTransformPersistence (TransformPersistence());
    }
  }

  if (myToDisplayVertices)
  {
    Standard_Real aRadius = myBoxPadding > 0 ? myBoxPadding * 0.5f / Cos (M_PI_4) : 2.0f;
    for (anIt = EDGE_INDEX; anIt < VERTEX_INDEX; anIt++)
    {
      Handle(Vertex) aPart = Handle(Vertex)::DownCast (myParts.ChangeFromIndex (anIt));
      aPart->Display (thePrsMgr, aGroup, aRadius);
      aPart->SetTransformPersistence (TransformPersistence());
    }
  }
}

//=======================================================================
//function : HasTransformation
//purpose  :
//=======================================================================
Standard_Boolean AIS_ViewCube::HasTransformation() const
{
  return !myAnimation->IsStopped();
}

//=======================================================================
//function : Add
//purpose  :
//=======================================================================
void AIS_ViewCube::AddTo (const Handle(AIS_InteractiveContext)& theContext,
                          const Handle(V3d_View)& theView)
{
  SetView (theView);

  theContext->Display (this, 0, 0, Standard_False);
  SetViewAffinity (theView);
}

//=======================================================================
//function : SetViewAffinity
//purpose  :
//=======================================================================
void AIS_ViewCube::SetViewAffinity (const Handle(V3d_View)& theView)
{
  const Handle(AIS_InteractiveContext)& aContext = GetContext();
  if (aContext.IsNull())
  {
    return;
  }

  // Set view affinity for child object
  myFlatPart->Presentation()->CStructure()->ViewAffinity = new Graphic3d_ViewAffinity;
  myFlatPart->Presentation()->CStructure()->ViewAffinity->SetVisible (Standard_False);

  Handle(Graphic3d_ViewAffinity) anAffinity = aContext->CurrentViewer()->StructureManager()->ObjectAffinity (this);
  anAffinity->SetVisible (Standard_False);

  // View Affinity should be applied after Display
  /*for (V3d_ListOfViewIterator aViewIter (aContext->CurrentViewer()->DefinedViewIterator()); aViewIter.More(); aViewIter.Next())
  {
    aContext->SetViewAffinity (this, aViewIter.Value(), Standard_False);
  }
  aContext->SetViewAffinity (this, theView, Standard_True);
  */

  anAffinity->SetVisible (theView->View()->Identification(), Standard_True);
  myFlatPart->Presentation()->CStructure()->ViewAffinity->SetVisible (theView->View()->Identification(), Standard_True);

}

//=======================================================================
//function : Hide
//purpose  :
//=======================================================================
void AIS_ViewCube::Hide()
{
  GetContext()->SetViewAffinity (this, myView, Standard_False);
  GetContext()->SetViewAffinity (myFlatPart, myView, Standard_False);
}

//=======================================================================
//function : Show
//purpose  :
//=======================================================================
void AIS_ViewCube::Show()
{
  GetContext()->SetViewAffinity (this, myView, Standard_True);
  GetContext()->SetViewAffinity (myFlatPart, myView, Standard_True);
}

//=======================================================================
//function : SetView
//purpose  :
//=======================================================================
void AIS_ViewCube::SetView (const Handle(V3d_View)& theView)
{
  myView = theView;
  myAnimation = new AIS_AnimationCamera ("ViewCube", theView);
}

//=======================================================================
//function : View
//purpose  :
//=======================================================================
const Handle(V3d_View)& AIS_ViewCube::View() const
{
  return myView;
}

//=======================================================================
//function : Transform
//purpose  :
//=======================================================================
void AIS_ViewCube::StartTransform (const Handle(SelectMgr_EntityOwner)& theOwner)
{
  // Make camera changings
  if (theOwner.IsNull())
  {
    return;
  }

  myStartState->Copy (myView->Camera());
  myEndState->Copy (myView->Camera());

  const Handle(CameraState)& aState = myStates.FindFromKey (theOwner);
  aState->FillCamera (myView, myEndState);

  myAnimation->SetCameraStart (myStartState);
  myAnimation->SetCameraEnd (myEndState);
  myAnimation->SetStartPts (0.0);
  myAnimation->SetOwnDuration (myDuration);
  myAnimation->StartTimer (0.0, 1.0, Standard_True, Standard_False);
}

//=======================================================================
//function : transform
//purpose  :
//=======================================================================
Standard_Boolean AIS_ViewCube::transform()
{
  const Standard_Real aPts = myAnimation->UpdateTimer();

  if (aPts >= myDuration)
  {
    myAnimation->Stop();
    onFinishTransformation();
    return Standard_False;
  }

  return Standard_True;
}

//=======================================================================
//function : Transform
//purpose  :
//=======================================================================
Standard_Boolean AIS_ViewCube::Transform (const Standard_Boolean theToUpdate)
{
  if (!HasTransformation())
  {
    return Standard_False;
  }

  if (!transform())
  {
    return Standard_False;
  }

  if (theToUpdate)
  {
    myView->IsInvalidated() ? myView->Redraw() : myView->RedrawImmediate();
  }

  onAfterTransform();
  return Standard_True;
}

//=======================================================================
//function : Transform
//purpose  :
//=======================================================================
void AIS_ViewCube::Transform (const Handle(SelectMgr_EntityOwner)& theOwner)
{
  StartTransform (theOwner);
  while (HasTransformation())
  {
    if (!transform())
    {
      return;
    }

    myView->IsInvalidated() ? myView->Redraw() : myView->RedrawImmediate();
    onAfterTransform();
  }
}

//=======================================================================
//function : SetAutoTransform
//purpose  :
//=======================================================================
void AIS_ViewCube::SetAutoTransform (const Standard_Boolean theValue)
{
  myIsAutoTransform = theValue;
}

//=======================================================================
//function : IsAutoTransform
//purpose  :
//=======================================================================
Standard_Boolean AIS_ViewCube::IsAutoTransform() const
{
  return myIsAutoTransform;
}

//=======================================================================
//function : addPart
//purpose  :
//=======================================================================
Standard_Integer AIS_ViewCube::addPart (const Handle(Part)& thePart,
                                        const gp_Dir& theDir, const gp_Dir& theUp,
                                        const Standard_Integer thePriority)
{
  Handle(SelectMgr_EntityOwner) anOwner = new AIS_ViewCubeOwner (this, thePriority);
  myStates.Add (anOwner, new CameraStateReplace (theDir, theUp));
  return myParts.Add (anOwner, thePart);
}

//=======================================================================
//function : addPart
//purpose  :
//=======================================================================
Standard_Integer AIS_ViewCube::addPart (const Handle(Part)& thePart,
                                        const Standard_Real theAngleX, const Standard_Real theAngleY, const Standard_Real theAngleZ,
                                        const Standard_Integer thePriority)
{
  Handle(SelectMgr_EntityOwner) anOwner = new AIS_ViewCubeOwner (this, thePriority);
  myStates.Add (anOwner, new CameraStateRotate (theAngleX, theAngleY, theAngleZ));
  return myParts.Add (anOwner, thePart);
}

// =======================================================================
// function : direction
// purpose  :
// =======================================================================
gp_XYZ AIS_ViewCube::direction (const Standard_Integer theDir)
{
  BRepPrim_Direction aDir = (BRepPrim_Direction)theDir;
  switch (aDir)
  {
    case BRepPrim_XMax:
    {
      return gp::DX().XYZ().Reversed();
    }
    case BRepPrim_XMin:
    {
      return gp::DX().XYZ();
    }
    case BRepPrim_YMax:
    {
      return gp::DY().XYZ().Reversed();
    }
    case BRepPrim_YMin:
    {
      return gp::DY().XYZ();
    }
    case BRepPrim_ZMax:
    {
      return gp::DZ().XYZ().Reversed();
    }
    case BRepPrim_ZMin:
    {
      return gp::DZ().XYZ();
    }
  }
  return gp_XYZ();
}

// =======================================================================
// function : sign
// purpose  :
// =======================================================================
Standard_Real AIS_ViewCube::sign (const Standard_Real theValue)
{
  return theValue > 0.0 ? 1.0 : -1.0;
}

//=======================================================================
//function : setLocalTransformation
//purpose  :
//=======================================================================
void AIS_ViewCube::setLocalTransformation (const Handle(Geom_Transformation)& /*theTrsf*/)
{
  Standard_ASSERT_INVOKE ("AIS_ViewCube::setLocalTransformation: "
                          "Custom transformation is not supported by this class");
}

//=======================================================================
//function : HilightOwnerWithColor
//purpose  :
//=======================================================================
void AIS_ViewCube::HilightOwnerWithColor (const Handle(PrsMgr_PresentationManager3d)& thePM,
                                          const Handle(Prs3d_Drawer)& theStyle,
                                          const Handle(SelectMgr_EntityOwner)& theOwner)
{
  if (theOwner.IsNull())
  {
    return;
  }

  const Handle(Part)& aPart = myParts.FindFromKey (theOwner);
  const Handle(Prs3d_Presentation)& aPresentation = aPart->HighlightPresentation();
  if (aPresentation.IsNull())
  {
    return;
  }

  // Manage view affinity if it is enabled for object
  Handle(Graphic3d_ViewAffinity) anAffinity = GetContext()->CurrentViewer()->StructureManager()->ObjectAffinity (this);
  if (anAffinity->IsVisible (View()->View()->Identification()))
  {
    if (aPresentation->CStructure()->ViewAffinity.IsNull())
    {
      aPresentation->CStructure()->ViewAffinity = new Graphic3d_ViewAffinity;
    }
    aPresentation->CStructure()->ViewAffinity->SetVisible (Standard_False);
    aPresentation->CStructure()->ViewAffinity->SetVisible (View()->View()->Identification(), Standard_True);
  }

  aPresentation->Highlight (theStyle);
  for (Graphic3d_SequenceOfGroup::Iterator aGroupIter (aPresentation->Groups());
       aGroupIter.More(); aGroupIter.Next())
  {
    Handle(Graphic3d_Group)& aGrp = aGroupIter.ChangeValue();
    if (!aGrp.IsNull() && aGrp->IsGroupPrimitivesAspectSet (Graphic3d_ASPECT_FILL_AREA))
    {
      aGrp->SetGroupPrimitivesAspect (myDynHilightDrawer->ShadingAspect()->Aspect());
    }
  }
  aPresentation->SetZLayer (theStyle->ZLayer());
  thePM->AddToImmediateList (aPresentation);
}

//=======================================================================
//function : HilightSelected
//purpose  :
//=======================================================================
void AIS_ViewCube::HilightSelected (const Handle(PrsMgr_PresentationManager3d)& thePM,
                                    const SelectMgr_SequenceOfOwner& theSeq)
{
  (void) thePM;

  if (!myIsAutoTransform)
  {
    return;
  }
  Transform (theSeq (1));
}

//=======================================================================
//function : HilightOwnerWithColor
//purpose  :
//=======================================================================
void AIS_ViewCube::ComputeSelection (const Handle(SelectMgr_Selection)& theSelection,
                                      const Standard_Integer theMode)
{
  if (theMode != 0)
  {
    return;
  }

  theSelection->Clear();
  Standard_Integer anIt = 1;
  for (; anIt < SIDE_INDEX; anIt++)
  {
    const Handle(SelectMgr_EntityOwner)& anOwner = myStates.FindKey (anIt);
    myParts.FindFromKey (anOwner)->ComputeSelection (anOwner, theSelection);
  }

  if (myToDisplayEdges)
  {
    for (anIt = SIDE_INDEX; anIt < EDGE_INDEX; anIt++)
    {
      const Handle(SelectMgr_EntityOwner)& anOwner = myStates.FindKey (anIt);
      myParts.FindFromKey (anOwner)->ComputeSelection (anOwner, theSelection);
    }
  }

  if (myToDisplayVertices)
  {
    for (anIt = EDGE_INDEX; anIt < VERTEX_INDEX; anIt++)
    {
      const Handle(SelectMgr_EntityOwner)& anOwner = myStates.FindKey (anIt);
      myParts.FindFromKey (anOwner)->ComputeSelection (anOwner, theSelection);
    }
  }

  // Compute selection of flat part
  if (!myFlatPart->HasSelection (theMode))
  {
    myFlatPart->RecomputePrimitives (theMode);
  }
  Handle(SelectMgr_Selection) aSelection = new SelectMgr_Selection (theMode);
  myFlatPart->ComputeSelection (aSelection, theMode);
}

// =======================================================================
// class    : CameraStateReplace
// function : FillCamera
// purpose  :
// =======================================================================
void AIS_ViewCube::CameraStateReplace::FillCamera (const Handle(V3d_View)& theView, const Handle(Graphic3d_Camera)& theCamera)
{
  Handle(Graphic3d_Camera) aBackupCamera = new Graphic3d_Camera (theView->Camera());
  theCamera->SetDirection (Direction);
  theCamera->SetUp (Up);

  theView->SetCamera (theCamera);
  theView->FitAll (0.01, Standard_False);
  theView->SetCamera (aBackupCamera);
}

// =======================================================================
// class    : CameraStateRotate
// function : FillCamera
// purpose  :
// =======================================================================
void AIS_ViewCube::CameraStateRotate::FillCamera (const Handle(V3d_View)& theView, const Handle(Graphic3d_Camera)& theCamera)
{
  // Do not use View for camera manipulation
  (void) theView;

  gp_Dir aBackDir (gp_Vec (theCamera->Center(), theCamera->Eye()));
  gp_Dir aXAxis (theCamera->Up().Crossed (aBackDir));
  gp_Dir aYAxis (aBackDir.Crossed (aXAxis));
  gp_Dir aZAxis (aXAxis.Crossed (aYAxis));

  gp_Trsf aRot[3], aTrsf;
  aRot[0].SetRotation (gp_Ax1 (theCamera->Center(), aYAxis), -AngleX);
  aRot[1].SetRotation (gp_Ax1 (theCamera->Center(), aXAxis), AngleY);
  aRot[2].SetRotation (gp_Ax1 (theCamera->Center(), aZAxis), AngleZ);
  aTrsf.Multiply (aRot[0]);
  aTrsf.Multiply (aRot[1]);
  aTrsf.Multiply (aRot[2]);

  theCamera->Transform (aTrsf);
}

// =======================================================================
// class    : Part
// function : Reset
// purpose  :
// =======================================================================
void AIS_ViewCube::Part::Reset()
{
  myTriangulations.Clear();
  if (!myHighlightPresentation.IsNull())
  {
    myHighlightPresentation->Clear();
  }
}

// =======================================================================
// class    : Part
// function : ComputeSelection
// purpose  :
// =======================================================================
void AIS_ViewCube::Part::ComputeSelection (const Handle(SelectMgr_EntityOwner)& theOwner,
                                           const Handle(SelectMgr_Selection)& theSelection)
{
  const NCollection_Sequence<Handle(Poly_Triangulation)>& aTris = Triangulations();
  for (NCollection_Sequence<Handle(Poly_Triangulation)>::Iterator aTriIt (aTris); aTriIt.More(); aTriIt.Next())
  {
    Handle(Select3D_SensitiveTriangulation) aTri = new Select3D_SensitiveTriangulation (theOwner, aTriIt.Value(), TopLoc_Location(), Standard_True);
    theSelection->Add (aTri);
  }
}

// =======================================================================
// class    : Part
// function : SetTransformPersistence
// purpose  :
// =======================================================================
void AIS_ViewCube::Part::SetTransformPersistence (const Handle(Graphic3d_TransformPers)& theTrsfPers)
{
  if (!myHighlightPresentation.IsNull())
  {
    myHighlightPresentation->SetTransformPersistence (theTrsfPers);
  }
}

// =======================================================================
// class    : Part
// function : Direction
// purpose  :
// =======================================================================
gp_XYZ AIS_ViewCube::Part::Direction (const BRepPrim_Direction theDir)
{
  switch (theDir)
  {
    case BRepPrim_XMax:
    {
      return gp::DX().XYZ();
    }
    case BRepPrim_XMin:
    {
      return gp::DX().XYZ().Reversed();
    }
    case BRepPrim_YMax:
    {
      return gp::DY().XYZ();
    }
    case BRepPrim_YMin:
    {
      return gp::DY().XYZ().Reversed();
    }
    case BRepPrim_ZMax:
    {
      return gp::DZ().XYZ();
    }
    case BRepPrim_ZMin:
    {
      return gp::DZ().XYZ().Reversed();
    }
  }
  return gp_XYZ();
}

//=======================================================================
//function : Location
//purpose  :
//=======================================================================
gp_XYZ AIS_ViewCube::Part::Location()
{
  const Standard_Real anOffset = myParent->Size() * 0.5 + myParent->BoxPadding();
  return gp_XYZ (-anOffset, -anOffset, -anOffset);
}

// =======================================================================
// function : NumberOfTriangleVerices
// purpose  :
// =======================================================================
Standard_Integer AIS_ViewCube::ToolRectangle::NumberOfTriangleVerices() const
{
   // Compute number of triangles
   return (myRadius > 0 ? 3 * ((Standard_Integer)(M_PI_2 / ROUND_INTERVAL + 1) * 4 + 6) : 6);
}

// =======================================================================
// function : FillCorner
// purpose  :
// =======================================================================
void AIS_ViewCube::ToolRectangle::FillCorner (const gp_Circ& theCircle,
                                               const Standard_Real theStartParameter,
                                               const Standard_Real theEndParameter,
                                               const Handle(Graphic3d_ArrayOfTriangles)& theArray)
{
  for (Standard_Real anIt = theStartParameter; anIt < theEndParameter; anIt += ROUND_INTERVAL)
  {
      theArray->AddVertex (ElCLib::Value (anIt, theCircle), theCircle.Position().Direction());
      theArray->AddVertex (ElCLib::Value (anIt + ROUND_INTERVAL, theCircle), theCircle.Position().Direction());
      theArray->AddVertex (theCircle.Location(), theCircle.Position().Direction());
  }
}

//=======================================================================
//class    : ToolRectangle
//function : FillArray
//purpose  :
//=======================================================================
void AIS_ViewCube::ToolRectangle::FillArray (Handle(Graphic3d_ArrayOfTriangles)& theArray,
                                                    Handle(Poly_Triangulation)& theTriangulation)
{
    theArray = new Graphic3d_ArrayOfTriangles (NumberOfTriangleVerices(), 0, Standard_True);
    const Standard_Real aCornerRadius = myRadius;
    gp_Ax2 aPosition = myPosition;
    if (aCornerRadius > 0.0f)
    {
        // Corners
        aPosition.SetLocation (gp_Pnt (myTopLeft.XYZ() + aPosition.XDirection().XYZ() * aCornerRadius - aPosition.YDirection().XYZ() * aCornerRadius));
        FillCorner (gp_Circ (aPosition, aCornerRadius), M_PI_2, M_PI, theArray);
        aPosition.SetLocation (gp_Pnt (myBottomLeft.XYZ() + aPosition.XDirection().XYZ() * aCornerRadius + aPosition.YDirection().XYZ() * aCornerRadius));
        FillCorner (gp_Circ (aPosition, aCornerRadius), M_PI, M_PI * 1.5, theArray);
        aPosition.SetLocation (gp_Pnt (myBottomRight.XYZ() - aPosition.XDirection().XYZ() * aCornerRadius + aPosition.YDirection().XYZ() * aCornerRadius));
        FillCorner (gp_Circ (aPosition, aCornerRadius), M_PI * 1.5, M_PI * 2, theArray);
        aPosition.SetLocation (gp_Pnt (myTopRight.XYZ() - aPosition.XDirection().XYZ() * aCornerRadius - aPosition.YDirection().XYZ() * aCornerRadius));
        FillCorner (gp_Circ (aPosition, aCornerRadius), 0, M_PI_2, theArray);

        // Side parts
        theArray->AddVertex (myTopLeft.XYZ() - aPosition.YDirection().XYZ() * aCornerRadius, aPosition.Direction());
        theArray->AddVertex (myBottomLeft.XYZ() + aPosition.YDirection().XYZ() * aCornerRadius, aPosition.Direction());
        theArray->AddVertex (myBottomLeft.XYZ() + aPosition.XDirection().XYZ() * aCornerRadius + aPosition.YDirection().XYZ() * aCornerRadius, aPosition.Direction());
        theArray->AddVertex (myBottomLeft.XYZ() + aPosition.XDirection().XYZ() * aCornerRadius + aPosition.YDirection().XYZ() * aCornerRadius, aPosition.Direction());
        theArray->AddVertex (myTopLeft.XYZ() + aPosition.XDirection().XYZ() * aCornerRadius - aPosition.YDirection().XYZ() * aCornerRadius, aPosition.Direction());
        theArray->AddVertex (myTopLeft.XYZ() - aPosition.YDirection().XYZ() * aCornerRadius, aPosition.Direction());
        theArray->AddVertex (myBottomRight.XYZ() - aPosition.XDirection().XYZ() * aCornerRadius + aPosition.YDirection().XYZ() * aCornerRadius, aPosition.Direction());
        theArray->AddVertex (myBottomRight.XYZ() + aPosition.YDirection().XYZ() * aCornerRadius, aPosition.Direction());
        theArray->AddVertex (myTopRight.XYZ() - aPosition.YDirection().XYZ() * aCornerRadius, aPosition.Direction());
        theArray->AddVertex (myTopRight.XYZ() - aPosition.YDirection().XYZ() * aCornerRadius, aPosition.Direction());
        theArray->AddVertex (myTopRight.XYZ() - aPosition.XDirection().XYZ() * aCornerRadius - aPosition.YDirection().XYZ() * aCornerRadius, aPosition.Direction());
        theArray->AddVertex (myBottomRight.XYZ() - aPosition.XDirection().XYZ() * aCornerRadius + aPosition.YDirection().XYZ() * aCornerRadius, aPosition.Direction());
    }

    theArray->AddVertex (myTopLeft.XYZ() + aPosition.XDirection().XYZ() * aCornerRadius, aPosition.Direction());
    theArray->AddVertex (myBottomLeft.XYZ() + aPosition.XDirection().XYZ() * aCornerRadius, aPosition.Direction());
    theArray->AddVertex (myBottomRight.XYZ() - aPosition.XDirection().XYZ() * aCornerRadius, aPosition.Direction());
    theArray->AddVertex (myTopLeft.XYZ() + aPosition.XDirection().XYZ() * aCornerRadius, aPosition.Direction());
    theArray->AddVertex (myBottomRight.XYZ() - aPosition.XDirection().XYZ() * aCornerRadius, aPosition.Direction());
    theArray->AddVertex (myTopRight.XYZ() - aPosition.XDirection().XYZ() * aCornerRadius, aPosition.Direction());

    theTriangulation = new Poly_Triangulation (4, 2, 0);
    theTriangulation->ChangeNodes().SetValue (1, myBottomLeft);
    theTriangulation->ChangeNodes().SetValue (2, myTopLeft);
    theTriangulation->ChangeNodes().SetValue (3, myTopRight);
    theTriangulation->ChangeNodes().SetValue (4, myBottomRight);
    theTriangulation->ChangeTriangles().SetValue (1, Poly_Triangle (1, 2, 3));
    theTriangulation->ChangeTriangles().SetValue (2, Poly_Triangle (1, 3, 4));
}

//=======================================================================
//class    : Side
//function : Init
//purpose  :
//=======================================================================
void AIS_ViewCube::Side::Display (const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                                   const Handle(Graphic3d_Group)& theGroup,
                                   const Handle(Prs3d_TextAspect)& theTextAspect)
{
  Reset();

  gp_Pnt aTopLeft, aTopRight, aBottomLeft, aBottomRight;
  const gp_Ax2& aSystem = gp::XOY();

  const Standard_Real aPadding = myParent->BoxPadding();
  const Standard_Real aSize = myParent->Size();
  const Standard_Real aCornerRadius = myParent->CornerRadius() * aSize;
  const gp_XYZ aLoc = Location();
  const gp_XYZ& anX = aSystem.XDirection().XYZ();
  const gp_XYZ& anY = aSystem.YDirection().XYZ();
  const gp_XYZ& aZ = aSystem.Direction().XYZ();
  gp_Ax2 aPosition;

  switch (myDirection)
  {
    case BRepPrim_XMax:
    {
      aPosition = gp_Ax2 (aLoc + anX * (aSize + 2 * aPadding) +  anY * aPadding + aZ * aPadding, anX, anY);
      break;
    }
    case BRepPrim_XMin:
    {
      aPosition = gp_Ax2 (aLoc + anY * (aSize + aPadding) + aZ * aPadding, anX * (-1), anY * (-1));
      break;
    }
    case BRepPrim_YMax:
    {
      aPosition = gp_Ax2 (aLoc + anX * (aSize + aPadding) + anY * (aSize + 2 * aPadding) + aZ * aPadding, anY, anX * (-1));
      break;
    }
    case BRepPrim_YMin:
    {
      aPosition = gp_Ax2 (aLoc + anX * aPadding + aZ * aPadding, anY * (-1), anX);
      break;
    }
    case BRepPrim_ZMax:
    {
      aPosition = gp_Ax2 (aLoc + anX * aPadding + anY * aPadding + aZ * (aSize + 2 * aPadding), aZ, anX);
      break;
    }
    case BRepPrim_ZMin:
    {
      aPosition = gp_Ax2 (aLoc + anX * aPadding + anY * (aSize + aPadding), aZ * (-1), anX);
      break;
    }
  }
  aBottomLeft = aPosition.Location();
  aTopLeft = aBottomLeft.XYZ() + aPosition.YDirection().XYZ() * aSize;
  aTopRight = aTopLeft.XYZ() + aPosition.XDirection().XYZ() * aSize;
  aBottomRight = aBottomLeft.XYZ() + aPosition.XDirection().XYZ() * aSize;
  const Standard_Real aCoef = aSize * 0.5;
  gp_Ax2 aTextPosition (aPosition.Translated (gp_Vec (aPosition.XDirection().XYZ() * aCoef + aPosition.YDirection().XYZ() * aCoef + aPosition.Direction().XYZ() * aSize * 0.02)));

  Handle(Graphic3d_ArrayOfTriangles) anArray;
  Handle(Poly_Triangulation) aTri;
  ToolRectangle aTool (aPosition, aBottomLeft, aTopLeft, aBottomRight, aTopRight, aCornerRadius);
  aTool.FillArray (anArray, aTri);
  theGroup->AddPrimitiveArray (anArray);

  Prs3d_Text::Draw (theGroup, theTextAspect, myText, aTextPosition);

  if (myHighlightPresentation.IsNull())
  {
    myHighlightPresentation = new Prs3d_Presentation (thePrsMgr->StructureManager());
  }
  Handle(Graphic3d_Group) aGroup = Prs3d_Root::CurrentGroup (myHighlightPresentation);
  aGroup->AddPrimitiveArray (anArray);

  myTriangulations.Append (aTri);
}

// =======================================================================
// class    : Vertex
// function : Display
// purpose  :
// =======================================================================
void AIS_ViewCube::Vertex::Display (const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                                     const Handle(Graphic3d_Group)& theGroup,
                                     const Standard_Real theRadius,
                                     const Standard_Integer theSlicesNb)
{
  Reset();

  gp_XYZ anX = Direction (myDirection1);
  gp_XYZ anY = Direction (myDirection2);
  gp_XYZ aZ = Direction (myDirection3);
  const gp_Ax2& aParentPosition = gp::XOY();
  const Standard_Real aPadding = myParent->BoxPadding();
  gp_Pnt aPos (Location() + (aParentPosition.XDirection().XYZ() * aPadding
                             + aParentPosition.YDirection().XYZ() * aPadding
                             + aParentPosition.Direction().XYZ() * aPadding)* 0.75);
  const Standard_Real aSize = myParent->Size() + aPadding * 0.5;
  gp_XYZ aT;
  if (myDirection1 == BRepPrim_XMax)
  {
    aT += aParentPosition.XDirection().XYZ() * aSize;
  }
  if (myDirection2 == BRepPrim_YMax)
  {
    aT += aParentPosition.YDirection().XYZ() * aSize;
  }
  if (myDirection3 == BRepPrim_ZMax)
  {
    aT += aParentPosition.Direction().XYZ() * aSize;
  }
  aPos.Translate (aT);

  gp_Ax2 aPosition (aPos, gp_Dir (anX + anY + aZ).Reversed());

  Prs3d_ToolDisk aTool (0.0, theRadius, theSlicesNb, 1);
  gp_Ax3 aSystem (aPosition);
  gp_Trsf aTrsf;
  aTrsf.SetTransformation (aSystem, gp_Ax3());
  Handle(Graphic3d_ArrayOfTriangles) anArray;
  Handle(Poly_Triangulation) aTri;
  aTool.FillArray (anArray, aTri, aTrsf);

  theGroup->AddPrimitiveArray (anArray);

  if (myHighlightPresentation.IsNull())
  {
    myHighlightPresentation = new Prs3d_Presentation (thePrsMgr->StructureManager());
  }

  Handle(Graphic3d_Group) aGroup = Prs3d_Root::CurrentGroup (myHighlightPresentation);
  aGroup->AddPrimitiveArray (anArray);

  myTriangulations.Append (aTri);
}

// =======================================================================
// class    : Edge
// function : Display
// purpose  :
// =======================================================================
void AIS_ViewCube::Edge::Display (const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                                   const Handle(Graphic3d_Group)& theGroup,
                                   const Standard_Real theThickness)
{
  Reset();

  gp_XYZ aDir1 = Direction (myDirection1);
  gp_XYZ aDir2 = Direction (myDirection2);
  const gp_Ax2& aParentPosition = gp::XOY();
  const Standard_Real aSize = (1 - 2 * myParent->CornerRadius()) * myParent->Size() * 0.5;
  if (aSize < Precision::Confusion())
  {
    return;
  }

  // Center of edge
  gp_XYZ aPos (aDir1 * (myParent->Size() + myParent->BoxPadding()) * 0.5 + aDir2 * (myParent->Size() + myParent->BoxPadding()) * 0.5);

  // Prepare vertices for edge rectangle
  gp_Dir aDir = aDir1 ^ aDir2;
  gp_Dir anX (aDir.IsParallel (aParentPosition.Direction(), Precision::Angular())
              ? aParentPosition.Direction()
              : (aDir.IsParallel(aParentPosition.XDirection(), Precision::Angular())
                  ? aParentPosition.XDirection()
                  : aParentPosition.YDirection()));
  gp_Dir aN (aDir1 + aDir2);
  gp_Dir anY (aN ^ anX);
  gp_Pnt aTopLeft, aTopRight, aBottomLeft, aBottomRight;
  aBottomLeft = aPos - anX.XYZ() * aSize - anY.XYZ() * theThickness * 0.5;
  aTopLeft = aPos - anX.XYZ() * aSize + anY.XYZ() * theThickness * 0.5;
  aTopRight = aPos + anX.XYZ() * aSize + anY.XYZ() * theThickness * 0.5;
  aBottomRight = aPos + anX.XYZ() * aSize - anY.XYZ() * theThickness * 0.5;

  //* Fill graphical structures
  Handle(Graphic3d_ArrayOfTriangles) anArray;
  Handle(Poly_Triangulation) aTri;
  ToolRectangle aTool (gp_Ax2 (gp::Origin(), aN, anX), aBottomLeft, aTopLeft, aBottomRight, aTopRight, theThickness * 0.5f);
  aTool.FillArray (anArray, aTri);

  theGroup->AddPrimitiveArray (anArray);
  if (myHighlightPresentation.IsNull())
  {
    myHighlightPresentation = new Prs3d_Presentation (thePrsMgr->StructureManager());
  }
  Handle(Graphic3d_Group) aGroup = Prs3d_Root::CurrentGroup (myHighlightPresentation);
  aGroup->AddPrimitiveArray (anArray);

  myTriangulations.Append (aTri);
}

//=======================================================================
//class    : Axis
//function : Constructor
//purpose  :
//=======================================================================
AIS_ViewCube::Axis::Axis (const gp_Ax1& theAxis,
                          const Quantity_Color& theColor,
                          const Standard_ExtCharacter& theSymbol,
                          const Standard_Real theLength)
: myPosition (theAxis),
  myColor (theColor),
  myLength (theLength),
  mySymbol (theSymbol)
{
  //
}

//=======================================================================
//class    : Axis
//function : Compute
//purpose  :
//=======================================================================
void AIS_ViewCube::Axis::Compute (const Handle(Graphic3d_Group)& theGroup,
                                  const Handle(Prs3d_ShadingAspect)& theAspect,
                                  const Handle(Prs3d_TextAspect)& theTextAspect)
{

  const Standard_Real anArrowLength   = 0.2 * myLength;
  Handle(Graphic3d_ArrayOfTriangles) aTriangleArray = Prs3d_Arrow::DrawShaded (myPosition, 1.0, myLength, 3.0, anArrowLength, 20);

  theGroup->SetGroupPrimitivesAspect (theAspect->Aspect());
  theGroup->AddPrimitiveArray (aTriangleArray);


  gp_Pnt aTextOrigin = myPosition.Location().Translated (gp_Vec (myPosition.Direction().X() * (myLength + anArrowLength),
                                                                 myPosition.Direction().Y() * (myLength + anArrowLength),
                                                                 myPosition.Direction().Z() * (myLength + anArrowLength)));
  Prs3d_Text::Draw (theGroup, theTextAspect, TCollection_ExtendedString (mySymbol), aTextOrigin);
}

//=======================================================================
//class    : ToolArrow
//function : FillArray
//purpose  :
//=======================================================================
void AIS_ViewCube::ToolArrow::FillArray (Handle(Graphic3d_ArrayOfTriangles)& theArray,
                                          Handle(Poly_Triangulation)& theTriangulation)
{
  theArray = new Graphic3d_ArrayOfTriangles (4, 0, Standard_True);
  const gp_Dir aDir = gp::DZ();
  gp_Pnt aPointer = myAxis.Location().XYZ() + myAxis.Direction().XYZ() * myLength;
  gp_XYZ aBotDir = myAxis.Direction().Crossed (aDir).XYZ();
  Standard_Real anEdgeLength = Tan (myAngle * 0.5) * myLength;

  theArray->AddVertex (aPointer, aDir);
  theArray->AddVertex (gp_Pnt (myAxis.Location().XYZ() - aBotDir * anEdgeLength), aDir);
  theArray->AddVertex (gp_Pnt (myAxis.Location().XYZ() + aBotDir * anEdgeLength), aDir);
  theArray->AddVertex (aPointer, aDir);

  theTriangulation = new Poly_Triangulation (3, 1, 0);
  theTriangulation->ChangeNodes().SetValue (1, aPointer);
  theTriangulation->ChangeNodes().SetValue (2, myAxis.Location().XYZ() + aBotDir * anEdgeLength);
  theTriangulation->ChangeNodes().SetValue (3, myAxis.Location().XYZ() - aBotDir * anEdgeLength);
  theTriangulation->ChangeTriangles().SetValue (1, Poly_Triangle (1, 2, 3));
}

//=======================================================================
//class    : FlatArrow
//function : Init
//purpose  :
//=======================================================================
void AIS_ViewCube::FlatArrow::Display (const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                                        const Handle(Graphic3d_Group)& theGroup,
                                        const gp_Ax1& theAxis,
                                        const Standard_Real theLength,
                                        const Standard_Real theAngle)
{
  Reset();

  Handle(Poly_Triangulation) aTri;
  Handle(Graphic3d_ArrayOfTriangles) anArray;

  ToolArrow anArrow (theAxis, theLength, theAngle);
  anArrow.FillArray (anArray, aTri);
  theGroup->AddPrimitiveArray (anArray);

  if (myHighlightPresentation.IsNull())
  {
    myHighlightPresentation = new Prs3d_Presentation (thePrsMgr->StructureManager());
  }
  Handle(Graphic3d_Group) aGroup = Prs3d_Root::CurrentGroup (myHighlightPresentation);
  aGroup->AddPrimitiveArray (anArray);

  myTriangulations.Append (aTri);
}

//=======================================================================
//class    : DiskSegment
//function : Init
//purpose  :
//=======================================================================
void AIS_ViewCube::RoundedArrow::Display (const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                                          const Handle(Graphic3d_Group)& theGroup,
                                          const gp_Pnt& theCenter,
                                          const Standard_Real theArrowLength,
                                          const Standard_Real theArrowAngle,
                                          const Standard_Real theInnerRadius,
                                          const Standard_Real theStartAngle,
                                          const Standard_Real theEndAngle,
                                          const Standard_Boolean theIsClockwise,
                                          const Standard_Integer theSlicesNb)
{
  Reset();

  // Make thickness of arrow tail depended from arrow length and angle
  const Standard_Real anInnerRadius = theInnerRadius;
  const Standard_Real anOuterRadius = theInnerRadius + Tan (theArrowAngle * 0.5) * theArrowLength * 0.6;
  const Standard_Real aRadius = (anInnerRadius + anOuterRadius) * 0.5;
  const Standard_Real aStartAngle = theIsClockwise ? theStartAngle + theArrowLength / aRadius : theStartAngle;
  const Standard_Real anEndAngle = theIsClockwise ? theEndAngle : theEndAngle - theArrowLength / aRadius;

  // Draw tail
  ToolDiskSegment aTool (anInnerRadius, anOuterRadius, aStartAngle, anEndAngle, theSlicesNb);
  gp_Ax3 aSystem (theCenter, gp::DZ(), gp::DX());
  gp_Trsf aTrsf;
  aTrsf.SetTransformation (aSystem, gp_Ax3());
  Handle(Poly_Triangulation) aTri1, aTri2;
  Handle(Graphic3d_ArrayOfTriangles) anArray1, anArray2;
  aTool.FillArray (anArray1, aTri1, aTrsf);
  AddTriangulation (aTri1);

  // Draw arrow
  gp_Pnt anArrowPos = ElCLib::CircleValue (theIsClockwise ? aStartAngle : anEndAngle, aSystem.Ax2(), aRadius);
  gp_Dir aRadiusDir (theCenter.XYZ() - anArrowPos.XYZ());
  ToolArrow anArrow (gp_Ax1 (anArrowPos, theIsClockwise ? aSystem.Direction() ^ aRadiusDir : aRadiusDir ^ aSystem.Direction()), theArrowLength, theArrowAngle);
  anArrow.FillArray (anArray2, aTri2);
  AddTriangulation (aTri2);

  theGroup->AddPrimitiveArray (anArray1);
  theGroup->AddPrimitiveArray (anArray2);

  if (myHighlightPresentation.IsNull())
  {
    myHighlightPresentation = new Prs3d_Presentation (thePrsMgr->StructureManager());
  }
  Handle(Graphic3d_Group) aGroup = Prs3d_Root::CurrentGroup (myHighlightPresentation);
  aGroup->AddPrimitiveArray (anArray1);
  aGroup->AddPrimitiveArray (anArray2);
}
//=======================================================================
//class    : ToolDiskSegment
//function : Constructor
//purpose  :
//=======================================================================
AIS_ViewCube::ToolDiskSegment::ToolDiskSegment (const Standard_Real    theInnerRadius,
                                                 const Standard_Real    theOuterRadius,
                                                 const Standard_Real    theStartAngle,
                                                 const Standard_Real    theEndAngle,
                                                 const Standard_Integer theNbFacettes)
: Prs3d_ToolDisk (theInnerRadius, theOuterRadius, theNbFacettes, 20),
  myStartAngle (theStartAngle),
  myEndAngle (theEndAngle)
{
}

//=======================================================================
//class    : ToolDiskSegment
//function : Vertex
//purpose  :
//=======================================================================
gp_Pnt AIS_ViewCube::ToolDiskSegment::Vertex (const Standard_Real theU, const Standard_Real theV)
{
  const Standard_Real aU      = myStartAngle + theU * (myEndAngle - myStartAngle);
  const Standard_Real aRadius = myInnerRadius + (myOuterRadius - myInnerRadius) * theV;
  return gp_Pnt (Cos (aU) * aRadius, Sin (aU) * aRadius,  0.0);
}

//=======================================================================
//function : ComputeFlat
//purpose  :
//=======================================================================
void AIS_ViewCube::ComputeFlat (const Handle(PrsMgr_PresentationManager3d)& thePrsMgr, const Handle(Prs3d_Presentation)& thePrs)
{
  Handle(Graphic3d_Group) aGroup = Prs3d_Root::NewGroup (thePrs);
  aGroup->SetGroupPrimitivesAspect (myArrowAspect->Aspect());

  gp_XYZ aCenter (gp::Origin().XYZ());
  const gp_Ax2& aPosition = gp::XOY();

  // Here minimal radius of arrow circle is computed: (Size + 2*BoxPadding + 2*AxisPadding) * Cos (M_PI_4) / 2
  //                                                   + AxisPadding + AxisDiameter + ArrowPadding
  // Additionally arrow padding is apply for customization
  Standard_Real aRadius = (mySize + 2 * myBoxPadding + 2 * myAxisPadding) * Sqrt(2.0) / 2.0 + 2.0 + myArrowPadding;
  gp_Ax1 aPositions[4] = {
    gp_Ax1 (aCenter - aPosition.XDirection().XYZ() * (aRadius), aPosition.XDirection().Reversed()),
    gp_Ax1 (aCenter + aPosition.YDirection().XYZ() * (aRadius), aPosition.YDirection()),
    gp_Ax1 (aCenter + aPosition.XDirection().XYZ() * (aRadius), aPosition.XDirection()),
    gp_Ax1 (aCenter - aPosition.YDirection().XYZ() * (aRadius), aPosition.YDirection().Reversed())
  };

  Standard_Integer aPosIndex = 0;
  for (Standard_Integer anIt = VERTEX_INDEX; anIt < ARROW_INDEX; anIt++)
  {
    Handle(FlatArrow) aPart = Handle(FlatArrow)::DownCast (myParts.ChangeFromIndex (anIt));
    aPart->Display (thePrsMgr, aGroup, aPositions[aPosIndex++], myDrawer->ArrowAspect()->Length(), myDrawer->ArrowAspect()->Angle());
    aPart->SetTransformPersistence (myFlatPart->TransformPersistence());
  }

  aRadius += myDrawer->ArrowAspect()->Length() * 0.3;
  Handle(RoundedArrow) anArrow = Handle(RoundedArrow)::DownCast (myParts.ChangeFromIndex (ARROW_INDEX));
  anArrow->Display (thePrsMgr, aGroup, gp_Pnt (aCenter), myDrawer->ArrowAspect()->Length(), myDrawer->ArrowAspect()->Angle(), aRadius, M_PI_4, M_PI_2 - 0.3, Standard_True);
  anArrow = Handle(RoundedArrow)::DownCast (myParts.ChangeFromIndex (ARROW_INDEX + 1));
  anArrow->Display (thePrsMgr, aGroup, gp_Pnt (aCenter), myDrawer->ArrowAspect()->Length(), myDrawer->ArrowAspect()->Angle(), aRadius, M_PI_2 + 0.3, M_PI - M_PI_4);
  for (Standard_Integer anIt = ARROW_INDEX; anIt < ROUND_ARROW_INDEX; anIt++)
  {
    myParts.ChangeFromIndex (anIt)->SetTransformPersistence (myFlatPart->TransformPersistence());
  }
}

//=======================================================================
//function : ComputeFlat
//purpose  :
//=======================================================================
void AIS_ViewCube::ComputeSelectionFlat (const Handle(SelectMgr_Selection)& theSelection)
{
  for (Standard_Integer anIt = VERTEX_INDEX; anIt < ROUND_ARROW_INDEX; anIt++)
  {
    const Handle(SelectMgr_EntityOwner)& anOwner = myStates.FindKey (anIt);
    anOwner->SetSelectable (myFlatPart);
    const Handle(Part)& aPart = myParts.FindFromKey (anOwner);
    const NCollection_Sequence<Handle(Poly_Triangulation)>& aTris = aPart->Triangulations();
    for (NCollection_Sequence<Handle(Poly_Triangulation)>::Iterator aTriIt (aTris); aTriIt.More(); aTriIt.Next())
    {
      Handle(Select3D_SensitiveTriangulation) aTri = new Select3D_SensitiveTriangulation (anOwner, aTriIt.Value(), TopLoc_Location(), Standard_True);
      theSelection->Add (aTri);
    }
  }
}

//=======================================================================
//function : Constructor
//purpose  :
//=======================================================================
AIS_ViewCubeFlat::AIS_ViewCubeFlat (const Handle(AIS_ViewCube)& theParent)
  : AIS_ViewCube (theParent)
{
  //
}

//=======================================================================
//function : parent
//purpose  :
//=======================================================================
Handle(AIS_ViewCube) AIS_ViewCubeFlat::parent() const
{
  Handle(PrsMgr_PresentableObject) aParent (Parent());
  return Handle(AIS_ViewCube)::DownCast (aParent);
}

//=======================================================================
//function : Compute
//purpose  :
//=======================================================================
void AIS_ViewCubeFlat::Compute (const Handle(PrsMgr_PresentationManager3d)& thePrsMgr,
                                const Handle(Prs3d_Presentation)& thePrs,
                                const Standard_Integer theMode)
{
  if (theMode != 0)
  {
    return;
  }

  Handle(AIS_ViewCube) aParent = parent();

  thePrs->SetMutable (Standard_True);
  thePrs->SetZLayer (aParent->ZLayer());

  // Set transform persistance options
  Aspect_TypeOfTriedronPosition aPersPos;
  Graphic3d_Vec2i aPersOffset;
  aParent->Position (aPersPos, aPersOffset);
  setTransformPersistence (new Graphic3d_TransformPers (Graphic3d_TMF_2d, aPersPos, aPersOffset));

  aParent->ComputeFlat (thePrsMgr, thePrs);
}

//=======================================================================
//function : ComputeSelection
//purpose  :
//=======================================================================
void AIS_ViewCubeFlat::ComputeSelection (const Handle(SelectMgr_Selection)& theSelection,
                                          const Standard_Integer theMode)
{
  if (theMode != 0)
  {
    return;
  }

  parent()->ComputeSelectionFlat (theSelection);
}

//=======================================================================
//function : HilightOwnerWithColor
//purpose  :
//=======================================================================
void AIS_ViewCubeFlat::HilightOwnerWithColor (const Handle(PrsMgr_PresentationManager3d)& thePM,
                                              const Handle(Prs3d_Drawer)& theStyle,
                                              const Handle(SelectMgr_EntityOwner)& theOwner)
{
  (void) theStyle;
  parent()->HilightOwnerWithColor (thePM, parent()->DynamicHilightAttributes(), theOwner);
}

//=======================================================================
//function : HilightSelected
//purpose  :
//=======================================================================
void AIS_ViewCubeFlat::HilightSelected (const Handle(PrsMgr_PresentationManager3d)& thePM,
                                        const SelectMgr_SequenceOfOwner& theSeq)
{
  parent()->HilightSelected (thePM, theSeq);
}
