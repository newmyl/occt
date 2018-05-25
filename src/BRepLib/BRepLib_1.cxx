// Created on: 2017-03-24
// Created by: Mikhail Sazonov
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

#include <BRepLib.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <Geom_OffsetCurve.hxx>
#include <Precision.hxx>
#include <TopExp.hxx>
#include <TopoDS_Vertex.hxx>
#include <BRepCheck_Edge.hxx>
#include <BRepAdaptor_HSurface.hxx>
#include <GeomAdaptor_HSurface.hxx>
#include <BRepAdaptor_HCurve.hxx>
#include <GeomAdaptor_HCurve.hxx>
#include <ProjLib_ProjectedCurve.hxx>
#include <AppParCurves_Constraint.hxx>
#include <ProjLib.hxx>
#include <GeomLib_CheckCurveOnSurface.hxx>
#include <GeomAdaptor.hxx>
#include <Adaptor3d_CurveOnSurface.hxx>
#include <Geom2dAdaptor_HCurve.hxx>
#include <GeomLib.hxx>
#include <BRep_TEdge.hxx>
//=======================================================================
// function: findNearestValidPoint
// purpose : Starting from the appointed end of the curve, find the nearest
//           point on the curve that is an intersection with the sphere with
//           center theVertPnt and radius theTol.
//=======================================================================
static Standard_Boolean findNearestValidPoint(
  const Adaptor3d_Curve& theCurve,
  const Standard_Real theFirst, const Standard_Real theLast,
  const Standard_Boolean isFirst,
  const gp_Pnt& theVertPnt,
  const Standard_Real theTol,
  const Standard_Real theEps,
  Standard_Real& thePar)
{
  // 1. Check that the needed end is inside the sphere

  Standard_Real aStartU = theFirst;
  Standard_Real anEndU = theLast;
  if (!isFirst)
    std::swap(aStartU, anEndU);
  gp_Pnt aP = theCurve.Value(aStartU);
  const Standard_Real aSqTol = theTol * theTol;
  if (aP.SquareDistance(theVertPnt) > aSqTol)
    // the vertex does not cover the corresponding to this vertex end of the curve
    return Standard_False;

  // 2. Find a nearest point that is outside

  // stepping along the curve by theTol till go out
  //
  // the general step is computed using general curve resolution
  Standard_Real aStep = theCurve.Resolution(theTol) * 1.01;
  // aD1Mag is a threshold to consider local derivative magnitude too small
  // and to accelerate going out of sphere
  // (inverse of resolution is the maximal derivative);
  // this is actual for bezier and b-spline types only
  Standard_Real aD1Mag = 0.;
  GeomAbs_CurveType aType = theCurve.GetType();
  if (aType == GeomAbs_OffsetCurve)
  {
    Handle(Geom_OffsetCurve) anOffsetCurve = theCurve.OffsetCurve();
    Handle(Geom_Curve) aBaseCurve = anOffsetCurve->BasisCurve();
    aType = GeomAdaptor_Curve(aBaseCurve).GetType();
  }
  if (aType == GeomAbs_BezierCurve || aType == GeomAbs_BSplineCurve)
  {
    aD1Mag = 1. / theCurve.Resolution(1.) * 0.01;
    aD1Mag *= aD1Mag;
  }
  if (!isFirst)
    aStep = -aStep;
  Standard_Boolean isOut = Standard_False;
  Standard_Real anUIn = aStartU;
  Standard_Real anUOut = anUIn;
  while (!isOut)
  {
    anUIn = anUOut;
    anUOut += aStep;
    if ((isFirst && anUOut > anEndU) || (!isFirst && anUOut < anEndU))
    {
      // step is too big and we go out of bounds,
      // check if the opposite bound is outside
      aP = theCurve.Value(anEndU);
      isOut = (aP.SquareDistance(theVertPnt) > aSqTol);
      if (!isOut)
        // all range is inside sphere
        return Standard_False;
      anUOut = anEndU;
      break;
    }
    if (aD1Mag > 0.)
    {
      Standard_Real aStepLocal = aStep;
      for (;;)
      {
        // cycle to go out of local singularity
        gp_Vec aD1;
        theCurve.D1(anUOut, aP, aD1);
        if (aD1.SquareMagnitude() < aD1Mag)
        {
          aStepLocal *= 2.;
          anUOut += aStepLocal;
          if ((isFirst && anUOut < anEndU) || (!isFirst && anUOut > anEndU))
            // still in range
            continue;
          // went out of range, so check if the end point has out state
          anUOut = anEndU;
          aP = theCurve.Value(anUOut);
          isOut = (aP.SquareDistance(theVertPnt) > aSqTol);
          if (!isOut)
            // all range is inside sphere
            return Standard_False;
        }
        break;
      }
    }
    else
    {
      aP = theCurve.Value(anUOut);
    }
    if (!isOut)
      isOut = (aP.SquareDistance(theVertPnt) > aSqTol);
  }

  // 3. Precise solution with binary search

  Standard_Real aDelta = Abs(anUOut - anUIn);
  while (aDelta > theEps)
  {
    Standard_Real aMidU = (anUIn + anUOut) * 0.5;
    aP = theCurve.Value(aMidU);
    isOut = (aP.SquareDistance(theVertPnt) > aSqTol);
    if (isOut)
      anUOut = aMidU;
    else
      anUIn = aMidU;
    aDelta = Abs(anUOut - anUIn);
  }
  thePar = (anUIn + anUOut) * 0.5;
  return Standard_True;
}

