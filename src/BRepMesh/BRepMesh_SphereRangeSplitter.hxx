// Created on: 2016-07-07
// Copyright (c) 2016 OPEN CASCADE SAS
// Created by: Oleg AGASHIN
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

#ifndef _BRepMesh_SphereRangeSplitter_HeaderFile
#define _BRepMesh_SphereRangeSplitter_HeaderFile

#include <BRepMesh_DefaultRangeSplitter.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <IMeshTools_Parameters.hxx>

//! Auxiliary class extending default range splitter in
//! order to generate internal nodes for spherical surface.
class BRepMesh_SphereRangeSplitter : public BRepMesh_DefaultRangeSplitter
{
public:

  //! Constructor.
  Standard_EXPORT BRepMesh_SphereRangeSplitter()
  {
  }

  //! Destructor.
  Standard_EXPORT virtual ~BRepMesh_SphereRangeSplitter()
  {
  }

  //! Returns list of nodes generated using surface data and specified parameters.
  Standard_EXPORT virtual Handle(IMeshData::ListOfPnt2d) GenerateSurfaceNodes(
    const IMeshTools_Parameters& theParameters) const Standard_OVERRIDE
  {
    // Calculate parameters for iteration in V direction
    Standard_Real aStep = 0.7 * GCPnts_TangentialDeflection::ArcAngularStep(
      GetDFace()->GetSurface()->Sphere().Radius(), GetDFace()->GetDeflection(),
      theParameters.Angle, theParameters.MinSize);

    const std::pair<Standard_Real, Standard_Real>* aRange[2] = {
      &GetRangeV(),
      &GetRangeU()
    };

    std::pair<Standard_Real, Standard_Real> aStepAndOffset[2];
    computeStep(*aRange[0], aStep, aStepAndOffset[0]);
    computeStep(*aRange[1], aStep, aStepAndOffset[1]);

    const Handle(NCollection_IncAllocator) aTmpAlloc =
      new NCollection_IncAllocator(IMeshData::MEMORY_BLOCK_SIZE_HUGE);
    Handle(IMeshData::ListOfPnt2d) aNodes = new IMeshData::ListOfPnt2d(aTmpAlloc);

    const Standard_Real aHalfDu = aStepAndOffset[1].first * 0.5;
    Standard_Boolean Shift = Standard_False;
    Standard_Real aPasV = aRange[0]->first + aStepAndOffset[0].first;
    for (; aPasV < aStepAndOffset[0].second; aPasV += aStepAndOffset[0].first)
    {
      Shift = !Shift;
      const Standard_Real d = (Shift) ? aHalfDu : 0.;
      Standard_Real aPasU = aRange[1]->first + d;
      for (; aPasU < aStepAndOffset[1].second; aPasU += aStepAndOffset[1].first)
      {
        aNodes->Append(gp_Pnt2d(aPasU, aPasV));
      }
    }

    return aNodes;
  }

private:

  //! Computes step for the given range.
  inline void computeStep(
    const std::pair<Standard_Real, Standard_Real>& theRange,
    const Standard_Real                            theDefaultStep,
    std::pair<Standard_Real, Standard_Real>&       theStepAndOffset) const
  {
    const Standard_Real aDiff = theRange.second - theRange.first;
    theStepAndOffset.first  = aDiff / ((Standard_Integer) (aDiff / theDefaultStep) + 1);
    theStepAndOffset.second = theRange.second - Precision::PConfusion();
  }
};

#endif
