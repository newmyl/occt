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

//  Modified by skv - Wed Jun  2 11:49:59 2004 OCC5898

#include <Adaptor3d_HCurve.hxx>
#include <AdvApprox_ApproxAFunction.hxx>
#include <GeomAPI_CurveChartParameter.hxx>
#include <BSplCLib.hxx>
#include <Extrema_ExtPC.hxx>
#include <Extrema_LocateExtPC.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_Curve.hxx>
#include <GeomAdaptor_Curve.hxx>
#include <GeomAdaptor_HCurve.hxx>
#include <GeomLib_MakeCurvefromApprox.hxx>
#include <Precision.hxx>
#include <Standard_ConstructionError.hxx>
#include <Standard_OutOfRange.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <Law_Interpolate.hxx>
#include <Law_BSpline.hxx>
#include <gp_Lin.hxx>
#include <gce_MakeLin.hxx>
#include <ElCLib.hxx>
//=======================================================================
//class    : GeomAPI_SameParameter_Evaluator
//purpose  : Used in same parameterization curve approximation.
//=======================================================================
class GeomAPI_CurveChartParameter_Evaluator : public AdvApprox_EvaluatorFunction
{
public:
  GeomAPI_CurveChartParameter_Evaluator (const Handle(Law_BSpline)& theReparLaw,
    const Handle(Geom_Curve)& theCurve3d)
    : myReparLaw(theReparLaw),
    myCurve3d(theCurve3d) {}

  virtual void Evaluate (Standard_Integer *Dimension,
    Standard_Real     StartEnd[2],
    Standard_Real    *Parameter,
    Standard_Integer *DerivativeRequest,
    Standard_Real    *Result, // [Dimension]
    Standard_Integer *ErrorCode);

private:
  Handle(Law_BSpline) myReparLaw;
  Handle(Geom_Curve) myCurve3d;
};

//=======================================================================
//function : Evaluate
//purpose  : 
//=======================================================================
void GeomAPI_CurveChartParameter_Evaluator::Evaluate (Standard_Integer *,/*Dimension*/
                                                     Standard_Real    /*StartEnd*/[2],
                                                     Standard_Real    *Parameter,
                                                     Standard_Integer *DerivativeRequest,
                                                     Standard_Real    *Result,
                                                     Standard_Integer *ReturnCode) 
{
  Standard_Real eval_result[2];

  // Evaluate the 1D B-Spline that represents the change in parameterization.

  myReparLaw->D1(*Parameter, eval_result[0], eval_result[1]);

  gp_Pnt aPoint;
  if (*DerivativeRequest == 0)
  {
    myCurve3d->D0(eval_result[0], aPoint);
    aPoint.Coord(Result[0],Result[1], Result[2]);
  }
  else if (*DerivativeRequest == 1)
  {
    gp_Vec aVector;
    myCurve3d->D1(eval_result[0], aPoint, aVector);
    aVector.Multiply(eval_result[1]);
    aVector.Coord(Result[0], Result[1], Result[2]);
  }

  ReturnCode[0] = 0;
}

//=======================================================================
//function : ProjectPointOnCurve
//purpose  : 
//=======================================================================
static void ProjectPointOnCurve(const Standard_Real      InitValue,
                                const gp_Pnt             APoint,
                                const Standard_Real      Tolerance,
                                const Standard_Integer   NumIteration,
                                const Adaptor3d_Curve&   Curve,
                                Standard_Boolean&        Status,
                                Standard_Real&           Result)
{
  Standard_Integer num_iter = 0, not_done = 1, ii;

  gp_Pnt a_point;
  gp_Vec vector, d1, d2;
  Standard_Real func, func_derivative,
    param = InitValue;
  Status = Standard_False;
  do
  {
    num_iter++;
    Curve.D2(param, a_point, d1, d2);
    for (ii = 1 ; ii <= 3 ; ii++) 
      vector.SetCoord(ii, APoint.Coord(ii) - a_point.Coord(ii));

    func = vector.Dot(d1);
    if ( Abs(func) < Tolerance * d1.Magnitude())
    {
      not_done = 0;
      Status = Standard_True;
    }
    else
    {
      func_derivative = vector.Dot(d2) - d1.Dot(d1);

      // Avoid division by zero.
      const Standard_Real Toler = 1.0e-12;
      if( Abs(func_derivative) > Toler )
        param -= func / func_derivative;

      param = Max(param,Curve.FirstParameter());
      param = Min(param,Curve.LastParameter());
    }
  } while (not_done && num_iter <= NumIteration);

  Result = param;
}