//=======================================================================
// function: FindValidRange
// purpose : 
//=======================================================================
Standard_Boolean BRepLib::FindValidRange
  (const Adaptor3d_Curve& theCurve, const Standard_Real theTolE,
   const Standard_Real theParV1, const gp_Pnt& thePntV1, const Standard_Real theTolV1,
   const Standard_Real theParV2, const gp_Pnt& thePntV2, const Standard_Real theTolV2,
   Standard_Real& theFirst, Standard_Real& theLast)
{
  if (theParV2 - theParV1 < Precision::PConfusion())
    return Standard_False;
  
  Standard_Real anEps = Max(theCurve.Resolution(theTolE) * 0.1, Precision::PConfusion());

  if (Precision::IsInfinite(theParV1))
    theFirst = theParV1;
  else
  {
    if (!findNearestValidPoint(theCurve, theParV1, theParV2, Standard_True,
                               thePntV1, theTolV1, anEps, theFirst))
      return Standard_False;
    if (theParV2 - theFirst < anEps)
      return Standard_False;
  }

  if (Precision::IsInfinite(theParV2))
    theLast = theParV2;
  else
  {
    if (!findNearestValidPoint(theCurve, theParV1, theParV2, Standard_False,
                               thePntV2, theTolV2, anEps, theLast))
      return Standard_False;
    if (theLast - theParV1 < anEps)
      return Standard_False;
  }

  // check found parameters
  if (theFirst > theLast)
  {
    // overlapping, not valid range
    return Standard_False;
  }

  return Standard_True;
}

//=======================================================================
// function: FindValidRange
// purpose : 
//=======================================================================
Standard_Boolean BRepLib::FindValidRange
  (const TopoDS_Edge& theEdge, Standard_Real& theFirst, Standard_Real& theLast)
{
  TopLoc_Location aLoc;
  Standard_Real f, l;
  if (BRep_Tool::Curve(theEdge, aLoc, f, l).IsNull())
    return Standard_False;
  BRepAdaptor_Curve anAC(theEdge);
  Standard_Real aParV[2] = { anAC.FirstParameter(), anAC.LastParameter() };
  if (aParV[1] - aParV[0] < Precision::PConfusion())
    return Standard_False;

  // get vertices
  TopoDS_Vertex aV[2];
  TopExp::Vertices(theEdge, aV[0], aV[1]);

  Standard_Real aTolE = BRep_Tool::Tolerance(theEdge);
  // to have correspondence with intersection precision
  // the tolerances of vertices are increased on Precision::Confusion()
  Standard_Real aTolV[2] = { Precision::Confusion(), Precision::Confusion() };
  gp_Pnt aPntV[2];
  for (Standard_Integer i = 0; i < 2; i++)
  {
    if (!aV[i].IsNull())
    {
      aTolV[i] += BRep_Tool::Tolerance(aV[i]);
      aPntV[i] = BRep_Tool::Pnt(aV[i]);
    }
    else if (!Precision::IsInfinite(aParV[i]))
    {
      aTolV[i] += aTolE;
      aPntV[i] = anAC.Value(aParV[i]);
    }
  }
  return FindValidRange(anAC, aTolE, 
                        aParV[0], aPntV[0], aTolV[0],
                        aParV[1], aPntV[1], aTolV[1],
                        theFirst, theLast);
}

