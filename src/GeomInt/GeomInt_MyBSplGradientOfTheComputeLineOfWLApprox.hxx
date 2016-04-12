// Created on: 1995-01-27
// Created by: Jacques GOUSSARD
// Copyright (c) 1995-1999 Matra Datavision
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

#ifndef _GeomInt_MyBSplGradientOfTheComputeLineOfWLApprox_HeaderFile
#define _GeomInt_MyBSplGradientOfTheComputeLineOfWLApprox_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Macro.hxx>

#include <AppParCurves_MultiBSpCurve.hxx>
#include <math_Vector.hxx>
#include <Standard_Real.hxx>
#include <Standard_Boolean.hxx>
#include <Standard_Integer.hxx>
#include <AppParCurves_HArray1OfConstraintCouple.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
class Standard_OutOfRange;
class StdFail_NotDone;
class GeomInt_TheMultiLineOfWLApprox;
class GeomInt_TheMultiLineToolOfWLApprox;
class GeomInt_BSpParLeastSquareOfMyBSplGradientOfTheComputeLineOfWLApprox;
class GeomInt_BSpParFunctionOfMyBSplGradientOfTheComputeLineOfWLApprox;
class GeomInt_BSpGradient_BFGSOfMyBSplGradientOfTheComputeLineOfWLApprox;
class AppParCurves_MultiBSpCurve;



class GeomInt_MyBSplGradientOfTheComputeLineOfWLApprox 
{
public:

  DEFINE_STANDARD_ALLOC

  
  Standard_EXPORT GeomInt_MyBSplGradientOfTheComputeLineOfWLApprox(const GeomInt_TheMultiLineOfWLApprox& SSP, const Standard_Integer FirstPoint, const Standard_Integer LastPoint, const Handle(AppParCurves_HArray1OfConstraintCouple)& TheConstraints, math_Vector& Parameters, const TColStd_Array1OfReal& Knots, const TColStd_Array1OfInteger& Mults, const Standard_Integer Deg, const Standard_Real Tol3d, const Standard_Real Tol2d, const Standard_Integer NbIterations = 1);
  
  Standard_EXPORT GeomInt_MyBSplGradientOfTheComputeLineOfWLApprox(const GeomInt_TheMultiLineOfWLApprox& SSP, const Standard_Integer FirstPoint, const Standard_Integer LastPoint, const Handle(AppParCurves_HArray1OfConstraintCouple)& TheConstraints, math_Vector& Parameters, const TColStd_Array1OfReal& Knots, const TColStd_Array1OfInteger& Mults, const Standard_Integer Deg, const Standard_Real Tol3d, const Standard_Real Tol2d, const Standard_Integer NbIterations, const Standard_Real lambda1, const Standard_Real lambda2);
  
  Standard_EXPORT Standard_Boolean IsDone() const;
  
  Standard_EXPORT AppParCurves_MultiBSpCurve Value() const;
  
  Standard_EXPORT Standard_Real Error (const Standard_Integer Index) const;
  
  Standard_EXPORT Standard_Real MaxError3d() const;
  
  Standard_EXPORT Standard_Real MaxError2d() const;
  
  Standard_EXPORT Standard_Real AverageError() const;




protected:

  
  Standard_EXPORT void Perform (const GeomInt_TheMultiLineOfWLApprox& SSP, const Standard_Integer FirstPoint, const Standard_Integer LastPoint, const Handle(AppParCurves_HArray1OfConstraintCouple)& TheConstraints, math_Vector& Parameters, const TColStd_Array1OfReal& Knots, const TColStd_Array1OfInteger& Mults, const Standard_Integer Deg, const Standard_Real Tol3d, const Standard_Real Tol2d, const Standard_Integer NbIterations = 200);




private:



  AppParCurves_MultiBSpCurve SCU;
  math_Vector ParError;
  Standard_Real AvError;
  Standard_Real MError3d;
  Standard_Real MError2d;
  Standard_Real mylambda1;
  Standard_Real mylambda2;
  Standard_Boolean myIsLambdaDefined;
  Standard_Boolean Done;


};







#endif // _GeomInt_MyBSplGradientOfTheComputeLineOfWLApprox_HeaderFile
