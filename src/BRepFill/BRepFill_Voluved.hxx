// Created on: 2018-03-14
// Created by: Nikolai BUKHALOV
// Copyright (c) 1999-2018 OPEN CASCADE SAS
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

#ifndef _BRepFill_Voluved_HeaderFile
#define _BRepFill_Voluved_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Handle.hxx>

#include <TopoDS_Compound.hxx>
#include <TopoDS_Wire.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
class BOPAlgo_MakerVolume;

//! Constructs an evolved volume from a spine (wire or face)
//! and  a profile ( wire).
class BRepFill_Voluved 
{
public:

  DEFINE_STANDARD_ALLOC

  Standard_EXPORT BRepFill_Voluved() :myErrorStatus(BRepFill_Voluved_Empty),
                                      myFuzzyValue(0.0)
  {
  }
  
  Standard_EXPORT void Perform(const TopoDS_Wire& theSpine, const TopoDS_Wire& theProfile, const Standard_Real theTolerance, const Standard_Boolean theSolidReq = Standard_True);
  
  Standard_Boolean IsDone(unsigned int* theErrorCode = 0) const
  {
    if (theErrorCode)
      *theErrorCode = myErrorStatus;

    return (myErrorStatus == BRepFill_Voluved_OK);
  }

  //! returns the resulting shape.
  const TopoDS_Shape& Shape() const
  {
    return myResult;
  }
  
protected:

  Standard_EXPORT void PerformSweep();

  Standard_EXPORT void GetLids();

  Standard_EXPORT void BuildSolid();

  Standard_EXPORT void RemoveExcessSolids(const TopTools_ListOfShape& theLSplits,
                                          const TopoDS_Shape& theShape,
                                          TopTools_ListOfShape& theArgsList,
                                          BOPAlgo_MakerVolume& theMV);

  Standard_EXPORT void ExtractOuterSolid(TopoDS_Shape& theShape,
                                         TopTools_ListOfShape& theArgsList);

  Standard_EXPORT void GetSpineAndProfile(const TopoDS_Wire& theSpine,
                                          const TopoDS_Wire& theProfile);

private:

  enum
  {
    BRepFill_Voluved_Empty = 0,
    BRepFill_Voluved_NotPlanarSpine,
    BRepFill_Voluved_SweepError,
    BRepFill_Voluved_NoLids,
    BRepFill_Voluved_NotSolid,
    BRepFill_Voluved_NotVolume,
    BRepFill_Voluved_OK = UINT_MAX
  } myErrorStatus;

  TopoDS_Wire mySpine;
  TopoDS_Wire myProfile;
  TopoDS_Shape myPipeShell;
  TopoDS_Compound myTopBottom; // Lids can be split on several faces
  TopoDS_Shape myResult;
  Standard_Real myFuzzyValue;


};







#endif // _BRepFill_Voluved_HeaderFile