//=======================================================================
//function : BuildPCurveForEdgeOnPlane
//purpose  : 
//=======================================================================
void BRepLib::BuildPCurveForEdgeOnPlane(const TopoDS_Edge& aE,
                                        const TopoDS_Face& aF)
{
  Standard_Boolean bToUpdate;
  Standard_Real aTolE;
  Handle(Geom2d_Curve) aC2D;
  BRep_Builder aBB;
  //
  BuildPCurveForEdgeOnPlane(aE, aF, aC2D, bToUpdate);
  if (bToUpdate) {
    aTolE = BRep_Tool::Tolerance(aE);
    aBB.UpdateEdge(aE, aC2D, aF, aTolE);
  }
}

//=======================================================================
//function : BuildPCurveForEdgeOnPlane
//purpose  : 
//=======================================================================
void BRepLib::BuildPCurveForEdgeOnPlane(const TopoDS_Edge& aE,
                                        const TopoDS_Face& aF,
                                        Handle(Geom2d_Curve)& aC2D,
                                        Standard_Boolean& bToUpdate)
{
  Standard_Real aT1, aT2;
  Standard_Boolean isStored;
  aC2D = BRep_Tool::CurveOnSurface(aE, aF, aT1, aT2, &isStored);
  bToUpdate = !isStored && !aC2D.IsNull();
}
//=======================================================================
//function : CompTol
//purpose  : 
//=======================================================================
static Standard_Real CompTol(const Handle(Geom_Curve)& theC3D, 
                             const Handle(Geom2d_Curve)& theC2D,
                             const Handle(Geom_Surface)& theS,
                             const Standard_Real theFirstPar,
                             const Standard_Real theLastPar)
{
  Standard_Real aTolRange = Max(Precision::PConfusion(),
    0.001 * (theLastPar - theFirstPar));
  GeomLib_CheckCurveOnSurface aCheckDist(theC3D, theS, theFirstPar, theLastPar, aTolRange);
  aCheckDist.Perform(theC2D);
  Standard_Real aTolR = 0.;
  if (aCheckDist.IsDone())
  {
    aTolR = aCheckDist.MaxDistance();
  }
  else
  {
    const Standard_Integer aNbPnts = 23;
    TColStd_Array1OfReal aPars(1, aNbPnts);
    Standard_Integer i;
    Standard_Real t, dt = (theLastPar - theFirstPar) / (aNbPnts - 1);
    aPars(1) = theFirstPar;
    aPars(aNbPnts) = theLastPar;
    for (i = 2, t = theFirstPar + dt; i < aNbPnts; ++i, t += dt)
    {
      aPars(i) = t;
    }
    Handle(Geom2dAdaptor_HCurve) aG2dAHC = new Geom2dAdaptor_HCurve(theC2D);
    GeomAdaptor_Curve aGAC(theC3D);
    Handle(GeomAdaptor_HSurface) aGAHS = new GeomAdaptor_HSurface(theS);
    Adaptor3d_CurveOnSurface aConS(aG2dAHC, aGAHS);
    GeomLib::EvalMaxParametricDistance(aConS, aGAC, 1., aPars, aTolR);
    aTolR *= 1.5; //possible deflection
  }

  return aTolR;
}
//=======================================================================
//function : UpdateTol
//purpose  : 
//=======================================================================
static void UpdateTol(const TopoDS_Edge& theE, const Standard_Real theTol)
{
  BRep_Builder aBB;
  TopoDS_Vertex aV1, aV2;
  TopExp::Vertices(theE, aV1, aV2);
  if (!aV1.IsNull())
  {
    aBB.UpdateVertex(aV1, theTol);
  }
  if (!aV2.IsNull())
  {
    aBB.UpdateVertex(aV2, theTol);
  }
  aBB.UpdateEdge(theE, theTol);
}

