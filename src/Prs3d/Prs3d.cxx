// Created on: 1993-08-27
// Created by: Jean-Louis FRENKEL
// Copyright (c) 1993-1999 Matra Datavision
// Copyright (c) 1999-2014 OPEN CASCADE SAS
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

#include <Prs3d.hxx>

#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Pnt.hxx>
#include <Graphic3d_Group.hxx>
#include <Prs3d_Drawer.hxx>
#include <Prs3d_LineAspect.hxx>
#include <Prs3d_Root.hxx>
#include <TopoDS_Shape.hxx>
#include <Graphic3d_ArrayOfSegments.hxx>

namespace
{
  static Standard_CString Prs3d_Table_PrintTypeOfHighlight[7] =
  {
    "NONE", "SELECTED", "DYNAMIC", "LOCAL_SELECTED", "LOCAL_DYNAMIC", "SUB_INTENSITY", "NB"
  };

  static Standard_CString Prs3d_Table_PrintTypeOfHLR[3] =
  {
    "NOT_SET", "POLY_ALGO", "ALGO"
  };

  static Standard_CString Prs3d_Table_PrintVertexDrawMode[3] =
  {
    "ISOLATED", "ALL", "INHERITED"
  };

  static Standard_CString Prs3d_Table_PrintDatumParts[11] =
  {
    "ORIGIN", "X_AXIS", "Y_AXIS", "Z_AXIS", "X_ARROW", "Y_ARROW", "Z_ARROW",
    "XOY_AXIS", "YOZ_AXIS", "XOZ_AXIS", "NONE"
  };

  static Standard_CString Prs3d_Table_PrintDatumAttribute[8] =
  {
    "X_AXIS_LENGTH", "Y_AXIS_LENGTH", "Z_AXIS_LENGTH", "SHADING_TUBE_RADIUS_PERCENT",
    "SHADING_CONE_RADIUS_PERCENT", "SHADING_CONE_LENGTH_PERCENT", "SHADING_ORIGIN_RADIUS_PERCENT",
    "SHADING_NUMBER_OF_FACETTES"
  };

  static Standard_CString Prs3d_Table_PrintDatumAxes[7] =
  {
    "X_AXIS", "Y_AXIS", "Z_AXIS", "XY_AXIS", "YZ_AXIS", "XZ_AXIS", "XYZ_AXIS"
  };

  static Standard_CString Prs3d_Table_PrintDimensionArrowOrientation[3] =
  {
    "INTERNAL", "EXTERNAL", "FIT"
  };

  static Standard_CString Prs3d_Table_PrintDimensionTextHorizontalPosition[4] =
  {
    "LEFT", "RIGHT", "CENTER", "FIT"
  };

  static Standard_CString Prs3d_Table_PrintDimensionTextVerticalPosition[3] =
  {
    "ABOVE", "BELOW", "CENTER"
  };
}

//=======================================================================
//function : MatchSegment
//purpose  :
//=======================================================================
Standard_Boolean Prs3d::MatchSegment 
                 (const Standard_Real X,
                  const Standard_Real Y,
                  const Standard_Real Z,
                  const Standard_Real aDistance,
                  const gp_Pnt& P1,
                  const gp_Pnt& P2,
                  Standard_Real& dist)
{
  Standard_Real X1,Y1,Z1,X2,Y2,Z2;
  P1.Coord(X1,Y1,Z1); P2.Coord(X2,Y2,Z2);
  Standard_Real DX = X2-X1; 
  Standard_Real DY = Y2-Y1; 
  Standard_Real DZ = Z2-Z1;
  Standard_Real Dist = DX*DX + DY*DY + DZ*DZ;
  if (Dist == 0.) return Standard_False;
  
  Standard_Real Lambda = ((X-X1)*DX + (Y-Y1)*DY + (Z-Z1)*DZ)/Dist;
  if ( Lambda < 0. || Lambda > 1. ) return Standard_False;
  dist =  Abs(X-X1-Lambda*DX) +
          Abs(Y-Y1-Lambda*DY) +
          Abs(Z-Z1-Lambda*DZ);
  return (dist < aDistance);
}

//=======================================================================
//function : GetDeflection
//purpose  :
//=======================================================================
Standard_Real Prs3d::GetDeflection (const TopoDS_Shape&         theShape,
                                    const Handle(Prs3d_Drawer)& theDrawer)
{
  if (theDrawer->TypeOfDeflection() != Aspect_TOD_RELATIVE)
  {
    return theDrawer->MaximalChordialDeviation();
  }

  Bnd_Box aBndBox;
  BRepBndLib::Add (theShape, aBndBox, Standard_False);
  if (aBndBox.IsVoid())
  {
    return theDrawer->MaximalChordialDeviation();
  }
  else if (aBndBox.IsOpen())
  {
    if (!aBndBox.HasFinitePart())
    {
      return theDrawer->MaximalChordialDeviation();
    }
    aBndBox = aBndBox.FinitePart();
  }

  Graphic3d_Vec3d aVecMin, aVecMax;
  aBndBox.Get (aVecMin.x(), aVecMin.y(), aVecMin.z(), aVecMax.x(), aVecMax.y(), aVecMax.z());
  const Graphic3d_Vec3d aDiag = aVecMax - aVecMin;
  const Standard_Real aDeflection = aDiag.maxComp() * theDrawer->DeviationCoefficient() * 4.0;

  // we store computed relative deflection of shape as absolute deviation coefficient
  // in case relative type to use it later on for sub-shapes.
  theDrawer->SetMaximalChordialDeviation (aDeflection);
  return aDeflection;
}