//=======================================================================
//function : ComputeTolReached
//purpose  :
//=======================================================================
static Standard_Real ComputeTolReached(const Handle(Geom_Curve)& c3d,
                  const GeomAPI_CurveChartParameter::CurveChart& CrvChart)
{
  Standard_Real d2 = 0.0; // Square max discrete deviation.
  Standard_Integer nbp = CrvChart.myNbPnts;
  for(Standard_Integer i = 1; i <= nbp; i++)
  {
    Standard_Real t = CrvChart.myParams->Value(i);
    gp_Pnt Pc3d = c3d->Value(t);
    const gp_Pnt& Pchart = CrvChart.myPoints->Value(i);
    d2 = Max(d2, Pc3d.SquareDistance(Pchart));
  }
  d2 = Sqrt(d2);
  d2 = Max(d2, Precision::Confusion()); // Tolerance in modeling space.
  return d2;
}
//=======================================================================
//function : FindChartParameter
//purpose  : Find parameter of point for curve chart.
//=======================================================================
static Standard_Real FindChartParameter(
                  const GeomAPI_CurveChartParameter::CurveChart& CrvChart, 
                  const gp_Pnt& Pc3d,
                  Standard_Integer& Ind)
{
  Standard_Integer i, imin = 0;
  Standard_Real dmin = RealLast(), anLmin = 0.;
  Standard_Integer ibeg = Max(Ind, 1);
  for(i = ibeg; i <= CrvChart.myNbPnts - 1; i++)
  {
    gp_Lin aLin = gce_MakeLin(gp_Pnt(CrvChart.myPoints->Value(i)), 
      gp_Pnt(CrvChart.myPoints->Value(i+1)));
    Standard_Real t = ElCLib::Parameter(aLin, Pc3d);
    Standard_Real anL = (CrvChart.myPoints->Value(i) - CrvChart.myPoints->Value(i+1)).Modulus();
    if(t >= 0. - Epsilon(1.) && t <= anL + Epsilon(anL))
    {
      Standard_Real d = aLin.SquareDistance(Pc3d);
      if(d < dmin)
      {
        dmin = d;
        imin = i;
        anLmin = anL;
      }
    }
  }
  //
  if(imin == 0)
  {
    Ind = 0;
    return Precision::Infinite();
  }
  //
  gp_Vec aV(Pc3d.XYZ() - CrvChart.myPoints->Value(imin));
  gp_Vec aT(CrvChart.myTangents->Value(imin));
  Standard_Real par = aT.Dot(aV);
  gp_Vec aV1(CrvChart.myPoints->Value(imin+1) - CrvChart.myPoints->Value(imin));
  Standard_Real aLen = aT.Dot(aV1);
  par = CrvChart.myParams->Value(imin) + par/aLen * anLmin * CrvChart.myScales->Value(imin);
  //
  Ind = imin;
  return par;
}
//=======================================================================
//function : Check
//purpose  : Check current interpolation for validity.
//=======================================================================
static Standard_Boolean Check(const Handle(Law_BSpline)& ReparLaw, 
                              const Handle(Geom_Curve)& c3d,
                              const Standard_Integer nbp,
                    const GeomAPI_CurveChartParameter::CurveChart& CrvChart,
                              Standard_Real& tol)
{

  Standard_Real FirstPar = CrvChart.myParams->Value(1);
  Standard_Real LastPar  = CrvChart.myParams->Value(CrvChart.myNbPnts);
  Standard_Real aParamFirst = c3d->FirstParameter();
  Standard_Real aParamLast = c3d->LastParameter();
  Standard_Real epsf = Epsilon(Max(Abs(aParamFirst), 1.));
  Standard_Real epsl = Epsilon(Max(Abs(aParamLast), 1.));

  const Standard_Integer nn = 2 * nbp;
  const Standard_Real unsurnn = 1.0/nn;
  tol = 0.;
  Standard_Real parcouple[2] = {aParamLast, aParamLast};
  Standard_Integer Iloc = 0;
  Standard_Integer Ind = 0;
  for(Standard_Integer i = 0; i <= nn; i++)
  {
    Standard_Real t = unsurnn*i;
    Standard_Real tchart = FirstPar*(1.-t) + LastPar*t;
    BSplCLib::Hunt(CrvChart.myParams->Array1(), tchart, Iloc);
    Standard_Real tc3d = ReparLaw->Value(tchart);
    if (tc3d < aParamFirst - epsf || tc3d > aParamLast + epsl)
    {
      return Standard_False;
    }
    if(i < 2)
    {
      parcouple[i] = tc3d;
    }
    else
    {
      parcouple[0] = parcouple[1];
      parcouple[1] = tc3d;
    }
    if(parcouple[1] < parcouple[0] )
    {
      // tc3d(tchart) must be strictly monotone increasing function
      return Standard_False;
    }
    gp_Pnt Pc3d = c3d->Value(tc3d);
    Standard_Real uchart = FindChartParameter(CrvChart, Pc3d, Ind);
    Standard_Real temp = Abs(tchart - uchart);
    if(temp > tol) tol = temp;
    if (Iloc != Ind)
    {
      return Standard_False;
    }
  }
  //
  return Standard_True;
}