//=======================================================================
//function : SetPCurve
//purpose  : 
//=======================================================================
void BRepLib::SetPCurve(const TopoDS_Edge& theE,
                        const Handle(Geom2d_Curve)& theC,
                        const TopoDS_Face& theF, 
                        const Standard_Real theMaxTol,
                        Standard_Real& theTolReached, Handle(Geom2d_Curve)& theProjCurve)
{
  Standard_Real aSMTol = Precision::PConfusion();
  Standard_Real aTol = BRep_Tool::Tolerance(theE);
  BRep_Builder aBB;
  Standard_Real fr, lr, f, l;
  BRep_Tool::Range(theE, fr, lr);
  Handle(Geom2d_Curve) aC = theC;
  f = theC->FirstParameter();
  l = theC->LastParameter();
  if (!(Precision::IsInfinite(f) || Precision::IsInfinite(l)))
  {
    GeomLib::SameRange(aSMTol, theC, f, l, fr, lr, aC);
  }
  aBB.UpdateEdge(theE, aC, theF, aTol);
  Handle(Geom_Surface) anS = BRep_Tool::Surface(theF);
  Handle(Geom_Curve) aC3D = BRep_Tool::Curve(theE, f, l);
  Standard_Real aTol1 = CompTol(aC3D, aC, anS, fr, lr);
  if (aTol1 <= aTol)
  {
    theTolReached = aTol1;
    aBB.SameParameter(theE, Standard_True);
    return;
  }
  else if (theMaxTol < aTol && aTol1 < 2.*aTol)
  {
    theTolReached = aTol1;
    UpdateTol(theE, theTolReached);
    return;
  }
  aBB.SameParameter(theE, Standard_False);
  Standard_Real aNewTol = -1;
  BRepLib::SameParameter(theE, aTol, aNewTol, Standard_True);
  if (aNewTol > 0)
  {
    //Set old tolerance for edge, which has been changed by sameparameter
    static_cast<BRep_TEdge*>(theE.TShape().get())->Tolerance(aTol);
  }
  if (aNewTol > 0.)
  {
    aC = BRep_Tool::CurveOnSurface(theE, theF, f, l);
    aNewTol = CompTol(aC3D, aC, anS, fr, lr);
    if (aNewTol < theMaxTol)
    {
      aBB.SameParameter(theE, Standard_True);
      theTolReached = aNewTol;
      UpdateTol(theE, theTolReached);
      return ;
    }
  }
  //Projection
  Handle(BRepAdaptor_HSurface) aBAHS = 
    new BRepAdaptor_HSurface(BRepAdaptor_Surface(theF, Standard_False));
  Handle(BRepAdaptor_HCurve) aBAHC = new BRepAdaptor_HCurve(theE);
  ProjLib_ProjectedCurve aProjCurv(aBAHS);
  Standard_Integer aDegMin = -1, aDegMax = -1, aMaxSegments = -1;
  Standard_Real aMaxDist = Max(1.e3 * theMaxTol, aNewTol);
  Standard_Real aTR = Precision::Confusion();
  Standard_Real aMaxTol = 1.e3 * aTR; //0.0001
  Standard_Boolean isAnaSurf = ProjLib::IsAnaSurf(aBAHS);
  AppParCurves_Constraint aBndPnt = AppParCurves_TangencyPoint;
  if (theMaxTol >= aMaxTol || aNewTol > 10. * theMaxTol)
  {
    aTR = aMaxTol;
    if (aNewTol >= 1.)
    {
      aTR = Min(10. * aTR, theMaxTol);
    }
    if (!isAnaSurf )
    {
      aBndPnt = AppParCurves_PassPoint;
    }
  }
  //
  theTolReached = RealLast();
  aProjCurv.Load(aTR);
  aProjCurv.SetDegree(aDegMin, aDegMax);
  aProjCurv.SetMaxSegments(aMaxSegments);
  aProjCurv.SetBndPnt(aBndPnt);
  aProjCurv.SetMaxDist(aMaxDist);
  aProjCurv.Perform(aBAHC);
  ProjLib::MakePCurveOfType(aProjCurv, theProjCurve);
  if (!theProjCurve.IsNull())
  {
    Standard_Real pf = theProjCurve->FirstParameter(), 
                  pl = theProjCurve->LastParameter();
    if (!(Precision::IsInfinite(pf) || Precision::IsInfinite(pl)))
    {
      if (Abs(pf - fr) > aSMTol || Abs(pl - lr) > aSMTol)
      {
        aC.Nullify();
        GeomLib::SameRange(aSMTol, theProjCurve, pf, pl, fr, lr, aC);
        if (!aC.IsNull() && theProjCurve != aC)
        {
          theProjCurve = aC;
        }
      }
    }
    Standard_Real aTolR = CompTol(aC3D, theProjCurve, anS, fr, lr);
    //
    if ((aNewTol > 0. && aTolR < aNewTol) || aNewTol < 0.) 
    {
      theTolReached = aTolR;
      //
      //Set new pcurve
      aBB.UpdateEdge(theE, theProjCurve, theF, theTolReached);
      UpdateTol(theE, theTolReached);
      aBB.SameParameter(theE, Standard_True);
    }
    else
    {
      if (aNewTol > 0.)
      {
        theTolReached = aNewTol;
      }
      else
      {
        theTolReached = aTol1;
      }
      UpdateTol(theE, theTolReached);
      aBB.SameParameter(theE, Standard_True);
    }
  }
  else
  {
    if (aNewTol > 0.)
    {
      theTolReached = aNewTol;
    }
    else
    {
      theTolReached = aTol1;
    }
    UpdateTol(theE, theTolReached);
    aBB.SameParameter(theE, Standard_True);
  }
}
//=======================================================================
//function : SetPCurve
//purpose  : 
//=======================================================================
void BRepLib::SetPCurve(const TopoDS_Edge& theE,
  const Handle(Geom2d_Curve)& theC1, const Handle(Geom2d_Curve)& theC2,
  const TopoDS_Face& theF,
  const Standard_Real theMaxTol,
  Standard_Real& theTolReached, 
  Handle(Geom2d_Curve)& theProjCurve1,
  Handle(Geom2d_Curve)& theProjCurve2)
{
  Standard_Real aSMTol = Precision::PConfusion();
  Standard_Real aTol = BRep_Tool::Tolerance(theE);
  BRep_Builder aBB;
  Standard_Real fr, lr, f, l;
  BRep_Tool::Range(theE, fr, lr);
  Handle(Geom2d_Curve) aC1 = theC1, aC2 = theC2;
  f = theC1->FirstParameter();
  l = theC1->LastParameter();
  if (!(Precision::IsInfinite(f) || Precision::IsInfinite(l)))
  {
    GeomLib::SameRange(aSMTol, theC1, f, l, fr, lr, aC1);
  }
  f = theC2->FirstParameter();
  l = theC2->LastParameter();
  if (!(Precision::IsInfinite(f) || Precision::IsInfinite(l)))
  {
    GeomLib::SameRange(aSMTol, theC2, f, l, fr, lr, aC2);
  }
  aBB.UpdateEdge(theE, aC1, aC2, theF, aTol);
  Handle(Geom_Surface) anS = BRep_Tool::Surface(theF);
  Handle(Geom_Curve) aC3D = BRep_Tool::Curve(theE, f, l);
  Standard_Real aTol1 = CompTol(aC3D, aC1, anS, fr, lr);
  aTol1 = Max(aTol1, CompTol(aC3D, aC2, anS, fr, lr));
  if (aTol1 <= aTol)
  {
    theTolReached = aTol1;
    aBB.SameParameter(theE, Standard_True);
    return;
  }
  aBB.SameParameter(theE, Standard_False);
  Standard_Real aNewTol = -1;
  BRepLib::SameParameter(theE, aTol, aNewTol, Standard_False);
  if (aNewTol > 0)
  {
    //Set old tolerance for edge, which has been changed by sameparameter
    static_cast<BRep_TEdge*>(theE.TShape().get())->Tolerance(aTol);
  }
  if (aNewTol > 0. && aNewTol < theMaxTol)
  {
    TopoDS_Vertex aV1, aV2;
    TopExp::Vertices(theE, aV1, aV2);
    if (!aV1.IsNull())
    {
      aBB.UpdateVertex(aV1, aNewTol);
    }
    if (!aV2.IsNull())
    {
      aBB.UpdateVertex(aV2, aNewTol);
    }
    theTolReached = aNewTol;
    return;
  }
  //Projection
  Handle(BRepAdaptor_HSurface) aBAHS = new BRepAdaptor_HSurface(theF);
  Handle(BRepAdaptor_HCurve) aBAHC = new BRepAdaptor_HCurve(theE);
  ProjLib_ProjectedCurve aProjCurv(aBAHS);
  Standard_Integer aDegMin = -1, aDegMax = -1, aMaxSegments = -1;
  Standard_Real aMaxDist = Max(1.e3 * theMaxTol, aNewTol);
  Standard_Real aTR = Precision::Confusion();
  Standard_Real aMaxTol = 1.e3 * aTR; //0.0001
  Standard_Boolean isAnaSurf = ProjLib::IsAnaSurf(aBAHS);
  AppParCurves_Constraint aBndPnt = AppParCurves_TangencyPoint;
  if (theMaxTol >= aMaxTol || aNewTol > 10. * theMaxTol)
  {
    aTR = aMaxTol;
    if (aNewTol >= 1.)
    {
      aTR = Min(10. * aTR, theMaxTol);
    }
    if (!isAnaSurf)
    {
      aBndPnt = AppParCurves_PassPoint;
    }
  }
  //
  theTolReached = RealLast();
  aProjCurv.Load(aTR);
  aProjCurv.SetDegree(aDegMin, aDegMax);
  aProjCurv.SetMaxSegments(aMaxSegments);
  aProjCurv.SetBndPnt(aBndPnt);
  aProjCurv.SetMaxDist(aMaxDist);
  aProjCurv.Perform(aBAHC);
  Handle(Geom2d_Curve) aCProj;
  ProjLib::MakePCurveOfType(aProjCurv, aCProj);
  if (!aCProj.IsNull())
  {
    Standard_Real pf = aCProj->FirstParameter(),
      pl = aCProj->LastParameter();
    if (!(Precision::IsInfinite(pf) || Precision::IsInfinite(pl)))
    {
      if (Abs(pf - fr) > aSMTol || Abs(pl - lr) > aSMTol)
      {
        aC1.Nullify();
        GeomLib::SameRange(aSMTol, aCProj, pf, pl, fr, lr, aC1);
        if (!aC1.IsNull() && aCProj != aC1)
        {
          aCProj = aC1;
        }
      }
    }
    Standard_Real aTolR = CompTol(aC3D, aCProj, anS, fr, lr);
    //
    if ((aNewTol > 0. && aTolR < aNewTol) || aNewTol < 0.)
    {
      theTolReached = aTolR;
      //
      gp_Pnt2d aP1, aP2, aPProj;
      Handle(Geom2d_Curve) aPC = BRep_Tool::CurveOnSurface(theE, theF, f, l);
      aP1 = aPC->Value(f);
      aPC = BRep_Tool::CurveOnSurface(TopoDS::Edge(theE.Reversed()), theF, f, l);
      aP2 = aPC->Value(f);
      aPProj = aCProj->Value(f);
      Standard_Real Dist1, Dist2;
      Dist1 = aPProj.Distance(aP1);
      Dist2 = aPProj.Distance(aP2);
      if (Dist1 < Dist2)  {
        theProjCurve1 = aCProj;
        Handle(Geom2d_Geometry) GG = aCProj->Translated(aP1, aP2);
        theProjCurve2 = Handle(Geom2d_Curve)::DownCast(GG);
      }
      else {
        theProjCurve2 = aCProj;
        Handle(Geom2d_Geometry) GG = aCProj->Translated(aP2, aP1);
        theProjCurve1 = Handle(Geom2d_BSplineCurve)::DownCast(GG);
      }
      //Set new pcurves
      aBB.UpdateEdge(theE, theProjCurve1, theProjCurve2, theF, aTolR);
      UpdateTol(theE, theTolReached);
      aBB.SameParameter(theE, Standard_True);
    }
    else
    {
      if (aNewTol > 0.)
      {
        theTolReached = aNewTol;
      }
      else
      {
        theTolReached = aTol1;
      }
      UpdateTol(theE, theTolReached);
      aBB.SameParameter(theE, Standard_True);
    }
  }
  else
  {
    if (aNewTol > 0.)
    {
      theTolReached = aNewTol;
    }
    else
    {
      theTolReached = aTol1;
    }
    UpdateTol(theE, theTolReached);
    aBB.SameParameter(theE, Standard_True);
  }
}