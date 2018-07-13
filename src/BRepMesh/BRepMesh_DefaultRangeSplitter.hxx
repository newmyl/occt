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

#ifndef _BRepMesh_DefaultRangeSplitter_HeaderFile
#define _BRepMesh_DefaultRangeSplitter_HeaderFile

#include <Standard_Type.hxx>
#include <gp_Pnt2d.hxx>
#include <BRepAdaptor_HSurface.hxx>
#include <IMeshData_Types.hxx>
#include <IMeshData_Face.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAbs_IsoType.hxx>

struct IMeshTools_Parameters;

//! Default tool to define range of discrete face model and 
//! obtain grid points disturbed within this range.
class BRepMesh_DefaultRangeSplitter
{
public:

  //! Constructor.
  Standard_EXPORT BRepMesh_DefaultRangeSplitter()
    : myIsValid (Standard_True)
  {
  }

  //! Destructor.
  Standard_EXPORT virtual ~BRepMesh_DefaultRangeSplitter()
  {
  }

  //! Resets this splitter. Must be called before first use.
  Standard_EXPORT virtual void Reset(const IMeshData::IFaceHandle& theDFace,
                                     const IMeshTools_Parameters& /*theParameters*/)
  {
    myDFace = theDFace;
    myRangeU.first  = myRangeV.first  =  1.e100;
    myRangeU.second = myRangeV.second = -1.e100;
    myDelta.first   = myDelta.second  = 1.;
    myTolerance.first = myTolerance.second = Precision::Confusion();
  }

  //! Registers border point.
  Standard_EXPORT virtual void AddPoint(const gp_Pnt2d& thePoint)
  {
    myRangeU.first  = Min(thePoint.X(), myRangeU.first);
    myRangeU.second = Max(thePoint.X(), myRangeU.second);
    myRangeV.first  = Min(thePoint.Y(), myRangeV.first);
    myRangeV.second = Max(thePoint.Y(), myRangeV.second);
  }

  //! Updates discrete range of surface according to its geometric range.
  Standard_EXPORT virtual void AdjustRange()
  {
    const Handle(BRepAdaptor_HSurface)& aSurface = GetSurface();
    updateRange(aSurface->FirstUParameter(), aSurface->LastUParameter(),
                aSurface->IsUPeriodic(), myRangeU.first, myRangeU.second);

    updateRange(aSurface->FirstVParameter(), aSurface->LastVParameter(),
                aSurface->IsVPeriodic(), myRangeV.first, myRangeV.second);

    const Standard_Real aLengthU = computeLengthU();
    const Standard_Real aLengthV = computeLengthV();
    myIsValid = aLengthU > Precision::PConfusion () && aLengthV > Precision::PConfusion ();

    if (myIsValid)
    {
      computeTolerance(aLengthU, aLengthV);
      computeDelta    (aLengthU, aLengthV);
    }
  }

  //! Returns True if computed range is valid.
  Standard_EXPORT virtual Standard_Boolean IsValid()
  {
    return myIsValid;
  }

  //! Scales the given point from real parametric space 
  //! to face basis and otherwise.
  //! @param thePoint point to be scaled.
  //! @param isToFaceBasis if TRUE converts point to face basis,
  //! otherwise performs reverse conversion.
  //! @return scaled point.
  Standard_EXPORT gp_Pnt2d Scale(const gp_Pnt2d&        thePoint,
                                 const Standard_Boolean isToFaceBasis) const
  {
    return isToFaceBasis ?
      gp_Pnt2d ((thePoint.X () - myRangeU.first) / myDelta.first,
                (thePoint.Y () - myRangeV.first) / myDelta.second) :
      gp_Pnt2d (thePoint.X () * myDelta.first  + myRangeU.first,
                thePoint.Y () * myDelta.second + myRangeV.first);
  }

  //! Returns list of nodes generated using surface data and specified parameters.
  //! By default returns null ptr.
  Standard_EXPORT virtual Handle(IMeshData::ListOfPnt2d) GenerateSurfaceNodes(
    const IMeshTools_Parameters& /*theParameters*/) const
  {
    return Handle(IMeshData::ListOfPnt2d)();
  }

  //! Returns point in 3d space corresponded to the given 
  //! point defined in parameteric space of surface.
  inline gp_Pnt Point(const gp_Pnt2d& thePoint2d) const
  {
    return GetSurface()->Value(thePoint2d.X(), thePoint2d.Y());
  }

protected:

  //! Computes parametric tolerance taking length along U and V into account.
  virtual void computeTolerance(
    const Standard_Real /*theLenU*/,
    const Standard_Real /*theLenV*/)
  {
    const Standard_Real aDiffU = myRangeU.second - myRangeU.first;
    const Standard_Real aDiffV = myRangeV.second - myRangeV.first;

    const Standard_Real aDeflectionUV = 1.e-05;
    myTolerance.first  = Max(aDeflectionUV, Precision::Confusion() * aDiffU);
    myTolerance.second = Max(aDeflectionUV, Precision::Confusion() * aDiffV);
  }