//=======================================================================
//function : Approx_CurveChartParameter
//purpose  : 
//=======================================================================
GeomAPI_CurveChartParameter::
  GeomAPI_CurveChartParameter(const Handle(Geom_BSplineCurve)&   C3D,
                    const GeomAPI_CurveChartParameter::CurveChart& CrvChart,
                    const Standard_Real         Tol)
  : myCurveChartParameter(Standard_True),
    myDone(Standard_False),
    myTolReached(Precision::Infinite()),
    myCrvChart(CrvChart)
{
  myReparCurve = C3D;
  Build(Tol);
}


//=======================================================================
//function : Build
//purpose  : 
//=======================================================================
void GeomAPI_CurveChartParameter::Build(const Standard_Real Tolerance)
{
  const Standard_Real anErrorMAX = 1.0e15;
  const Standard_Integer aMaxArraySize = 1000;
  Standard_Integer aNbPoints = 1000;

  Standard_Integer ii ;
  Standard_Real fc3d = myReparCurve->FirstParameter();
  Standard_Real lc3d = myReparCurve->LastParameter();
  Standard_Real fpar = myCrvChart.myParams->Value(1);
  Standard_Real lpar = myCrvChart.myParams->Value(myCrvChart.myNbPnts);
  if(Abs(fc3d - fpar) > Precision::PConfusion() ||
     Abs(lc3d - lpar) > Precision::PConfusion())
  {
    //reparametrize
    myReparCurve = Handle(Geom_BSplineCurve)::DownCast(myReparCurve->Copy());
    TColStd_Array1OfReal aKnots(1, myReparCurve->NbKnots());
    myReparCurve->Knots(aKnots);
    BSplCLib::Reparametrize(fpar, lpar, aKnots);
    myReparCurve->SetKnots(aKnots);
    fc3d = myReparCurve->FirstParameter();
    lc3d = myReparCurve->LastParameter();
  }

  const Standard_Real Tol = Tolerance;
  const Standard_Real Tol2 = Tol * Tol;
  Standard_Real deltamin = Precision::PConfusion();

  Standard_Real besttol2 = Tol2;

  Standard_Boolean interpolok = 0;
  Standard_Real tolsov = Precision::Infinite();

  Standard_Real qpchart[aMaxArraySize], qnewpchart[aMaxArraySize], 
                qpc3d[aMaxArraySize], qnewpc3d[aMaxArraySize];
  Standard_Real * pchart = qpchart;
  Standard_Real * newpchart = qnewpchart;
  Standard_Real * pc3d = qpc3d; 
  Standard_Real * newpc3d = qnewpc3d;

  Standard_Integer ncontrol = myCrvChart.myNbPnts - 1;
  for ( ii = 0 ; ii <= ncontrol; ii++) {
    pchart[ii] = myCrvChart.myParams->Value(ii+1);
    pc3d[ii]  = pchart[ii];
  }

  // Check existing same parameter state.
  GeomAdaptor_Curve GAReparCurve(myReparCurve);
  Extrema_LocateExtPC Projector;
  Projector.Initialize(GAReparCurve, fc3d, lc3d, Tol);

  Standard_Integer count = 1;
  Standard_Real previousp = fc3d, initp=0, curp;
  Standard_Real bornesup = lc3d - deltamin;
  Standard_Boolean projok = 0,  use_parameter ;
  Standard_Real dmax2 = -RealLast();
  ii = 0;
  const gp_Pnt& Pchart = myCrvChart.myPoints->Value(ii+1);
  gp_Pnt Pc3d;
  myReparCurve->D0(pc3d[ii], Pc3d);
  Standard_Real dist2 = Pchart.SquareDistance(Pc3d);
  if(dist2 > dmax2) dmax2 = dist2;
  for (ii = 1; ii < ncontrol; ii++)
  {    
    const gp_Pnt& Pchart = myCrvChart.myPoints->Value(ii+1);
    myReparCurve->D0(pc3d[ii], Pc3d);
    dist2 = Pchart.SquareDistance(Pc3d);
    use_parameter = (dist2 <= Tol2  && (pc3d[ii] > pc3d[count-1] + deltamin)) ;
    Standard_Real aDistMin = RealLast();
    if(use_parameter) {

      if(dist2 > dmax2) dmax2 = dist2;
      initp = previousp = pc3d[count] = pc3d[ii];
      pchart[count] = pchart[ii];
      count++;

    }
    else {
      if(!projok) initp = pc3d[ii];
      projok = myCurveChartParameter = Standard_False;
      Projector.Perform(Pchart, initp);
      if (Projector.IsDone()) {
        curp = Projector.Point().Parameter();
        aDistMin = Projector.SquareDistance();
        if(aDistMin <= Tol2)
        {
          projok = Standard_True;
        }
      }
      else
      {
        ProjectPointOnCurve(initp, Pchart, Tol, 30, GAReparCurve, projok, curp);
        if(projok)
        {
          gp_Pnt ap1 =myReparCurve->Value(curp);
          aDistMin = Pchart.SquareDistance(ap1);
          if(aDistMin > Tol2)
          {
            projok = Standard_False;
          }
        }
      }
      projok = (projok && (curp > previousp + deltamin && curp < bornesup));
      if(projok)
      {
        initp = previousp = pc3d[count] = curp;
        pchart[count] = pchart[ii];
        count++;
      }
      else
      {
        Extrema_ExtPC PR(Pchart, GAReparCurve, fc3d, lc3d, Tol);
        if(PR.IsDone())
        {
          const Standard_Integer aNbExt = PR.NbExt();
          if(aNbExt > 0)
          {
            Standard_Integer anIndMin = 0;
            Standard_Real aCurDistMin = RealLast();
            for(Standard_Integer i = 1; i <= aNbExt; i++)
            {
              const gp_Pnt &aP = PR.Point(i).Value();
              Standard_Real aDist2 = aP.SquareDistance(Pchart);
              if(aDist2 < aCurDistMin)
              {
                aCurDistMin = aDist2;
                anIndMin = i;
              }
            }
            if(anIndMin)
            {
              curp = PR.Point(anIndMin).Parameter();
              if( curp > previousp + deltamin && curp < bornesup)
              {
                aDistMin = aCurDistMin;
                initp = previousp = pc3d[count] = curp;
                pchart[count] = pchart[ii];
                count++;
                projok = Standard_True;
              }
            }
          }
        }
      }

      if(projok && besttol2 < aDistMin)
      {
        besttol2 = aDistMin;
      }
      else if(!projok)
      {
        //Projector
#ifdef OCCT_DEBUG
        cout << "Projection not done" << endl;
#endif
      }
    }
  }
  //
  pchart[count] = lc3d;
  pc3d[count]  = lc3d;
  ii = count;
  const gp_Pnt& Pchart1 = myCrvChart.myPoints->Value(ii+1);
  myReparCurve->D0(pc3d[ii], Pc3d);
  dist2 = Pchart1.SquareDistance(Pc3d);
  if(dist2 > dmax2) dmax2 = dist2;
  //
  if(myCurveChartParameter){
    myTolReached = 1.5*sqrt(dmax2);
    return;
  }
  //
  // There is at least one point where same parameter is broken.
  Standard_Boolean hasCountChanged = Standard_False;
  do
  {
    // The tables and their limits for the interpolation.
    TColStd_Array1OfReal    Paramc3d(*pc3d,1,count+1);
    TColStd_Array1OfReal    Paramchart(*pchart,1,count+1);
    Handle(TColStd_HArray1OfReal) aPar3d = new TColStd_HArray1OfReal(Paramc3d);
    Handle(TColStd_HArray1OfReal) aParChart = new TColStd_HArray1OfReal(Paramchart);
    //
    Law_Interpolate anInterpol(aPar3d, aParChart, Standard_False, Precision::PConfusion());
    anInterpol.Perform();
    if(!anInterpol.IsDone())
    {
      myDone = Standard_False;
      myCurveChartParameter = Standard_False;
      return;
    }

    myReparLaw = anInterpol.Curve();
    Standard_Integer newcount = 0;
    Standard_Integer Ind = 0;
    for (ii = 0; ii < count; ii++) {

      newpchart[newcount] = pchart[ii];
      newpc3d[newcount] = pc3d[ii];
      newcount++;

      if (count - ii + newcount == aNbPoints) continue;

      Standard_Real parc3d = myReparLaw->Value(.5*(pchart[ii]+pchart[ii+1]));

      if(parc3d < pc3d[ii] || parc3d > pc3d[ii+1]) {
        Standard_Real uc3d  = 0.5*(pc3d[ii]+pc3d[ii+1]);
        gp_Pnt aPc3d = myReparCurve->Value(uc3d);
        Standard_Real uchart = FindChartParameter(myCrvChart, aPc3d, Ind);

        if(uchart > pchart[ii] + deltamin && uchart < pchart[ii+1] - deltamin){
          newpc3d[newcount] = uc3d;
          newpchart[newcount] = uchart;
          newcount ++;
        }
      }
    }

    newpc3d[newcount] = pc3d[count];
    newpchart[newcount] = pchart[count];
    Standard_Real * temp;
    temp = pc3d;
    pc3d = newpc3d;
    newpc3d = temp;
    temp = pchart;
    pchart = newpchart;
    newpchart = temp;

    if ((count != newcount) && newcount < aNbPoints)
    {
      hasCountChanged = Standard_True;
      count = newcount;
      continue;
    }

    count = newcount;

    Standard_Real algtol = sqrt(besttol2);
    Standard_Real partol;
    myReparCurve->Resolution(algtol, partol);

    interpolok = Check (myReparLaw,  myReparCurve, count+1, myCrvChart, partol);

    tolsov = algtol;

    interpolok = (interpolok || // Good result.
      count >= aNbPoints - 1); // Number of points.

    if(interpolok) {
      Standard_Real besttol = sqrt(besttol2);

      Handle(TColStd_HArray1OfReal) tol1d,tol2d,tol3d;
      tol3d = new TColStd_HArray1OfReal(1,1) ;
      tol3d->SetValue(1, Tolerance);
      GeomAPI_CurveChartParameter_Evaluator ev (myReparLaw, myReparCurve); 
      AdvApprox_ApproxAFunction  anApproximator(0, 0, 1, tol1d, tol2d, tol3d, fc3d, lc3d,
        GeomAbs_C1, 15, 1000, ev);

      if (anApproximator.IsDone() || anApproximator.HasResult()) {
        GeomLib_MakeCurvefromApprox  aCurveBuilder(anApproximator);
        Handle(Geom_BSplineCurve) aNewCurve = aCurveBuilder.Curve(1);

        myTolReached = ComputeTolReached(aNewCurve, myCrvChart);

        if(myTolReached > anErrorMAX)
        {
          //This tolerance is too big. Probably, we will not be able to get 
          //edge with sameparameter in this case.

          myDone = Standard_False;
          return;
        }

        if( (myTolReached < 250.0*besttol) || 
          (count >= aNbPoints - 2) ||
          !hasCountChanged) //if count does not change after adding new point
          //(else we can have circularity)
        {
          myReparCurve = aNewCurve;
          myDone = Standard_True;
        }
        else
        {
          interpolok = Standard_False;
        }
      } 
    }

    if(!interpolok)
    {

      newcount = 0;
      Standard_Integer Ind = 0;
      for(Standard_Integer n = 0; n < count; n++){
        newpc3d[newcount] = pc3d[n];
        newpchart[newcount] = pchart[n];
        newcount ++;

        if (count - n + newcount == aNbPoints) continue;

        Standard_Real uc3d  = 0.5*(pc3d[n]+pc3d[n+1]);
        gp_Pnt aPc3d = myReparCurve->Value(uc3d);
        curp = FindChartParameter(myCrvChart, aPc3d, Ind);

         if(curp > pchart[n] + deltamin && curp < pchart[n+1] - deltamin){
          newpc3d[newcount] = uc3d;
          newpchart[newcount] = curp;
          newcount ++;
        }
      }
      newpc3d[newcount] = pc3d[count];
      newpchart[newcount] = pchart[count];
      Standard_Real * tempx;
      tempx = pc3d;
      pc3d = newpc3d;
      newpc3d = tempx;
      tempx = pchart;
      pchart = newpchart;
      newpchart = tempx;

      if(count != newcount)
      {
        count = newcount;
        hasCountChanged = Standard_True;
      }
      else
      {
        hasCountChanged = Standard_False;
      }
    }
  } while(!interpolok && hasCountChanged);

  if(myTolReached <= Tolerance)
  {
    myCurveChartParameter = Standard_True;
  }
  myDone = Standard_True;
  
}