//==================================================================
// function: PrimitivesFromPolylines
// purpose:
//==================================================================
Handle(Graphic3d_ArrayOfPrimitives) Prs3d::PrimitivesFromPolylines (const Prs3d_NListOfSequenceOfPnt& thePoints)
{
  if (thePoints.IsEmpty())
  {
    return Handle(Graphic3d_ArrayOfPrimitives)();
  }

  Standard_Integer aNbVertices = 0;
  for (Prs3d_NListOfSequenceOfPnt::Iterator anIt (thePoints); anIt.More(); anIt.Next())
  {
    aNbVertices += anIt.Value()->Length();
  }
  const Standard_Integer aSegmentEdgeNb = (aNbVertices - thePoints.Size()) * 2;
  Handle(Graphic3d_ArrayOfSegments) aSegments = new Graphic3d_ArrayOfSegments (aNbVertices, aSegmentEdgeNb);
  for (Prs3d_NListOfSequenceOfPnt::Iterator anIt (thePoints); anIt.More(); anIt.Next())
  {
    const Handle(TColgp_HSequenceOfPnt)& aPoints = anIt.Value();

    Standard_Integer aSegmentEdge = aSegments->VertexNumber() + 1;
    aSegments->AddVertex (aPoints->First());
    for (Standard_Integer aPntIter = aPoints->Lower() + 1; aPntIter <= aPoints->Upper(); ++aPntIter)
    {
      aSegments->AddVertex (aPoints->Value (aPntIter));
      aSegments->AddEdge (  aSegmentEdge);
      aSegments->AddEdge (++aSegmentEdge);
    }
  }

  return aSegments;
}

//==================================================================
// function: AddPrimitivesGroup
// purpose:
//==================================================================
void Prs3d::AddPrimitivesGroup (const Handle(Prs3d_Presentation)& thePrs,
                                const Handle(Prs3d_LineAspect)&   theAspect,
                                Prs3d_NListOfSequenceOfPnt&       thePolylines)
{
  Handle(Graphic3d_ArrayOfPrimitives) aPrims = Prs3d::PrimitivesFromPolylines (thePolylines);
  thePolylines.Clear();
  if (!aPrims.IsNull())
  {
    Handle(Graphic3d_Group) aGroup = Prs3d_Root::NewGroup (thePrs);
    aGroup->SetPrimitivesAspect (theAspect->Aspect());
    aGroup->AddPrimitiveArray (aPrims);
  }
}

//=======================================================================
//function : TypeOfHighlightToString
//purpose  :
//=======================================================================
Standard_CString Prs3d::TypeOfHighlightToString (Prs3d_TypeOfHighlight theType)
{
  return Prs3d_Table_PrintTypeOfHighlight[theType];
}

