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

#ifndef _GeomAPI_CurveChartParameter_HeaderFile
#define _GeomAPI_CurveChartParameter_HeaderFile

#include <Standard.hxx>

#include <Standard_Boolean.hxx>
#include <Standard_Real.hxx>
#include <TColgp_HArray1OfXYZ.hxx>
#include <Adaptor3d_HCurve.hxx>


class Geom_BSplineCurve;
class Law_BSpline;
//! Approximation of a  Curve  to  make  its
//! parameter be the same that the parameter of a given 
//! chart data


class GeomAPI_CurveChartParameter 
{
public:

  DEFINE_STANDARD_ALLOC
  struct CurveChart
   {
     Standard_Integer myNbPnts;
     Handle(TColgp_HArray1OfXYZ) myPoints;
     Handle(TColgp_HArray1OfXYZ) myTangents;
     Handle(TColStd_HArray1OfReal) myScales;
     Handle(TColStd_HArray1OfReal) myParams;
     Standard_Real myMaxDefl;
     Standard_Real myMaxAngDefl;
   };

  //! 
  Standard_EXPORT GeomAPI_CurveChartParameter(const Handle(Geom_BSplineCurve)& C3D, 
                                              const CurveChart& CrvChart, const Standard_Real Tol);
    
  Standard_EXPORT Standard_Boolean IsDone() const;
  
  Standard_EXPORT Standard_Real TolReached() const;
  
  //! Tells whether the original data  had already the  same
  //! parameter up to  the tolerance :  in that case nothing
  //! is done.
  Standard_EXPORT Standard_Boolean IsCurveChartParameter() const;
  
  //! Returns the 3D  curve that has  the same parameter  as
  //! curve chart

  Standard_EXPORT  const Handle(Geom_BSplineCurve)& ReparCurve() const;

  //! Returns the reparametrization law, used for reparametrization
  Standard_EXPORT  const Handle(Law_BSpline)& ReparLaw() const;

  //! Returns the reparametrization law, used for reparametrization
  Standard_EXPORT static Standard_Boolean 
     ReparWithLaw(const Handle(Geom_BSplineCurve)& theCrv, 
                  const Handle(Law_BSpline)& theReparLaw,
                  const Standard_Real theTol,
                  Handle(Geom_BSplineCurve)& theReparCurve,
                  Standard_Real& theTolReached);

protected:





private:
  
  //! 
  Standard_EXPORT void Build (const Standard_Real Tol);

  Standard_Boolean myCurveChartParameter;
  Standard_Boolean myDone;
  Standard_Real myTolReached;
  CurveChart myCrvChart;
  Handle(Geom_BSplineCurve) myReparCurve;
  Handle(Law_BSpline) myReparLaw;

};



#endif // _GeomAPI_CurveChartParameter_HeaderFile