  //! Computes parametric delta taking length along U and V and value of tolerance into account.
  virtual void computeDelta(
    const Standard_Real theLengthU,
    const Standard_Real theLengthV)
  {
    const Standard_Real aDiffU = myRangeU.second - myRangeU.first;
    const Standard_Real aDiffV = myRangeV.second - myRangeV.first;

    myDelta.first  = aDiffU / (theLengthU < myTolerance.first  ? 1. : theLengthU);
    myDelta.second = aDiffV / (theLengthV < myTolerance.second ? 1. : theLengthV);
  }

public:
  //! Returns face model.
  inline const IMeshData::IFaceHandle& GetDFace() const
  {
    return myDFace;
  }

  //! Returns surface.
  inline const Handle(BRepAdaptor_HSurface)& GetSurface() const
  {
    return myDFace->GetSurface();
  }

  //! Returns U range.
  inline const std::pair<Standard_Real, Standard_Real>& GetRangeU() const
  {
    return myRangeU;
  }

  //! Returns V range.
  inline const std::pair<Standard_Real, Standard_Real>& GetRangeV() const
  {
    return myRangeV;
  }

  //! Returns delta.
  inline const std::pair<Standard_Real, Standard_Real>& GetDelta () const
  {
    return myDelta;
  }

  inline const std::pair<Standard_Real, Standard_Real>& GetToleranceUV() const
  {
    return myTolerance;
  }

private:

  //! Computes length along U direction.
  inline Standard_Real computeLengthU()
  {
    Standard_Real longu = 0.0;
    gp_Pnt P11, P12, P21, P22, P31, P32;

    Standard_Real du     = 0.05 * (myRangeU.second - myRangeU.first);
    Standard_Real dfvave = 0.5  * (myRangeV.second + myRangeV.first);
    Standard_Real dfucur;
    Standard_Integer i1;

    const Handle(BRepAdaptor_HSurface)& gFace = GetSurface();
    gFace->D0(myRangeU.first, myRangeV.first,  P11);
    gFace->D0(myRangeU.first, dfvave,          P21);
    gFace->D0(myRangeU.first, myRangeV.second, P31);
    for (i1 = 1, dfucur = myRangeU.first + du; i1 <= 20; i1++, dfucur += du)
    {
      gFace->D0(dfucur, myRangeV.first,  P12);
      gFace->D0(dfucur, dfvave,          P22);
      gFace->D0(dfucur, myRangeV.second, P32);
      longu += (P11.Distance(P12) + P21.Distance(P22) + P31.Distance(P32));
      P11 = P12;
      P21 = P22;
      P31 = P32;
    }

    return longu / 3.;
  }

  //! Computes length along V direction.
  inline Standard_Real computeLengthV()
  {
    Standard_Real longv = 0.0;
    gp_Pnt P11, P12, P21, P22, P31, P32;

    Standard_Real dv     = 0.05 * (myRangeV.second - myRangeV.first);
    Standard_Real dfuave = 0.5  * (myRangeU.second + myRangeU.first);
    Standard_Real dfvcur;
    Standard_Integer i1;

    const Handle(BRepAdaptor_HSurface)& gFace = GetSurface();
    gFace->D0(myRangeU.first,  myRangeV.first, P11);
    gFace->D0(dfuave,          myRangeV.first, P21);
    gFace->D0(myRangeU.second, myRangeV.first, P31);
    for (i1 = 1, dfvcur = myRangeV.first + dv; i1 <= 20; i1++, dfvcur += dv)
    {
      gFace->D0(myRangeU.first,  dfvcur, P12);
      gFace->D0(dfuave,          dfvcur, P22);
      gFace->D0(myRangeU.second, dfvcur, P32);
      longv += (P11.Distance(P12) + P21.Distance(P22) + P31.Distance(P32));
      P11 = P12;
      P21 = P22;
      P31 = P32;
    }

    return longv / 3.;
  }

  //! Updates discrete range of surface according to its geometric range.
  inline void updateRange(const Standard_Real     theGeomFirst,
                          const Standard_Real     theGeomLast,
                          const Standard_Boolean  isPeriodic,
                          Standard_Real&          theDiscreteFirst,
                          Standard_Real&          theDiscreteLast)
  {
    if (theDiscreteFirst < theGeomFirst ||
        theDiscreteLast  > theGeomLast)
    {
      if (isPeriodic)
      {
        if ((theDiscreteLast - theDiscreteFirst) > (theGeomLast - theGeomFirst))
        {
          theDiscreteLast = theDiscreteFirst + (theGeomLast - theGeomFirst);
        }
      }
      else
      {
        if (theGeomFirst > theDiscreteFirst)
        {
          theDiscreteFirst = theGeomFirst;
        }

        if (theGeomLast < theDiscreteLast)
        {
          theDiscreteLast = theGeomLast;
        }
      }
    }
  }

protected:
  IMeshData::IFaceHandle                  myDFace;
  std::pair<Standard_Real, Standard_Real> myRangeU;
  std::pair<Standard_Real, Standard_Real> myRangeV;
  std::pair<Standard_Real, Standard_Real> myDelta;
  std::pair<Standard_Real, Standard_Real> myTolerance;
  Standard_Boolean                        myIsValid;
};

#endif