//=======================================================================
//function : TypeOfHighlightFromString
//purpose  :
//=======================================================================
Standard_Boolean Prs3d::TypeOfHighlightFromString (Standard_CString theTypeString,
                                                   Prs3d_TypeOfHighlight& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= Prs3d_TypeOfHighlight_NB; ++aTypeIter)
  {
    Standard_CString aTypeName = Prs3d_Table_PrintTypeOfHighlight[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Prs3d_TypeOfHighlight (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : TypeOfHLRToString
//purpose  :
//=======================================================================
Standard_CString Prs3d::TypeOfHLRToString (Prs3d_TypeOfHLR theType)
{
  return Prs3d_Table_PrintTypeOfHLR[theType];
}

//=======================================================================
//function : TypeOfHLRFromString
//purpose  :
//=======================================================================
Standard_Boolean Prs3d::TypeOfHLRFromString (Standard_CString theTypeString,
                                             Prs3d_TypeOfHLR& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= Prs3d_TOH_Algo; ++aTypeIter)
  {
    Standard_CString aTypeName = Prs3d_Table_PrintTypeOfHLR[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Prs3d_TypeOfHLR (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : VertexDrawModeToString
//purpose  :
//=======================================================================
Standard_CString Prs3d::VertexDrawModeToString (Prs3d_VertexDrawMode theType)
{
  return Prs3d_Table_PrintVertexDrawMode[theType];
}

//=======================================================================
//function : VertexDrawModeFromString
//purpose  :
//=======================================================================
Standard_Boolean Prs3d::VertexDrawModeFromString (Standard_CString theTypeString,
                                                  Prs3d_VertexDrawMode& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= Prs3d_VDM_Inherited; ++aTypeIter)
  {
    Standard_CString aTypeName = Prs3d_Table_PrintVertexDrawMode[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Prs3d_VertexDrawMode (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : DatumPartsToString
//purpose  :
//=======================================================================
Standard_CString Prs3d::DatumPartsToString (Prs3d_DatumParts theType)
{
  return Prs3d_Table_PrintDatumParts[theType];
}

//=======================================================================
//function : DatumPartsFromString
//purpose  :
//=======================================================================
Standard_Boolean Prs3d::DatumPartsFromString (Standard_CString theTypeString,
                                              Prs3d_DatumParts& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= Prs3d_DP_None; ++aTypeIter)
  {
    Standard_CString aTypeName = Prs3d_Table_PrintDatumParts[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Prs3d_DatumParts (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : DatumAttributeToString
//purpose  :
//=======================================================================
Standard_CString Prs3d::DatumAttributeToString (Prs3d_DatumAttribute theType)
{
  return Prs3d_Table_PrintDatumAttribute[theType];
}

//=======================================================================
//function : DatumAttributeFromString
//purpose  :
//=======================================================================
Standard_Boolean Prs3d::DatumAttributeFromString (Standard_CString theTypeString,
                                                  Prs3d_DatumAttribute& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= Prs3d_DP_ShadingNumberOfFacettes; ++aTypeIter)
  {
    Standard_CString aTypeName = Prs3d_Table_PrintDatumAttribute[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Prs3d_DatumAttribute (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : DatumAxesToString
//purpose  :
//=======================================================================
Standard_CString Prs3d::DatumAxesToString (Prs3d_DatumAxes theType)
{
  return Prs3d_Table_PrintDatumAxes[theType];
}

//=======================================================================
//function : DatumAxesFromString
//purpose  :
//=======================================================================
Standard_Boolean Prs3d::DatumAxesFromString (Standard_CString theTypeString,
                                             Prs3d_DatumAxes& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= Prs3d_DA_XYZAxis; ++aTypeIter)
  {
    Standard_CString aTypeName = Prs3d_Table_PrintDatumAxes[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Prs3d_DatumAxes (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : DimensionArrowOrientationToString
//purpose  :
//=======================================================================
Standard_CString Prs3d::DimensionArrowOrientationToString (Prs3d_DimensionArrowOrientation theType)
{
  return Prs3d_Table_PrintDimensionArrowOrientation[theType];
}

//=======================================================================
//function : DimensionArrowOrientationFromString
//purpose  :
//=======================================================================
Standard_Boolean Prs3d::DimensionArrowOrientationFromString (Standard_CString theTypeString,
                                                             Prs3d_DimensionArrowOrientation& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= Prs3d_DAO_Fit; ++aTypeIter)
  {
    Standard_CString aTypeName = Prs3d_Table_PrintDimensionArrowOrientation[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Prs3d_DimensionArrowOrientation (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : DimensionTextHorizontalPositionToString
//purpose  :
//=======================================================================
Standard_CString Prs3d::DimensionTextHorizontalPositionToString (Prs3d_DimensionTextHorizontalPosition theType)
{
  return Prs3d_Table_PrintDimensionTextHorizontalPosition[theType];
}

//=======================================================================
//function : DimensionTextHorizontalPositionFromString
//purpose  :
//=======================================================================
Standard_Boolean Prs3d::DimensionTextHorizontalPositionFromString (Standard_CString theTypeString,
                                                                   Prs3d_DimensionTextHorizontalPosition& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= Prs3d_DTHP_Fit; ++aTypeIter)
  {
    Standard_CString aTypeName = Prs3d_Table_PrintDimensionTextHorizontalPosition[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Prs3d_DimensionTextHorizontalPosition (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}

//=======================================================================
//function : DimensionTextVerticalPositionToString
//purpose  :
//=======================================================================
Standard_CString Prs3d::DimensionTextVerticalPositionToString (Prs3d_DimensionTextVerticalPosition theType)
{
  return Prs3d_Table_PrintDimensionTextVerticalPosition[theType];
}

//=======================================================================
//function : DimensionTextVerticalPositionFromString
//purpose  :
//=======================================================================
Standard_Boolean Prs3d::DimensionTextVerticalPositionFromString (Standard_CString theTypeString,
                                                                 Prs3d_DimensionTextVerticalPosition& theType)
{
  TCollection_AsciiString aName (theTypeString);
  aName.UpperCase();
  for (Standard_Integer aTypeIter = 0; aTypeIter <= Prs3d_DTVP_Center; ++aTypeIter)
  {
    Standard_CString aTypeName = Prs3d_Table_PrintDimensionTextVerticalPosition[aTypeIter];
    if (aName == aTypeName)
    {
      theType = Prs3d_DimensionTextVerticalPosition (aTypeIter);
      return Standard_True;
    }
  }
  return Standard_False;
}