//=======================================================================
//function : ReparCurve
//purpose  : 
//=======================================================================
const Handle(Geom_BSplineCurve)& GeomAPI_CurveChartParameter::ReparCurve() const
{
  return myReparCurve;
}

//=======================================================================
//function : ReparLaw
//purpose  : 
//=======================================================================
const Handle(Law_BSpline)& GeomAPI_CurveChartParameter::ReparLaw() const
{
  return myReparLaw;
}

//=======================================================================
//function : IsDone
//purpose  : 
//=======================================================================
Standard_Boolean GeomAPI_CurveChartParameter::IsDone() const
{
  return myDone;
}

//=======================================================================
//function : TolReached
//purpose  : 
//=======================================================================
Standard_Real GeomAPI_CurveChartParameter::TolReached() const
{
  return myTolReached;
}

//=======================================================================
//function : IsCurveChartParameter
//purpose  : 
//======================================================================= 
Standard_Boolean GeomAPI_CurveChartParameter::IsCurveChartParameter() const
{
  return myCurveChartParameter;
}

//=======================================================================
//function : ReparWithLaw
//purpose  : 
//======================================================================= 
Standard_Boolean   
GeomAPI_CurveChartParameter::ReparWithLaw (
                  const Handle(Geom_BSplineCurve)& theCrv, 
                  const Handle(Law_BSpline)& theReparLaw,
                  const Standard_Real theTol,
                  Handle(Geom_BSplineCurve)& theReparCurve,
                  Standard_Real& theTolReached )
{
  Handle(TColStd_HArray1OfReal) tol1d,tol2d,tol3d;
  tol3d = new TColStd_HArray1OfReal(1,1) ;
  tol3d->SetValue(1, theTol);
  theReparCurve = Handle(Geom_BSplineCurve)::DownCast(theCrv->Copy());
  Standard_Real f = theReparLaw->FirstParameter(),
    l = theReparLaw->LastParameter();
  Standard_Real tfirst = theReparLaw->Value(f),
    tlast = theReparLaw->Value(l);
  if(Abs(tfirst - theCrv->FirstParameter()) > Precision::PConfusion() ||
     Abs(tlast - theCrv->LastParameter()) > Precision::PConfusion())
  {
    //reparametrize
    TColStd_Array1OfReal aKnots(1, theReparCurve->NbKnots());
    theReparCurve->Knots(aKnots);
    BSplCLib::Reparametrize(tfirst, tlast, aKnots);
    theReparCurve->SetKnots(aKnots);
  }
  //
  GeomAPI_CurveChartParameter_Evaluator ev (theReparLaw, theReparCurve); 
  AdvApprox_ApproxAFunction  anApproximator(0, 0, 1, tol1d, tol2d, tol3d, f, l,
    GeomAbs_C1, 15, 1000, ev);

  if (anApproximator.IsDone() || anApproximator.HasResult()) {
    GeomLib_MakeCurvefromApprox  aCurveBuilder(anApproximator);
    theReparCurve = aCurveBuilder.Curve(1);
    theTolReached = anApproximator.MaxError(3, 1);
    return Standard_True;
  }
  return Standard_False;
}