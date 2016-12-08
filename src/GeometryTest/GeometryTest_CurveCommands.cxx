// Created on: 1993-08-12
// Created by: Bruno DUMORTIER
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

// 09/06/97 : JPI : suppression des commandes redondantes suite a la creation de GeomliteTest

#include <GeometryTest.hxx>
#include <Draw_Appli.hxx>
#include <DrawTrSurf.hxx>
#include <DrawTrSurf_Curve.hxx>
#include <DrawTrSurf_Curve2d.hxx>
#include <DrawTrSurf_BezierCurve.hxx>
#include <DrawTrSurf_BSplineCurve.hxx>
#include <DrawTrSurf_BezierCurve2d.hxx>
#include <DrawTrSurf_BSplineCurve2d.hxx>
#include <Draw_Marker3D.hxx>
#include <Draw_Marker2D.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <Draw_Color.hxx>
#include <Draw_Display.hxx>

#include <GeomAPI.hxx>
#include <GeomAPI_IntCS.hxx>
#include <GeomAPI_IntSS.hxx>

//#include <GeomLProp.hxx>
#include <GeomProjLib.hxx>
#include <BSplCLib.hxx>

#include <gp.hxx>
#include <gp_Pln.hxx>
#include <gp_Parab2d.hxx>
#include <gp_Elips2d.hxx>
#include <gp_Hypr2d.hxx>

#include <Geom_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_Hyperbola.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom2d_Parabola.hxx>
#include <Geom2d_Hyperbola.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <Geom2d_Curve.hxx>

#include <GccAna_Lin2dBisec.hxx>
#include <GccAna_Circ2dBisec.hxx>
#include <GccAna_CircLin2dBisec.hxx>
#include <GccAna_CircPnt2dBisec.hxx>
#include <GccAna_LinPnt2dBisec.hxx>
#include <GccAna_Pnt2dBisec.hxx>
#include <GccInt_Bisec.hxx>
#include <GccInt_IType.hxx>

#include <Geom_Plane.hxx>
#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Geom_TrimmedCurve.hxx>

#include <Law_BSpline.hxx>

#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>

#include <Adaptor3d_HCurve.hxx>
#include <Adaptor3d_HSurface.hxx>
#include <Adaptor3d_CurveOnSurface.hxx>

#include <GeomAdaptor_HCurve.hxx>
#include <GeomAdaptor_HSurface.hxx>
#include <GeomAdaptor.hxx>
#include <Geom2dAdaptor_HCurve.hxx>

#include <GeomAbs_SurfaceType.hxx>
#include <GeomAbs_CurveType.hxx>

#include <ProjLib_CompProjectedCurve.hxx>
#include <ProjLib_HCompProjectedCurve.hxx>
#include <Approx_CurveOnSurface.hxx>
#include <Precision.hxx>
#include <Geom2dAdaptor.hxx>


#include <Precision.hxx>

#include <Geom_Surface.hxx>
#include <Adaptor2d_HCurve2d.hxx>
#include <stdio.h>
#include <BSplCLib.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <GCPnts_QuasiUniformDeflection.hxx>
#include <GCPnts_UniformDeflection.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <GCPnts_DistFunction.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <gce_MakeLin.hxx>
#include <TColStd_Array1OfBoolean.hxx>
#include <GeomAdaptor_HSurface.hxx>
#include <Adaptor3d_TopolTool.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <Geom_BSplineSurface.hxx>
#include <DrawTrSurf_BSplineSurface.hxx>
#include <TColStd_HArray1OfReal.hxx>

//epa test
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <AIS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <GeomLProp_CLProps.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <DBRep.hxx>


#ifdef _WIN32
Standard_IMPORT Draw_Viewer dout;
#endif

//=======================================================================
//function : polecurve2d
//purpose  : 
//=======================================================================

static Standard_Integer polelaw (Draw_Interpretor& , Standard_Integer n, const char** a)
{
  Standard_Integer k,
  jj,
  qq,
  i;


  if (n < 3) return 1;
  Standard_Boolean periodic = Standard_False ;
  Standard_Integer deg = Draw::Atoi(a[2]);
  Standard_Integer nbk = Draw::Atoi(a[3]);
  
  TColStd_Array1OfReal    knots(1, nbk);
  TColStd_Array1OfInteger mults(1, nbk);
  k = 4;
  Standard_Integer Sigma = 0;
  for (i = 1; i<=nbk; i++) {
    knots( i) = Draw::Atof(a[k]);
    k++;
    mults( i) = Draw::Atoi(a[k]);
    Sigma += mults(i);
    k++;
  }

  Standard_Integer np;
  np = Sigma - deg  -1;
  TColStd_Array1OfReal flat_knots(1, Sigma) ;
  jj = 1 ;
  for (i = 1 ; i <= nbk ; i++) {
    for(qq = 1 ; qq <= mults(i) ; qq++) {
      flat_knots(jj) = knots(i) ;
      jj ++ ;
    }
  }
  
  TColgp_Array1OfPnt2d poles  (1, np);
  TColStd_Array1OfReal schoenberg_points(1,np) ;
  BSplCLib::BuildSchoenbergPoints(deg,
				  flat_knots,
				  schoenberg_points) ;
  for (i = 1; i <= np; i++) {
    poles(i).SetCoord(schoenberg_points(i),Draw::Atof(a[k]));
    k++;
  }
    
  Handle(Geom2d_BSplineCurve) result =
    new Geom2d_BSplineCurve(poles, knots, mults, deg, periodic);
  DrawTrSurf::Set(a[1],result);

  
  return 0;
}
//=======================================================================
//function : to2d
//purpose  : 
//=======================================================================

static Standard_Integer to2d (Draw_Interpretor& , Standard_Integer n, const char** a)
{
  if (n < 3) return 1;

  // get the curve
  Handle(Geom_Curve) C = DrawTrSurf::GetCurve(a[2]);
  if (C.IsNull())
    return 1;

  Handle(Geom_Surface) S;
  if (n >= 4) {
    S = DrawTrSurf::GetSurface(a[3]);
    if (S.IsNull()) return 1;
  }
  else
    S = new Geom_Plane(gp::XOY());
  
  Handle(Geom_Plane) P = Handle(Geom_Plane)::DownCast(S);
  if (P.IsNull()) return 1;
  Handle(Geom2d_Curve) r = GeomAPI::To2d(C,P->Pln());
  DrawTrSurf::Set(a[1],r);
  return 0;
}

//=======================================================================
//function : to3d
//purpose  : 
//=======================================================================

static Standard_Integer to3d (Draw_Interpretor& , Standard_Integer n, const char** a)
{
  if (n < 3) return 1;
  
  Handle(Geom2d_Curve) C = DrawTrSurf::GetCurve2d(a[2]);
  if (C.IsNull()) return 1;
  
  Handle(Geom_Surface) S;
  if (n >= 4) {
    S = DrawTrSurf::GetSurface(a[3]);
    if (S.IsNull()) return 1;
  }
  else
    S = new Geom_Plane(gp::XOY());
  
  Handle(Geom_Plane) P = Handle(Geom_Plane)::DownCast(S);
  if (P.IsNull()) return 1;
  Handle(Geom_Curve) r = GeomAPI::To3d(C,P->Pln());
  
  DrawTrSurf::Set(a[1],r);
  return 0;
}

//=======================================================================
//function : gproject
//purpose  : 
//=======================================================================


static Standard_Integer gproject(Draw_Interpretor& di, Standard_Integer n, const char** a)
{
  
  char newname[1024];
  char* temp = newname;
  char newname1[10];
  char* temp1 = newname1;
  char name[100];
  Standard_Integer ONE = 1;

  if (n == 3)
    Sprintf(name,"p");
  else if (n == 4) {
    Sprintf(name,"%s",a[1]);
    ONE = 2;
  }
  else {
   di << "gproject wait 2 or 3 arguments\n";
   return 1;
  } 

  Handle(Geom_Curve) Cur = DrawTrSurf::GetCurve(a[ONE]);
  Handle(Geom_Surface) Sur = DrawTrSurf::GetSurface(a[ONE+1]);
  if (Cur.IsNull() || Sur.IsNull()) return 1;

  Handle(GeomAdaptor_HCurve) hcur = new GeomAdaptor_HCurve(Cur);
  Handle(GeomAdaptor_HSurface) hsur = new GeomAdaptor_HSurface(Sur);


  Standard_Real myTol3d = 1.e-6;
  GeomAbs_Shape myContinuity = GeomAbs_C2;
  Standard_Integer myMaxDegree = 14, myMaxSeg = 16;


  ProjLib_CompProjectedCurve Projector(hsur, hcur, myTol3d/10, myTol3d/10);
  Handle(ProjLib_HCompProjectedCurve) HProjector = new ProjLib_HCompProjectedCurve();
  HProjector->Set(Projector);

  Standard_Integer k;
  Standard_Real Udeb, Ufin, UIso, VIso;
  Standard_Boolean Only2d, Only3d;
  gp_Pnt2d P2d, Pdeb, Pfin;
  gp_Pnt P;
  Handle(Adaptor2d_HCurve2d) HPCur;
  Handle(Geom2d_Curve) PCur2d; // Only for isoparametric projection

  for(k = 1; k <= Projector.NbCurves(); k++){
    Sprintf(newname,"%s_%d",name,k);
    Sprintf(newname1,"%s2d_%d",name,k);
    if(Projector.IsSinglePnt(k, P2d)){
//      cout<<"Part "<<k<<" of the projection is punctual"<<endl;
      Projector.GetSurface()->D0(P2d.X(), P2d.Y(), P);
      DrawTrSurf::Set(temp, P);
      DrawTrSurf::Set(temp1, P2d);
      di<<temp<<" is 3d projected curve\n";
      di<<temp1<<" is pcurve\n";
    }
    else {
      Only2d = Only3d = Standard_False;
      Projector.Bounds(k, Udeb, Ufin);
      gp_Dir2d Dir; // Only for isoparametric projection
      
      if (Projector.IsUIso(k, UIso)) {
//      cout<<"Part "<<k<<" of the projection is U-isoparametric curve"<<endl;
        Projector.D0(Udeb, Pdeb);
        Projector.D0(Ufin, Pfin);
        Udeb = Pdeb.Y();
        Ufin = Pfin.Y();
        if (Udeb > Ufin) {
          Dir = gp_Dir2d(0, -1);
          Udeb = - Udeb;
          Ufin = - Ufin;
        }
        else Dir = gp_Dir2d(0, 1);
        PCur2d = new Geom2d_TrimmedCurve(new Geom2d_Line(gp_Pnt2d(UIso, 0), Dir), Udeb, Ufin);
        HPCur = new Geom2dAdaptor_HCurve(PCur2d);
        Only3d = Standard_True;
      }
      else if(Projector.IsVIso(k, VIso)) {
//      cout<<"Part "<<k<<" of the projection is V-isoparametric curve"<<endl;
        Projector.D0(Udeb, Pdeb);
        Projector.D0(Ufin, Pfin);
        Udeb = Pdeb.X();
        Ufin = Pfin.X();
        if (Udeb > Ufin) {
          Dir = gp_Dir2d(-1, 0);
          Udeb = - Udeb;
          Ufin = - Ufin;
        }
        else Dir = gp_Dir2d(1, 0);
        PCur2d = new Geom2d_TrimmedCurve(new Geom2d_Line(gp_Pnt2d(0, VIso), Dir), Udeb, Ufin);
        HPCur = new Geom2dAdaptor_HCurve(PCur2d);
        Only3d = Standard_True;
      }
      else HPCur = HProjector;
      
      if(Projector.MaxDistance(k) <= myTol3d)
        Only2d = Standard_True;
      
      if(Only2d && Only3d) {
        Handle(Geom_Curve) OutCur = new Geom_TrimmedCurve(GeomAdaptor::MakeCurve(hcur->Curve()), Ufin, Udeb);
        DrawTrSurf::Set(temp, OutCur);
        DrawTrSurf::Set(temp1, PCur2d);
        di<<temp<<" is 3d projected curve\n";
        di<<temp1<<" is pcurve\n";
        return 0;
        }
      else {
        Approx_CurveOnSurface appr(HPCur, hsur, Udeb, Ufin, myTol3d, 
				   myContinuity, myMaxDegree, myMaxSeg, 
				   Only3d, Only2d);
        if(!Only3d) {
	  PCur2d = appr.Curve2d();
	  di << " Error in 2d is " << appr.MaxError2dU()
	       << ";  "  << appr.MaxError2dV() << "\n"; 
	}
        if(Only2d) {
          Handle(Geom_Curve) OutCur = 
	    new Geom_TrimmedCurve(GeomAdaptor::MakeCurve(hcur->Curve()), 
				  Ufin, Udeb);
          DrawTrSurf::Set(temp, OutCur);
          }
        else {
	  di << " Error in 3d is " <<  appr.MaxError3d() << "\n";
	  DrawTrSurf::Set(temp, appr.Curve3d());
	}
        DrawTrSurf::Set(temp1, PCur2d);
        di<<temp<<" is 3d projected curve\n";
        di<<temp1<<" is pcurve\n";
      }
    }
  }
return 0;
}
//=======================================================================
//function : project
//purpose  : 
//=======================================================================

static Standard_Integer project (Draw_Interpretor& di, 
				 Standard_Integer n, const char** a)
{
  if ( n == 1) {
    
    di << "project result2d c3d surf [-e p] [-v n] [-t tol]\n";
    di << "   -e p   : extent the surface of <p>%\n";
    di << "   -v n   : verify the projection at <n> points.\n";
    di << "   -t tol : set the tolerance for approximation\n";
    return 0;
  }

  if (n < 4) return 1;
  Handle(Geom_Surface) GS = DrawTrSurf::GetSurface(a[3]);
  if (GS.IsNull()) return 1;
    
  Handle(Geom_Curve) GC = DrawTrSurf::GetCurve(a[2]);
  if (GC.IsNull()) return 1;

  Standard_Real tolerance = Precision::Confusion() ;

  Standard_Real U1,U2,V1,V2;
  GS->Bounds(U1,U2,V1,V2);

  Standard_Boolean Verif = Standard_False;
  Standard_Integer NbPoints=0;

  Standard_Integer index = 4;
  while ( index+1 < n) {
    if ( a[index][0] != '-') return 1;

    if ( a[index][1] == 'e') {
      Standard_Real p = Draw::Atof(a[index+1]);
      Standard_Real dU = p * (U2 - U1) / 100.;
      Standard_Real dV = p * (V2 - V1) / 100.;
      U1 -= dU; U2 += dU; V1 -= dV; V2 += dV;
    }
    else if ( a[index][1] == 'v') {
      Verif = Standard_True;
      NbPoints = Draw::Atoi(a[index+1]);
    }
    else if ( a[index][1] == 't') {
      tolerance = Draw::Atof(a[index+1]);
    }
    index += 2;
  }
  
  Handle(Geom2d_Curve) G2d = 
    GeomProjLib::Curve2d(GC, GS, U1, U2, V1, V2, tolerance);

  if ( G2d.IsNull() ) {
    di << "\nProjection Failed\n";
    return 1;
  }
  else {
    DrawTrSurf::Set(a[1],G2d);
  }
  if ( Verif) {  // verify the projection on n points
    if ( NbPoints <= 0) { 
      di << " n must be positive\n";
      return 0;
    }
    gp_Pnt P1,P2;
    gp_Pnt2d P2d;
    
    Standard_Real U, dU;
    Standard_Real Dist,DistMax = -1.;
    U1 = GC->FirstParameter();
    U2 = GC->LastParameter();
    dU = ( U2 - U1) / (NbPoints + 1);
    for ( Standard_Integer i = 0 ; i <= NbPoints +1; i++) {
      U = U1 + i *dU;
      P1 = GC->Value(U);
      P2d = G2d->Value(U);
      P2 = GS->Value(P2d.X(), P2d.Y());
      Dist = P1.Distance(P2);
      di << " Parameter = " << U << "\tDistance = " << Dist << "\n";
      if ( Dist > DistMax) DistMax = Dist;
    }
    di << " **** Distance Maximale : " << DistMax << "\n";
  }

  return 0;
}

//=======================================================================
//function : projonplane
//purpose  : 
//=======================================================================

Standard_Integer projonplane(Draw_Interpretor& di, 
			     Standard_Integer n, const char** a)
{
  if ( n < 4 ) return 1;

  Handle(Geom_Surface) S = DrawTrSurf::GetSurface(a[3]);
  if ( S.IsNull()) return 1;

  Handle(Geom_Plane)   Pl = Handle(Geom_Plane)::DownCast(S);
  if ( Pl.IsNull()) {
    di << " The surface must be a plane\n";
    return 1;
  }

  Handle(Geom_Curve) C = DrawTrSurf::GetCurve(a[2]);
  if ( C.IsNull()) return 1;
  
  Standard_Boolean Param = Standard_True;
  if ((n == 5 && Draw::Atoi(a[4]) == 0) ||
      (n == 8 && Draw::Atoi(a[7]) == 0)) Param = Standard_False;

  gp_Dir D;
  
  if ( n == 8) {
    D = gp_Dir(Draw::Atof(a[4]),Draw::Atof(a[5]),Draw::Atof(a[6]));
  }
  else { 
    D = Pl->Pln().Position().Direction();
  }

  Handle(Geom_Curve) Res = 
    GeomProjLib::ProjectOnPlane(C,Pl,D,Param);

  DrawTrSurf::Set(a[1],Res);
  return 0;

}


//=======================================================================
//function : bisec
//purpose  : 
//=======================================================================

static void solution(const Handle(GccInt_Bisec)& Bis,
		     const char* name,
		     const Standard_Integer i)
{
  char solname[200];
  if ( i == 0) 
    Sprintf(solname,"%s",name);
  else
    Sprintf(solname,"%s_%d",name,i);
  const char* temp = solname; // pour portage WNT

  switch ( Bis->ArcType()) {
  case GccInt_Lin:
    DrawTrSurf::Set(temp, new Geom2d_Line(Bis->Line()));
    break;
  case GccInt_Cir:
    DrawTrSurf::Set(temp, new Geom2d_Circle(Bis->Circle()));
    break;
  case GccInt_Ell:
    DrawTrSurf::Set(temp, new Geom2d_Ellipse(Bis->Ellipse()));
    break;
  case GccInt_Par:
    DrawTrSurf::Set(temp, new Geom2d_Parabola(Bis->Parabola()));
    break;
  case GccInt_Hpr:
    DrawTrSurf::Set(temp, new Geom2d_Hyperbola(Bis->Hyperbola()));
    break;
  case GccInt_Pnt:
    DrawTrSurf::Set(temp, Bis->Point());
    break;
  }
}

static Standard_Integer bisec (Draw_Interpretor& di, 
			       Standard_Integer n, const char** a)
{
  if (n < 4) return 1;
  
  Handle(Geom2d_Curve) C1 = DrawTrSurf::GetCurve2d(a[2]);
  Handle(Geom2d_Curve) C2 = DrawTrSurf::GetCurve2d(a[3]);
  gp_Pnt2d P1,P2;
  Standard_Boolean ip1 = DrawTrSurf::GetPoint2d(a[2],P1);
  Standard_Boolean ip2 = DrawTrSurf::GetPoint2d(a[3],P2);
  Standard_Integer i, Compt = 0;
  Standard_Integer NbSol = 0;

  if ( !C1.IsNull()) {
    Handle(Standard_Type) Type1 = C1->DynamicType();
    if ( !C2.IsNull()) {
      Handle(Standard_Type) Type2 = C2->DynamicType();
      if ( Type1 == STANDARD_TYPE(Geom2d_Line) && 
	   Type2 == STANDARD_TYPE(Geom2d_Line)   ) {
	GccAna_Lin2dBisec Bis(Handle(Geom2d_Line)::DownCast(C1)->Lin2d(),
			      Handle(Geom2d_Line)::DownCast(C2)->Lin2d());
	if ( Bis.IsDone()) {
	  char solname[200];
	  NbSol = Bis.NbSolutions();
	  for ( i = 1; i <= NbSol; i++) {
	    Sprintf(solname,"%s_%d",a[1],i);
	    const char* temp = solname; // pour portage WNT
	    DrawTrSurf::Set(temp,new Geom2d_Line(Bis.ThisSolution(i)));
	  }
	}
	else {
	  di << " Bisec has failed !!\n";
	  return 1;
	}
      }
      else if ( Type1 == STANDARD_TYPE(Geom2d_Line) && 
	        Type2 == STANDARD_TYPE(Geom2d_Circle) ) {
	GccAna_CircLin2dBisec 
	  Bis(Handle(Geom2d_Circle)::DownCast(C2)->Circ2d(),
	      Handle(Geom2d_Line)::DownCast(C1)->Lin2d());
	if ( Bis.IsDone()) {
	  NbSol= Bis.NbSolutions();
	  if ( NbSol >= 2) Compt = 1;
	  for ( i = 1; i <= NbSol; i++) {
	    solution(Bis.ThisSolution(i),a[1],Compt);
	    Compt++;
	  }
	}
	else {
	  di << " Bisec has failed !!\n";
	  return 1;
	}
      }
      else if ( Type2 == STANDARD_TYPE(Geom2d_Line) && 
	        Type1 == STANDARD_TYPE(Geom2d_Circle) ) {
	GccAna_CircLin2dBisec 
	  Bis(Handle(Geom2d_Circle)::DownCast(C1)->Circ2d(), 
	      Handle(Geom2d_Line)::DownCast(C2)->Lin2d());
	if ( Bis.IsDone()) {
//	  char solname[200];
	  NbSol = Bis.NbSolutions();
	  if ( NbSol >= 2) Compt = 1;
	  for ( i = 1; i <= NbSol; i++) {
	    solution(Bis.ThisSolution(i),a[1],Compt);
	    Compt++;
	  }
	}
	else {
	  di << " Bisec has failed !!\n";
	  return 1;
	}
      }
      else if ( Type2 == STANDARD_TYPE(Geom2d_Circle) && 
	        Type1 == STANDARD_TYPE(Geom2d_Circle) ) {
	GccAna_Circ2dBisec 
	  Bis(Handle(Geom2d_Circle)::DownCast(C1)->Circ2d(), 
	      Handle(Geom2d_Circle)::DownCast(C2)->Circ2d());
	if ( Bis.IsDone()) {
//	  char solname[200];
	  NbSol = Bis.NbSolutions();
	  if ( NbSol >= 2) Compt = 1;
	  for ( i = 1; i <= NbSol; i++) {
	    solution(Bis.ThisSolution(i),a[1],Compt);
	    Compt++;
	  }
	}
	else {
	  di << " Bisec has failed !!\n";
	  return 1;
	}
      }
      else {
	di << " args must be line/circle/point line/circle/point\n";
	return 1;
      }
    }
    else if (ip2) {
      if ( Type1 == STANDARD_TYPE(Geom2d_Circle)) {
	GccAna_CircPnt2dBisec Bis
	  (Handle(Geom2d_Circle)::DownCast(C1)->Circ2d(),P2);
	if ( Bis.IsDone()) {
	  NbSol = Bis.NbSolutions();
	  if ( NbSol >= 2) Compt = 1;
	  for ( i = 1; i <= NbSol; i++) {
	    solution(Bis.ThisSolution(i),a[1],Compt);
	    Compt++;
	  }
	}
	else {
	  di << " Bisec has failed !!\n";
	  return 1;
	}
      }
      else if ( Type1 == STANDARD_TYPE(Geom2d_Line)) {
	GccAna_LinPnt2dBisec Bis
	  (Handle(Geom2d_Line)::DownCast(C1)->Lin2d(),P2);
	if ( Bis.IsDone()) {
	  NbSol = 1;
	  solution(Bis.ThisSolution(),a[1],0);
	}
	else {
	  di << " Bisec has failed !!\n";
	  return 1;
	}
      }
    }
    else {
      di << " the second arg must be line/circle/point \n";
    }
  }
  else if ( ip1) {
    if ( !C2.IsNull()) {
      Handle(Standard_Type) Type2 = C2->DynamicType();
      if ( Type2 == STANDARD_TYPE(Geom2d_Circle)) {
	GccAna_CircPnt2dBisec Bis
	  (Handle(Geom2d_Circle)::DownCast(C2)->Circ2d(),P1);
	if ( Bis.IsDone()) {
	  NbSol = Bis.NbSolutions();
	  if ( NbSol >= 2) Compt = 1;
	  for ( i = 1; i <= Bis.NbSolutions(); i++) {
	    solution(Bis.ThisSolution(i),a[1],Compt);
	    Compt++;
	  }
	}
	else {
	  di << " Bisec has failed !!\n";
	  return 1;
	}
      }
      else if ( Type2 == STANDARD_TYPE(Geom2d_Line)) {
	GccAna_LinPnt2dBisec Bis
	  (Handle(Geom2d_Line)::DownCast(C2)->Lin2d(),P1);
	if ( Bis.IsDone()) {
	  NbSol = 1;
	  solution(Bis.ThisSolution(),a[1],0);
	}
	else {
	  di << " Bisec has failed !!\n";
	  return 1;
	}
      }
    }
    else if (ip2) {
      GccAna_Pnt2dBisec Bis(P1,P2);
      if ( Bis.HasSolution()) {
	NbSol = 1;
	DrawTrSurf::Set(a[1],new Geom2d_Line(Bis.ThisSolution()));
      }
      else {
	di << " Bisec has failed !!\n";
	return 1;
      }
    }
    else {
      di << " the second arg must be line/circle/point \n";
      return 1;
    }
  }
  else {
    di << " args must be line/circle/point line/circle/point\n";
    return 1;
  }

  if ( NbSol >= 2) {
    di << "There are " << NbSol << " Solutions.\n";
  }
  else {
    di << "There is " << NbSol << " Solution.\n";
  }

  return 0;
}

//=======================================================================
//function : cmovetangent
//purpose  : 
//=======================================================================

static Standard_Integer movelaw (Draw_Interpretor& di, Standard_Integer n, const char** a)
{
  Standard_Integer 
    ii,
    condition=0,
    error_status ;
  Standard_Real u,
    x,
    tolerance,
    tx ;

  u = Draw::Atof(a[2]);
  x = Draw::Atof(a[3]);
  tolerance = 1.0e-5 ;
  if (n < 5) {
    return 1 ;
  }
  Handle(Geom2d_BSplineCurve) G2 = DrawTrSurf::GetBSplineCurve2d(a[1]);
  if (!G2.IsNull()) {
    tx = Draw::Atof(a[4]) ;
    if (n == 6) {
      condition = Max(Draw::Atoi(a[5]), -1)  ;
      condition = Min(condition, G2->Degree()-1) ;
    }
    TColgp_Array1OfPnt2d   curve_poles(1,G2->NbPoles()) ;
    TColStd_Array1OfReal    law_poles(1,G2->NbPoles()) ;
    TColStd_Array1OfReal    law_knots(1,G2->NbKnots()) ;
    TColStd_Array1OfInteger law_mults(1,G2->NbKnots()) ;

    G2->Knots(law_knots) ;
    G2->Multiplicities(law_mults) ;
    G2->Poles(curve_poles) ;
    for (ii = 1 ; ii <= G2->NbPoles() ; ii++) {
      law_poles(ii) = curve_poles(ii).Coord(2) ;
    }

    Law_BSpline  a_law(law_poles,
      law_knots,
      law_mults,
      G2->Degree(),
      Standard_False) ;

    a_law.MovePointAndTangent(u, 
      x, 
      tx,
      tolerance,
      condition,
      condition,
      error_status) ;

    for (ii = 1 ; ii <= G2->NbPoles() ; ii++) {
      curve_poles(ii).SetCoord(2,a_law.Pole(ii)) ;
      G2->SetPole(ii,curve_poles(ii)) ;
    }


    if (! error_status) {
      Draw::Repaint();
    }
    else {
      di << "Not enought degree of freedom increase degree please\n";
    }


  }
  return 0;
} 


//Static method computing deviation of curve and polyline
#include <math_PSO.hxx>
#include <math_PSOParticlesPool.hxx>
#include <math_MultipleVarFunction.hxx>
#include <math_BrentMinimum.hxx>

static Standard_Real CompLocalDev(const Handle(Geom_Curve)& theCurve,  
                                  const Standard_Real u1, const Standard_Real u2);

static void ComputeDeviation(const Handle(Geom_Curve)& theCurve,
                             const Handle(Geom_BSplineCurve)& thePnts,
                             Standard_Real& theDmax,
                             Standard_Real& theUfMax,
                             Standard_Real& theUlMax,
                             Standard_Integer& theImax)
{
  theDmax = 0.;
  theUfMax = 0.;
  theUlMax = 0.;
  theImax = 0;

  //take knots
  Standard_Integer nbp = thePnts->NbKnots();
  TColStd_Array1OfReal aKnots(1, nbp);
  thePnts->Knots(aKnots);

  Standard_Integer i;
  for(i = 1; i < nbp; ++i)
  {
    Standard_Real u1 = aKnots(i), u2 = aKnots(i+1);
    Standard_Real d = CompLocalDev(theCurve, u1, u2);
    if(d > theDmax)
    {
      theDmax = d;
      theImax = i;
      theUfMax = u1;
      theUlMax = u2;
    }
  }
}

Standard_Real CompLocalDev(const Handle(Geom_Curve)& theCurve,  
                           const Standard_Real u1, const Standard_Real u2)
{
  math_Vector aLowBorder(1,1);
  math_Vector aUppBorder(1,1);
  math_Vector aSteps(1,1);
  GeomAdaptor_Curve TCurve(theCurve);
  //
  aLowBorder(1) = u1;
  aUppBorder(1) = u2;
  aSteps(1) =(aUppBorder(1) - aLowBorder(1)) * 0.01; // Run PSO on even distribution with 100 points.
  //
  GCPnts_DistFunction aFunc1(TCurve,  u1, u2);
  //
  Standard_Real aValue;
  math_Vector aT(1,1);
  GCPnts_DistFunctionMV aFunc(aFunc1);

  math_PSO aFinder(&aFunc, aLowBorder, aUppBorder, aSteps); // Choose 32 best points from 100 above.
  aFinder.Perform(aSteps, aValue, aT);
  Standard_Real d = 0.;

  Standard_Real d1, d2;
  Standard_Real x1 = Max(u1, aT(1) - aSteps(1));
  Standard_Boolean Ok = aFunc1.Value(x1, d1);
  if(!Ok)
  {
    return Sqrt(-aValue);
  }
  Standard_Real x2 = Min(u2, aT(1) + aSteps(1));
  Ok = aFunc1.Value(x2, d2);
  if(!Ok)
  {
    return Sqrt(-aValue);
  }
  if(!(d1 > aValue && d2 > aValue))
  {
    Standard_Real dmin = Min(d1, Min(aValue, d2));
    return Sqrt(-dmin);
  }

  math_BrentMinimum anOptLoc(Precision::PConfusion());
  anOptLoc.Perform(aFunc1, x1, aT(1), x2);

  if (anOptLoc.IsDone())
  {
    d = -anOptLoc.Minimum();
  }
  else
  {
    d = -aValue;
  }
  return Sqrt(d);
}

//=======================================================================
//function : crvpoints
//purpose  : 
//=======================================================================

static Standard_Integer crvpoints (Draw_Interpretor& di, Standard_Integer /*n*/, const char** a)
{
  Standard_Integer i, nbp;
  Standard_Real defl;

  Handle(Geom_Curve) C = DrawTrSurf::GetCurve(a[2]);
  defl = Draw::Atof(a[3]);

  GeomAdaptor_Curve GAC(C);
  GCPnts_QuasiUniformDeflection PntGen(GAC, defl);
    
  if(!PntGen.IsDone()) {
    di << "Points generation failed\n";
    return 1;
  }

  nbp = PntGen.NbPoints();
  di << "Nb points : " << nbp << "\n";

  TColgp_Array1OfPnt aPoles(1, nbp);
  TColStd_Array1OfReal aKnots(1, nbp);
  TColStd_Array1OfInteger aMults(1, nbp);

  for(i = 1; i <= nbp; ++i) {
    aPoles(i) = PntGen.Value(i);
    aKnots(i) = PntGen.Parameter(i);
    aMults(i) = 1;
  }
  
  aMults(1) = 2;
  aMults(nbp) = 2;

  Handle(Geom_BSplineCurve) aPnts = new Geom_BSplineCurve(aPoles, aKnots, aMults, 1);
  Handle(DrawTrSurf_BSplineCurve) aDrCrv = new DrawTrSurf_BSplineCurve(aPnts);

  aDrCrv->ClearPoles();
  Draw_Color aKnColor(Draw_or);
  aDrCrv->SetKnotsColor(aKnColor);
  aDrCrv->SetKnotsShape(Draw_Plus);

  Draw::Set(a[1], aDrCrv);

  Standard_Real dmax = 0., ufmax = 0., ulmax = 0.;
  Standard_Integer imax = 0;

  //check deviation
  ComputeDeviation(C,aPnts,dmax,ufmax,ulmax,imax);
  di << "Max defl: " << dmax << " " << ufmax << " " << ulmax << " " << imax << "\n"; 

  return 0;
} 

//=======================================================================
//function : crvtpoints
//purpose  : 
//=======================================================================

static Standard_Integer crvtpoints (Draw_Interpretor& di, Standard_Integer n, const char** a)
{
  Standard_Integer i, nbp;
  Standard_Real defl, angle = Precision::Angular();

  Handle(Geom_Curve) C = DrawTrSurf::GetCurve(a[2]);
  defl = Draw::Atof(a[3]);

  if(n > 3)
    angle = Draw::Atof(a[4]);

  GeomAdaptor_Curve GAC(C);
  GCPnts_TangentialDeflection PntGen(GAC, angle, defl, 2);
  
  nbp = PntGen.NbPoints();
  di << "Nb points : " << nbp << "\n";

  TColgp_Array1OfPnt aPoles(1, nbp);
  TColStd_Array1OfReal aKnots(1, nbp);
  TColStd_Array1OfInteger aMults(1, nbp);

  for(i = 1; i <= nbp; ++i) {
    aPoles(i) = PntGen.Value(i);
    aKnots(i) = PntGen.Parameter(i);
    aMults(i) = 1;
  }
  
  aMults(1) = 2;
  aMults(nbp) = 2;

  Handle(Geom_BSplineCurve) aPnts = new Geom_BSplineCurve(aPoles, aKnots, aMults, 1);
  Handle(DrawTrSurf_BSplineCurve) aDrCrv = new DrawTrSurf_BSplineCurve(aPnts);

  aDrCrv->ClearPoles();
  Draw_Color aKnColor(Draw_or);
  aDrCrv->SetKnotsColor(aKnColor);
  aDrCrv->SetKnotsShape(Draw_Plus);

  Draw::Set(a[1], aDrCrv);

  Standard_Real dmax = 0., ufmax = 0., ulmax = 0.;
  Standard_Integer imax = 0;

  //check deviation
  ComputeDeviation(C,aPnts,dmax,ufmax,ulmax,imax);
  //
  di << "Max defl: " << dmax << " " << ufmax << " " << ulmax << " " << imax << "\n"; 

  return 0;
} 
//=======================================================================
//function : uniformAbscissa
//purpose  : epa test (TATA-06-002 (Problem with GCPnts_UniformAbscissa class)
//=======================================================================
static Standard_Integer uniformAbscissa (Draw_Interpretor& di, Standard_Integer n, const char** a)
{
  if( n != 3 )
    return 1;
  
  /*Handle(Geom_BSplineCurve) ellip;
  ellip = DrawTrSurf::GetBSplineCurve(a[1]);
  if (ellip.IsNull())
  {
    di << " BSpline is NULL  \n";     
    return 1;
  }*/

  Handle(Geom_Curve) ellip;
  ellip = DrawTrSurf::GetCurve(a[1]);
  if (ellip.IsNull())
  {
    di << " Curve is NULL  \n";     
    return 1;
  }

  Standard_Integer nocp;
  nocp = Draw::Atoi(a[2]);
  if(nocp < 2)
    return 1;


  //test nbPoints for Geom_Ellipse

  try
  {
    GeomLProp_CLProps Prop(ellip,2,Precision::Intersection());
    Prop.SetCurve(ellip);

    GeomAdaptor_Curve GAC(ellip);
    di<<"Type Of curve: "<<GAC.GetType()<<"\n";
    Standard_Real Tol = Precision::Confusion();
    Standard_Real L;

    L = GCPnts_AbscissaPoint::Length(GAC, GAC.FirstParameter(), GAC.LastParameter(), Tol);
    di<<"Ellipse length = "<<L<<"\n";
    Standard_Real Abscissa = L/(nocp-1);
    di << " CUR : Abscissa " << Abscissa << "\n";

    GCPnts_UniformAbscissa myAlgo(GAC, Abscissa, ellip->FirstParameter(), ellip->LastParameter());
    if ( myAlgo.IsDone() )
    {
      di << " CasCurve  - nbpoints " << myAlgo.NbPoints() << "\n";
      for(Standard_Integer i = 1; i<= myAlgo.NbPoints(); i++ )
        di << i <<" points = " << myAlgo.Parameter( i ) << "\n";
    }
  }

  catch (Standard_Failure )
  {
    di << " Standard Failure  \n";
  }
  return 0;
}

//=======================================================================
//function : EllipsUniformAbscissa
//purpose  : epa test (TATA-06-002 (Problem with GCPnts_UniformAbscissa class)
//=======================================================================
static Standard_Integer EllipsUniformAbscissa (Draw_Interpretor& di, Standard_Integer n, const char** a)
{
  if( n != 4 )
    return 1;  
  
  Standard_Real R1;
  R1 = Draw::Atof(a[1]);
  Standard_Real R2;
  R2 = Draw::Atof(a[2]);

  Standard_Integer nocp;
  nocp = Draw::Atoi(a[3]);
  if(nocp < 2)
    return 1;
  
  //test nbPoints for Geom_Ellipse
  Handle(Geom_Ellipse) ellip;


  try
  {
    gp_Pnt location;
    location = gp_Pnt( 0.0, 0.0, 0.0);
    gp_Dir main_direction(0.0, 0.0, 1.0);

    gp_Dir x_direction(1.0, 0.0, 0.0);
    gp_Ax2 mainaxis( location, main_direction);

    mainaxis.SetXDirection(x_direction);
    ellip = new Geom_Ellipse(mainaxis,R1, R2);

    BRepBuilderAPI_MakeEdge curve_edge(ellip);
    TopoDS_Edge edge_curve = curve_edge.Edge();

    DBRep::Set("Ellipse",edge_curve);
  }
  
  catch(Standard_Failure)
  {
    di << " Standard Failure  \n";     
  }

  try
  {
    GeomLProp_CLProps Prop(ellip,2,Precision::Intersection());
    Prop.SetCurve(ellip);

    GeomAdaptor_Curve GAC(ellip);
    di<<"Type Of curve: "<<GAC.GetType()<<"\n";
    Standard_Real Tol = Precision::Confusion();
    Standard_Real L;

    L = GCPnts_AbscissaPoint::Length(GAC, GAC.FirstParameter(), GAC.LastParameter(), Tol);
    di<<"Ellipse length = "<<L<<"\n";
    Standard_Real Abscissa = L/(nocp-1);
    di << " CUR : Abscissa " << Abscissa << "\n";

    GCPnts_UniformAbscissa myAlgo(GAC, Abscissa, ellip->FirstParameter(), ellip->LastParameter());
    if ( myAlgo.IsDone() )
    {
      di << " CasCurve  - nbpoints " << myAlgo.NbPoints() << "\n";
      for(Standard_Integer i = 1; i<= myAlgo.NbPoints(); i++ )
        di << i <<" points = " << myAlgo.Parameter( i ) << "\n";
    }
  }

  catch (Standard_Failure )
  {
    di << " Standard Failure  \n";
  }
  return 0;
}

//=======================================================================
//function : discrCurve
//purpose  :
//=======================================================================
static Standard_Integer discrCurve(Draw_Interpretor& di, Standard_Integer theArgNb, const char** theArgVec)
{
  if (theArgNb < 3)
  {
    di << "Invalid number of parameters.\n";
    return 1;
  }

  Handle(Geom_Curve) aCurve = DrawTrSurf::GetCurve(theArgVec[2]);
  if (aCurve.IsNull())
  {
    di << "Curve is NULL.\n";
    return 1;
  }

  Standard_Integer aSrcNbPnts = 0;
  Standard_Boolean isUniform = Standard_False;
  for (Standard_Integer anArgIter = 3; anArgIter < theArgNb; ++anArgIter)
  {
    TCollection_AsciiString anArg     (theArgVec[anArgIter]);
    TCollection_AsciiString anArgCase (anArg);
    anArgCase.LowerCase();
    if (anArgCase == "nbpnts")
    {
      if (++anArgIter >= theArgNb)
      {
        di << "Value for argument '" << anArg << "' is absent.\n";
        return 1;
      }

      aSrcNbPnts = Draw::Atoi (theArgVec[anArgIter]);
    }
    else if (anArgCase == "uniform")
    {
      if (++anArgIter >= theArgNb)
      {
        di << "Value for argument '" << anArg << "' is absent.\n";
        return 1;
      }

      isUniform = (Draw::Atoi (theArgVec[anArgIter]) == 1);
    }
    else
    {
      di << "Invalid argument '" << anArg << "'.\n";
      return 1;
    }
  }

  if (aSrcNbPnts < 2)
  {
    di << "Invalid count of points.\n";
    return 1;
  }

  if (!isUniform)
  {
    di << "Invalid type of discretization.\n";
    return 1;
  }

  GeomAdaptor_Curve aCurveAdaptor(aCurve);
  GCPnts_UniformAbscissa aSplitter(aCurveAdaptor, aSrcNbPnts, Precision::Confusion());
  if (!aSplitter.IsDone())
  {
    di << "Error: Invalid result.\n";
    return 0;
  }

  const Standard_Integer aDstNbPnts = aSplitter.NbPoints();

  if (aDstNbPnts < 2)
  {
    di << "Error: Invalid result.\n";
    return 0;
  }

  TColgp_Array1OfPnt aPoles(1, aDstNbPnts);
  TColStd_Array1OfReal aKnots(1, aDstNbPnts);
  TColStd_Array1OfInteger aMultiplicities(1, aDstNbPnts);

  for (Standard_Integer aPntIter = 1; aPntIter <= aDstNbPnts; ++aPntIter)
  {
    aPoles.ChangeValue(aPntIter) = aCurveAdaptor.Value(aSplitter.Parameter(aPntIter));
    aKnots.ChangeValue(aPntIter) = (aPntIter - 1) / (aDstNbPnts - 1.0);
    aMultiplicities.ChangeValue(aPntIter) = 1;
  }
  aMultiplicities.ChangeValue(1) = 2;
  aMultiplicities.ChangeValue(aDstNbPnts) = 2;

  Handle(Geom_BSplineCurve) aPolyline =
    new Geom_BSplineCurve(aPoles, aKnots, aMultiplicities, 1);
  DrawTrSurf::Set(theArgVec[1], aPolyline);

  return 0;
}

//=======================================================================
//function : mypoints
//purpose  : 
//=======================================================================

static Standard_Integer mypoints (Draw_Interpretor& di, Standard_Integer /*n*/, const char** a)
{
  Standard_Integer i, nbp;
  Standard_Real defl;

  Handle(Geom_Curve) C = DrawTrSurf::GetCurve(a[2]);
  defl = Draw::Atof(a[3]);
  Handle(Geom_BSplineCurve) aBS (Handle(Geom_BSplineCurve)::DownCast(C));

  if(aBS.IsNull()) return 1;

  Standard_Integer ui1 = aBS->FirstUKnotIndex();
  Standard_Integer ui2 = aBS->LastUKnotIndex();

  Standard_Integer nbsu = ui2-ui1+1; nbsu += (nbsu - 1) * (aBS->Degree()-1);

  TColStd_Array1OfReal anUPars(1, nbsu);
  TColStd_Array1OfBoolean anUFlg(1, nbsu);

  Standard_Integer j, k, nbi;
  Standard_Real t1, t2, dt;

  //Filling of sample parameters
  nbi = aBS->Degree();
  k = 0;
  t1 = aBS->Knot(ui1);
  for(i = ui1+1; i <= ui2; ++i) {
    t2 = aBS->Knot(i);
    dt = (t2 - t1)/nbi;
    j = 1;
    do { 
      ++k;
      anUPars(k) = t1;
      anUFlg(k) = Standard_False;
      t1 += dt;	
    }
    while (++j <= nbi);
    t1 = t2;
  }
  ++k;
  anUPars(k) = t1;

  Standard_Integer l;
  defl *= defl;

  j = 1;
  anUFlg(1) = Standard_True;
  anUFlg(nbsu) = Standard_True;
  Standard_Boolean bCont = Standard_True;
  while (j < nbsu-1 && bCont) {
    t2 = anUPars(j);
    gp_Pnt p1 = aBS->Value(t2);
    for(k = j+2; k <= nbsu; ++k) {
      t2 = anUPars(k);
      gp_Pnt p2 = aBS->Value(t2);
      gce_MakeLin MkLin(p1, p2);
      const gp_Lin& lin = MkLin.Value();
      Standard_Boolean ok = Standard_True;
      for(l = j+1; l < k; ++l) {
	if(anUFlg(l)) continue;
	gp_Pnt pp =  aBS->Value(anUPars(l));
	Standard_Real d = lin.SquareDistance(pp);
	  
	if(d <= defl) continue;

	ok = Standard_False;
	break;
      }


      if(!ok) {
	j = k - 1;
	anUFlg(j) = Standard_True;
	break;
      }

    }

    if(k >= nbsu) bCont = Standard_False;
  }

  nbp = 0;
  for(i = 1; i <= nbsu; ++i) {
    if(anUFlg(i)) nbp++;
  }

  TColgp_Array1OfPnt aPoles(1, nbp);
  TColStd_Array1OfReal aKnots(1, nbp);
  TColStd_Array1OfInteger aMults(1, nbp);
  j = 0;
  for(i = 1; i <= nbsu; ++i) {
    if(anUFlg(i)) {
      ++j;
      aKnots(j) = anUPars(i);
      aMults(j) = 1;
      aPoles(j) = aBS->Value(aKnots(j));
    }
  }
  
  aMults(1) = 2;
  aMults(nbp) = 2;

  Handle(Geom_BSplineCurve) aPnts = new Geom_BSplineCurve(aPoles, aKnots, aMults, 1);
  Handle(DrawTrSurf_BSplineCurve) aDrCrv = new DrawTrSurf_BSplineCurve(aPnts);

  aDrCrv->ClearPoles();
  Draw_Color aKnColor(Draw_or);
  aDrCrv->SetKnotsColor(aKnColor);
  aDrCrv->SetKnotsShape(Draw_Plus);

  Draw::Set(a[1], aDrCrv);

  Standard_Real dmax = 0., ufmax = 0., ulmax = 0.;
  Standard_Integer imax = 0;

  ComputeDeviation(C,aPnts,dmax,ufmax,ulmax,imax);
  di << "Max defl: " << dmax << " " << ufmax << " " << ulmax << " " << imax << "\n"; 

  return 0;
} 



//=======================================================================
//function : surfpoints
//purpose  : 
//=======================================================================

static Standard_Integer surfpoints (Draw_Interpretor& /*di*/, Standard_Integer /*n*/, const char** a)
{
  Standard_Integer i;
  Standard_Real defl;

  Handle(Geom_Surface) S = DrawTrSurf::GetSurface(a[2]);
  defl = Draw::Atof(a[3]);

  Handle(GeomAdaptor_HSurface) AS = new GeomAdaptor_HSurface(S);

  Handle(Adaptor3d_TopolTool) aTopTool = new Adaptor3d_TopolTool(AS);

  aTopTool->SamplePnts(defl, 10, 10);

  Standard_Integer nbpu = aTopTool->NbSamplesU();
  Standard_Integer nbpv = aTopTool->NbSamplesV();
  TColStd_Array1OfReal Upars(1, nbpu), Vpars(1, nbpv);
  aTopTool->UParameters(Upars);
  aTopTool->VParameters(Vpars);

  TColgp_Array2OfPnt aPoles(1, nbpu, 1, nbpv);
  TColStd_Array1OfReal anUKnots(1, nbpu);
  TColStd_Array1OfReal aVKnots(1, nbpv);
  TColStd_Array1OfInteger anUMults(1, nbpu);
  TColStd_Array1OfInteger aVMults(1, nbpv);

  Standard_Integer j;
  for(i = 1; i <= nbpu; ++i) {
    anUKnots(i) = Upars(i);
    anUMults(i) = 1;
    for(j = 1; j <= nbpv; ++j) {
      aVKnots(j) = Vpars(j);
      aVMults(j) = 1;
      aPoles(i,j) = S->Value(anUKnots(i),aVKnots(j));
    }
  }
  
  anUMults(1) = 2;
  anUMults(nbpu) = 2;
  aVMults(1) = 2;
  aVMults(nbpv) = 2;

  Handle(Geom_BSplineSurface) aPnts = new Geom_BSplineSurface(aPoles, anUKnots,  aVKnots, 
							      anUMults, aVMults, 1, 1);
  Handle(DrawTrSurf_BSplineSurface) aDrSurf = new DrawTrSurf_BSplineSurface(aPnts);

  aDrSurf->ClearPoles();
  Draw_Color aKnColor(Draw_or);
  aDrSurf->SetKnotsColor(aKnColor);
  aDrSurf->SetKnotsShape(Draw_Plus);

  Draw::Set(a[1], aDrSurf);


  return 0;
} 



//=======================================================================
//function : intersect
//purpose  : 
//=======================================================================
static Standard_Integer intersection (Draw_Interpretor& di, 
                                      Standard_Integer n, const char** a)
{
  if (n < 4)
    return 1;

  //
  Handle(Geom_Curve) GC1;
  Handle(Geom_Surface) GS1 = DrawTrSurf::GetSurface(a[2]);
  if (GS1.IsNull())
  {
    GC1 = DrawTrSurf::GetCurve(a[2]);
    if (GC1.IsNull())
      return 1;
  }

  //
  Handle(Geom_Surface) GS2 = DrawTrSurf::GetSurface(a[3]);
  if (GS2.IsNull())
    return 1;

  //
  Standard_Real tol = Precision::Confusion();
  if (n == 5 || n == 9 || n == 13 || n == 17)
    tol = Draw::Atof(a[n-1]);

  //
  Handle(Geom_Curve) Result;
  gp_Pnt             Point;

  //
  if (GC1.IsNull())
  {
    GeomInt_IntSS Inters;
    //
    // Surface Surface
    if (n <= 5)
    {
      // General case
      Inters.Perform(GS1,GS2,tol,Standard_True);
    }
    else if (n == 8 || n == 9 || n == 12 || n == 13 || n == 16 || n == 17)
    {
      Standard_Boolean useStart = Standard_True, useBnd = Standard_True;
      Standard_Integer ista1=0,ista2=0,ibnd1=0,ibnd2=0;
      Standard_Real UVsta[4];
      Handle(GeomAdaptor_HSurface) AS1,AS2;

      //
      if (n <= 9)          // user starting point
      {
        useBnd = Standard_False;
        ista1 = 4;
        ista2 = 7;
      }
      else if (n <= 13)   // user bounding
      {
        useStart = Standard_False;
        ibnd1 = 4; ibnd2 = 11;
      }
      else        // both user starting point and bounding
      {
        ista1 = 4; ista2 = 7;
        ibnd1 = 8; ibnd2 = 15;
      }

      if (useStart)
      {
        for (Standard_Integer i=ista1; i <= ista2; i++)
        {
          UVsta[i-ista1] = Draw::Atof(a[i]);
        }
      }

      if (useBnd)
      {
        Standard_Real UVbnd[8];
        for (Standard_Integer i=ibnd1; i <= ibnd2; i++)
          UVbnd[i-ibnd1] = Draw::Atof(a[i]);

        AS1 = new GeomAdaptor_HSurface(GS1,UVbnd[0],UVbnd[1],UVbnd[2],UVbnd[3]);
        AS2 = new GeomAdaptor_HSurface(GS2,UVbnd[4],UVbnd[5],UVbnd[6],UVbnd[7]);
      }

      //
      if (useStart && !useBnd)
      {
        Inters.Perform(GS1,GS2,tol,UVsta[0],UVsta[1],UVsta[2],UVsta[3]);
      }
      else if (!useStart && useBnd)
      {
        Inters.Perform(AS1,AS2,tol);
      }
      else
      {
        Inters.Perform(AS1,AS2,tol,UVsta[0],UVsta[1],UVsta[2],UVsta[3]);
      }
    }//else if (n == 8 || n == 9 || n == 12 || n == 13 || n == 16 || n == 17)
    else
    {
      di<<"incorrect number of arguments\n";
      return 1;
    }

    //
    if (!Inters.IsDone())
    {
      di<<"No intersections found!\n";

      return 1;
    }

    //
    char buf[1024];
    Standard_Integer i, aNbLines, aNbPoints; 

    //
    aNbLines = Inters.NbLines();
    if (aNbLines >= 2)
    {
      for (i=1; i<=aNbLines; ++i)
      {
        Sprintf(buf, "%s_%d",a[1],i);
        di << buf << " ";
        Result = Inters.Line(i);
        const char* temp = buf;
        DrawTrSurf::Set(temp,Result);
      }
    }
    else if (aNbLines == 1)
    {
      Result = Inters.Line(1);
      Sprintf(buf,"%s",a[1]);
      di << buf << " ";
      DrawTrSurf::Set(a[1],Result);
    }

    //
    aNbPoints=Inters.NbPoints();
    for (i=1; i<=aNbPoints; ++i)
    {
      Point=Inters.Point(i);
      Sprintf(buf,"%s_p_%d",a[1],i);
      di << buf << " ";
      const char* temp = buf;
      DrawTrSurf::Set(temp, Point);
    }
  }// if (GC1.IsNull())
  else
  {
    // Curve Surface
    GeomAPI_IntCS Inters(GC1,GS2);

    //
    if (!Inters.IsDone())
    {
      di<<"No intersections found!\n";
      return 1;
    }

    Standard_Integer nblines = Inters.NbSegments();
    Standard_Integer nbpoints = Inters.NbPoints();

    char newname[1024];

    if ( (nblines+nbpoints) >= 2)
    {
      Standard_Integer i;
      Standard_Integer Compt = 1;

      if(nblines >= 1)
        cout << "   Lines: " << endl;

      for (i = 1; i <= nblines; i++, Compt++)
      {
        Sprintf(newname,"%s_%d",a[1],Compt);
        di << newname << " ";
        Result = Inters.Segment(i);
        const char* temp = newname; // pour portage WNT
        DrawTrSurf::Set(temp,Result);
      }

      if(nbpoints >= 1)
        cout << "   Points: " << endl;

      const Standard_Integer imax = nblines+nbpoints;

      for (/*i = 1*/; i <= imax; i++, Compt++)
      {
        Sprintf(newname,"%s_%d",a[1],i);
        di << newname << " ";
        Point = Inters.Point(i);
        const char* temp = newname; // pour portage WNT
        DrawTrSurf::Set(temp,Point);
      }
    }
    else if (nblines == 1)
    {
      Result = Inters.Segment(1);
      Sprintf(newname,"%s",a[1]);
      di << newname << " ";
      DrawTrSurf::Set(a[1],Result);
    }
    else if (nbpoints == 1)
    {
      Point = Inters.Point(1);
      Sprintf(newname,"%s",a[1]);
      di << newname << " ";
      DrawTrSurf::Set(a[1],Point);
    }
  }

  dout.Flush();
  return 0;
}

//=======================================================================
//function : GetCurveContinuity
//purpose  : Returns the continuity of the given curve
//=======================================================================
static Standard_Integer GetCurveContinuity( Draw_Interpretor& theDI,
                                            Standard_Integer theNArg,
                                            const char** theArgv)
{
  if(theNArg != 2)
  {
    theDI << "Use: getcurvcontinuity {curve or 2dcurve} \n";
    return 1;
  }

  char aContName[7][3] = {"C0",   //0
                          "G1",   //1
                          "C1",   //2
                          "G2",   //3
                          "C2",   //4
                          "C3",   //5
                          "CN"};  //6

  Handle(Geom2d_Curve) GC2d;
  Handle(Geom_Curve) GC3d = DrawTrSurf::GetCurve(theArgv[1]);
  if(GC3d.IsNull())
  {
    GC2d = DrawTrSurf::GetCurve2d(theArgv[1]);
    if(GC2d.IsNull())
    {
      theDI << "Argument is not a 2D or 3D curve!\n";
      return 1;
    }
    else
    {
      theDI << theArgv[1] << " has " << aContName[GC2d->Continuity()] << " continuity.\n";
    }
  }
  else
  {
    theDI << theArgv[1] << " has " << aContName[GC3d->Continuity()] << " continuity.\n";
  }

  return 0;
}
//=======================================================================
//function : repar
//purpose  : 
//=======================================================================
//repar
#include <GeomAPI_CurveChartParameter.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <BSplCLib.hxx>
static Standard_Integer repar( Draw_Interpretor& theDI,
                                            Standard_Integer theNArg,
                                            const char** theArgv)
{
  Handle(Geom_BSplineCurve) C3d = DrawTrSurf::GetBSplineCurve(theArgv[1]);
  Standard_Real defl = .01;
  if(theNArg > 2)
  {
    defl = atof(theArgv[2]);
  }
  Standard_Real angdefl = M_PI/6;
  if(theNArg > 3)
  {
    angdefl = atof(theArgv[3]);
  }
  Standard_Integer ToPrint = 0;
  if(theNArg > 4)
  {
    ToPrint = atoi(theArgv[4]);
  }
  //
  //Prepare Curve chart
  GeomAPI_CurveChartParameter::CurveChart aCrvChart;
  //
  GeomAdaptor_Curve aGAC(C3d);
  GCPnts_TangentialDeflection aSampler(aGAC, angdefl, defl, 3);
  Standard_Integer nbp = aSampler.NbPoints();
  aCrvChart.myNbPnts = nbp;
  aCrvChart.myParams = new TColStd_HArray1OfReal(1, nbp);
  aCrvChart.myScales = new TColStd_HArray1OfReal(1, nbp);
  aCrvChart.myPoints = new TColgp_HArray1OfXYZ(1, nbp);
  aCrvChart.myTangents = new TColgp_HArray1OfXYZ(1, nbp);
  aCrvChart.myMaxDefl = defl;
  aCrvChart.myMaxAngDefl = angdefl;
  //
  TColStd_Array1OfReal aPars(1, nbp);
  Standard_Integer i;
  gp_Pnt aP;
  gp_Vec aV;
  for(i = 1; i <= nbp; ++i)
  {
    Standard_Real par = aSampler.Parameter(i);
    C3d->D1(par, aP, aV);
    aV.Normalize();
    aCrvChart.myPoints->SetValue(i, aP.XYZ());
    aCrvChart.myTangents->SetValue(i, aV.XYZ());
    aPars(i) = par;
  }
  //
  aCrvChart.myScales->SetValue(1, 1.);
  aCrvChart.myParams->SetValue(1, 0.);
  for(i = 2; i < nbp; ++i)
  {
    gp_Vec Cim1(aCrvChart.myPoints->Value(i) - aCrvChart.myPoints->Value(i-1));
    Standard_Real Lim1 = Cim1.Magnitude();
    Cim1.Normalize();
    gp_Vec Ci(aCrvChart.myPoints->Value(i+1) - aCrvChart.myPoints->Value(i));
    Ci.Normalize();
    gp_Vec Ti(aCrvChart.myTangents->Value(i));
    Standard_Real a = Ci.Dot(Ti);
    Standard_Real b = Cim1.Dot(Ti);
    Standard_Real s = b / a;
    Standard_Real sim1 = aCrvChart.myScales->Value(i-1);
    aCrvChart.myScales->SetValue(i, s * sim1);
    Standard_Real t = aCrvChart.myParams->Value(i-1) + Lim1 * sim1;
    aCrvChart.myParams->SetValue(i, t);
  }
  gp_Vec Cim1(aCrvChart.myPoints->Value(nbp) - aCrvChart.myPoints->Value(nbp-1));
  Standard_Real Lim1 = Cim1.Magnitude();
  Standard_Real sim1 = aCrvChart.myScales->Value(nbp-1);
  Standard_Real t = aCrvChart.myParams->Value(nbp-1) + Lim1 * sim1;
  aCrvChart.myParams->SetValue(nbp, t);
  //
  //
  BSplCLib::Reparametrize(aCrvChart.myParams->Value(1), aCrvChart.myParams->Value(nbp), aPars);
  if(ToPrint != 0)
  {
    Standard_Real dmax = -RealLast(), imax = 0;
    for(i = 1; i <= nbp; ++i)
    {
      Standard_Real d = aPars(i) - aCrvChart.myParams->Value(i);
      if(Abs(d) > dmax)
      {
          dmax = Abs(d);
          imax = i;
      }
      cout << i << " " << aPars(i) << " " << aCrvChart.myParams->Value(i) << " " << d << endl;
    }
    cout << "max deviation : " << dmax << " for i = " << imax << endl;
    //
    Handle(TColgp_HArray1OfPnt) aPnts = new TColgp_HArray1OfPnt(1, nbp);
    TColgp_Array1OfVec aTans(1, nbp);
    Handle(TColStd_HArray1OfBoolean) aTFlg = new TColStd_HArray1OfBoolean(1, nbp);
    for(i = 1; i <= nbp; ++i)
    {
      gp_Pnt aP(aCrvChart.myPoints->Value(i));
      gp_Vec aT(aCrvChart.myTangents->Value(i));
      aPnts->SetValue(i, aP);
      aTans(i) =  aT;
      aTFlg->SetValue(i, Standard_True);
    }
    //
    GeomAPI_Interpolate Interpol(aPnts, aCrvChart.myParams, Standard_False, Precision::Confusion());
    Standard_Boolean ToScale = Standard_False;
    Interpol.Load(aTans, aTFlg, ToScale);
    Interpol.Perform();
    if(Interpol.IsDone())
    {
      const Handle(Geom_BSplineCurve)& IntCrv = Interpol.Curve();
      TCollection_AsciiString name(theArgv[1]);
      name.AssignCat("_int");
      DrawTrSurf::Set(name.ToCString(), IntCrv);
    }
    else
    {
      cout << "Interpolation fails" << endl;
    }
    //

  }

  Standard_Real tol = Precision::Confusion();
  GeomAPI_CurveChartParameter Repar(C3d, aCrvChart, tol);
  //
  Standard_Boolean samepar = Repar.IsCurveChartParameter();
  const Handle(Geom_BSplineCurve)& ReparCrv = Repar.ReparCurve();
  const Handle(Law_BSpline)& ReparLaw = Repar.ReparLaw();
  Standard_Real tolreached = Repar.TolReached();
  cout << "samepar, tolreached : " << samepar << " " << tolreached << endl;
  TCollection_AsciiString name(theArgv[1]);
  name.AssignCat("_rep");
  DrawTrSurf::Set(name.ToCString(), ReparCrv);
  //
  tolreached = 0.;
  Handle(Geom_BSplineCurve) ReparCrv1;
  Standard_Boolean isdone = 
    GeomAPI_CurveChartParameter::ReparWithLaw(C3d, ReparLaw, tol, ReparCrv1, tolreached);
  if(isdone)
  {
      name.AssignCat("_1");
      DrawTrSurf::Set(name.ToCString(), ReparCrv1);
      cout << "Test ReparWithLaw: tol = " << tolreached << endl;
  }

  //
  //Display ReparLaw
  Standard_Integer np = ReparLaw->NbPoles();
  TColStd_Array1OfReal lawpoles(1, np);
  ReparLaw->Poles(lawpoles);
  TColgp_Array1OfPnt2d poles  (1, np);
  TColStd_Array1OfReal schoenberg_points(1,np) ;
  Standard_Integer deg = ReparLaw->Degree();
  TColStd_Array1OfInteger mults(1, ReparLaw->NbKnots());
  TColStd_Array1OfReal knots(1, ReparLaw->NbKnots());
  ReparLaw->Multiplicities(mults);
  ReparLaw->Knots(knots);
  Standard_Integer nbfk = BSplCLib::KnotSequenceLength(mults, deg, Standard_False);
  TColStd_Array1OfReal flat_knots(1, nbfk);
  BSplCLib::KnotSequence (knots, mults, deg, Standard_False, flat_knots);
  BSplCLib::BuildSchoenbergPoints(deg, flat_knots, schoenberg_points) ;
  for (i = 1; i <= np; i++) {
    poles(i).SetCoord(schoenberg_points(i), lawpoles(i));
  }
  //  
  Handle(Geom2d_BSplineCurve) replaw =
    new Geom2d_BSplineCurve(poles, knots, mults, deg, Standard_False);
  TCollection_AsciiString name1(theArgv[1]);
  name1.AssignCat("_law");
  DrawTrSurf::Set(name1.ToCString(),replaw);

  return 0;
}
//=======================================================================
//function : repar1
//purpose  : 
//=======================================================================
//repar
#include <GeomAPI_CurveChartParameter.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <BSplCLib.hxx>
static Standard_Integer repar1(Draw_Interpretor& theDI,
  Standard_Integer theNArg,
  const char** theArgv)
{
  Handle(Geom_BSplineCurve) C3d = DrawTrSurf::GetBSplineCurve(theArgv[1]);
  Standard_Real defl = 0.00038339555326448400;
  Standard_Real angdefl = 0.1;
  Standard_Integer ToPrint = 0;
  if (theNArg > 2)
  {
    ToPrint = atoi(theArgv[2]);
  }
  //
  //Prepare Curve chart
  GeomAPI_CurveChartParameter::CurveChart aCrvChart;
  //
  //GeomAdaptor_Curve aGAC(C3d);
  //GCPnts_TangentialDeflection aSampler(aGAC, angdefl, defl, 3);
  Standard_Integer nbp = 4;
  aCrvChart.myNbPnts = nbp;
  aCrvChart.myParams = new TColStd_HArray1OfReal(1, nbp);
  aCrvChart.myScales = new TColStd_HArray1OfReal(1, nbp);
  aCrvChart.myPoints = new TColgp_HArray1OfXYZ(1, nbp);
  aCrvChart.myTangents = new TColgp_HArray1OfXYZ(1, nbp);
  aCrvChart.myMaxDefl = defl;
  aCrvChart.myMaxAngDefl = angdefl;
  //
  TColStd_Array1OfReal aPars(1, nbp);
  aPars(1) = -0.905512071967118; //- 0.045590995014919200;
  aPars(2) = -0.86214986402222715; // 6.9388939039072284e - 017;
  aPars(3) = -0.81591053335525043; // 0.045590998521897241;
  aPars(4) = -0.66536655030555; // 0.17352049169934308;
  //
  aCrvChart.myPoints->SetValue(1, gp_XYZ(-37.189440374433303, -179.60939705152902, 18.090012407095099));
  aCrvChart.myPoints->SetValue(2, gp_XYZ(-37.957066118931799, -134.05822930062362, 19.833816748834401));
  aCrvChart.myPoints->SetValue(3, gp_XYZ(-38.711737341576047, -88.508713273238499, 21.625800911802802));
  aCrvChart.myPoints->SetValue(4, gp_XYZ(-40.730358640490650, 39.290637270081852, 27.022255887704500));
  gp_Dir T1(-0.016962281103315172, 0.99914196743617467, 0.037783725696947892);
  gp_Dir T2(-0.016704267169068841, 0.99910956299647591, 0.038743368301363483);
  gp_Dir T3(-0.016391327687553539, 0.99906893787615003, 0.039907164114485479);
  gp_Dir T4(-0.015001219681028915, 0.99887096257369723, 0.045075087744529223);
  aCrvChart.myTangents->SetValue(1, T1.XYZ());
  aCrvChart.myTangents->SetValue(2, T2.XYZ());
  aCrvChart.myTangents->SetValue(3, T3.XYZ());
  aCrvChart.myTangents->SetValue(4, T4.XYZ());

  Standard_Integer i;
  //
  aCrvChart.myScales->SetValue(1, 0.99999996153870097);
  aCrvChart.myParams->SetValue(1, -0.045590995014919200);
  for (i = 2; i < nbp; ++i)
  {
    gp_Vec Cim1(aCrvChart.myPoints->Value(i) - aCrvChart.myPoints->Value(i - 1));
    Standard_Real Lim1 = Cim1.Magnitude();
    Cim1.Normalize();
    gp_Vec Ci(aCrvChart.myPoints->Value(i + 1) - aCrvChart.myPoints->Value(i));
    Ci.Normalize();
    gp_Vec Ti(aCrvChart.myTangents->Value(i));
    Standard_Real a = Ci.Dot(Ti);
    Standard_Real b = Cim1.Dot(Ti);
    Standard_Real s = b / a;
    Standard_Real sim1 = aCrvChart.myScales->Value(i - 1);
    aCrvChart.myScales->SetValue(i, s * sim1);
    Standard_Real t = aCrvChart.myParams->Value(i - 1) + Lim1 * sim1;
    aCrvChart.myParams->SetValue(i, t);
  }
  gp_Vec Cim1(aCrvChart.myPoints->Value(nbp) - aCrvChart.myPoints->Value(nbp - 1));
  Standard_Real Lim1 = Cim1.Magnitude();
  Standard_Real sim1 = aCrvChart.myScales->Value(nbp - 1);
  Standard_Real t = aCrvChart.myParams->Value(nbp - 1) + Lim1 * sim1;
  aCrvChart.myParams->SetValue(nbp, t);
  aCrvChart.myScales->SetValue(nbp, 0.);
  //
  //
  BSplCLib::Reparametrize(aCrvChart.myParams->Value(1), aCrvChart.myParams->Value(nbp), aPars);
  if (ToPrint != 0)
  {
    cout << "Parameters and scales : " << endl;
    for (i = 1; i <= nbp; ++i)
    {
      cout << aCrvChart.myParams->Value(i) << " " << aCrvChart.myScales->Value(i) << endl;
    }

    Standard_Real dmax = -RealLast(), imax = 0;
    for (i = 1; i <= nbp; ++i)
    {
      Standard_Real d = aPars(i) - aCrvChart.myParams->Value(i);
      if (Abs(d) > dmax)
      {
        dmax = Abs(d);
        imax = i;
      }
      cout << i << " " << aPars(i) << " " << aCrvChart.myParams->Value(i) << " " << d << endl;
    }
    cout << "max deviation : " << dmax << " for i = " << imax << endl;
    //
    Handle(TColgp_HArray1OfPnt) aPnts = new TColgp_HArray1OfPnt(1, nbp);
    TColgp_Array1OfVec aTans(1, nbp);
    Handle(TColStd_HArray1OfBoolean) aTFlg = new TColStd_HArray1OfBoolean(1, nbp);
    for (i = 1; i <= nbp; ++i)
    {
      gp_Pnt aP(aCrvChart.myPoints->Value(i));
      gp_Vec aT(aCrvChart.myTangents->Value(i));
      aPnts->SetValue(i, aP);
      aTans(i) = aT;
      aTFlg->SetValue(i, Standard_True);
    }
    //
    GeomAPI_Interpolate Interpol(aPnts, aCrvChart.myParams, Standard_False, Precision::Confusion());
    Standard_Boolean ToScale = Standard_False;
    Interpol.Load(aTans, aTFlg, ToScale);
    Interpol.Perform();
    if (Interpol.IsDone())
    {
      const Handle(Geom_BSplineCurve)& IntCrv = Interpol.Curve();
      TCollection_AsciiString name(theArgv[1]);
      name.AssignCat("_int");
      DrawTrSurf::Set(name.ToCString(), IntCrv);
    }
    else
    {
      cout << "Interpolation fails" << endl;
    }
    //

  }

  Standard_Real tol = Precision::Confusion();
  GeomAPI_CurveChartParameter Repar(C3d, aCrvChart, tol);
  //
  Standard_Boolean samepar = Repar.IsCurveChartParameter();
  const Handle(Geom_BSplineCurve)& ReparCrv = Repar.ReparCurve();
  const Handle(Law_BSpline)& ReparLaw = Repar.ReparLaw();
  Standard_Real tolreached = Repar.TolReached();
  cout << "samepar, tolreached : " << samepar << " " << tolreached << endl;
  TCollection_AsciiString name(theArgv[1]);
  name.AssignCat("_rep");
  DrawTrSurf::Set(name.ToCString(), ReparCrv);
  //
  tolreached = 0.;
  Handle(Geom_BSplineCurve) ReparCrv1;
  Standard_Boolean isdone =
    GeomAPI_CurveChartParameter::ReparWithLaw(C3d, ReparLaw, tol, ReparCrv1, tolreached);
  if (isdone)
  {
    name.AssignCat("_1");
    DrawTrSurf::Set(name.ToCString(), ReparCrv1);
    cout << "Test ReparWithLaw: tol = " << tolreached << endl;
  }

  //
  //Display ReparLaw
  Standard_Integer np = ReparLaw->NbPoles();
  TColStd_Array1OfReal lawpoles(1, np);
  ReparLaw->Poles(lawpoles);
  TColgp_Array1OfPnt2d poles(1, np);
  TColStd_Array1OfReal schoenberg_points(1, np);
  Standard_Integer deg = ReparLaw->Degree();
  TColStd_Array1OfInteger mults(1, ReparLaw->NbKnots());
  TColStd_Array1OfReal knots(1, ReparLaw->NbKnots());
  ReparLaw->Multiplicities(mults);
  ReparLaw->Knots(knots);
  Standard_Integer nbfk = BSplCLib::KnotSequenceLength(mults, deg, Standard_False);
  TColStd_Array1OfReal flat_knots(1, nbfk);
  BSplCLib::KnotSequence(knots, mults, deg, Standard_False, flat_knots);
  BSplCLib::BuildSchoenbergPoints(deg, flat_knots, schoenberg_points);
  for (i = 1; i <= np; i++) {
    poles(i).SetCoord(schoenberg_points(i), lawpoles(i));
  }
  //  
  Handle(Geom2d_BSplineCurve) replaw =
    new Geom2d_BSplineCurve(poles, knots, mults, deg, Standard_False);
  TCollection_AsciiString name1(theArgv[1]);
  name1.AssignCat("_law");
  DrawTrSurf::Set(name1.ToCString(), replaw);

  return 0;
}
//=======================================================================
//function : repar2
//purpose  : 
//=======================================================================
//repar
#include <GeomAPI_CurveChartParameter.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <BSplCLib.hxx>
static Standard_Integer repar2(Draw_Interpretor& theDI,
  Standard_Integer theNArg,
  const char** theArgv)
{
  Handle(Geom_BSplineCurve) C3d = DrawTrSurf::GetBSplineCurve(theArgv[1]);
  Standard_Real defl = 0.00030905644679046150;
  Standard_Real angdefl = 0.1;
  Standard_Integer ToPrint = 0;
  if (theNArg > 2)
  {
    ToPrint = atoi(theArgv[2]);
  }
  Standard_Real tol = Precision::Confusion();
  if (theNArg > 3)
  {
    tol = atof(theArgv[3]);
  }
  //
  //Prepare Curve chart
  GeomAPI_CurveChartParameter::CurveChart aCrvChart;
  //
  //GeomAdaptor_Curve aGAC(C3d);
  //GCPnts_TangentialDeflection aSampler(aGAC, angdefl, defl, 3);
  Standard_Integer nbp = 8;
  aCrvChart.myNbPnts = nbp;
  aCrvChart.myParams = new TColStd_HArray1OfReal(1, nbp);
  aCrvChart.myScales = new TColStd_HArray1OfReal(1, nbp);
  aCrvChart.myPoints = new TColgp_HArray1OfXYZ(1, nbp);
  aCrvChart.myTangents = new TColgp_HArray1OfXYZ(1, nbp);
  aCrvChart.myMaxDefl = defl;
  aCrvChart.myMaxAngDefl = angdefl;
  //
  aCrvChart.myPoints->SetValue(1, gp_XYZ(-53.8965422523175, 100.00000000003, 72.2722557505194));
  aCrvChart.myPoints->SetValue(2, gp_XYZ(-60.2697677808272, 100, 72.2722557505166));
  aCrvChart.myPoints->SetValue(3, gp_XYZ(-61.8630741629532, 99.6962053205614, 72.2436732600066));
  aCrvChart.myPoints->SetValue(4, gp_XYZ(-63.0832105762871, 98.9310977002817, 72.164142039151));
  aCrvChart.myPoints->SetValue(5, gp_XYZ(-63.852743662729, 97.8709421578265, 72.0360113528936));
  aCrvChart.myPoints->SetValue(6, gp_XYZ(-64.1126772863536, 96.788064941547, 71.8834912689113));
  aCrvChart.myPoints->SetValue(7, gp_XYZ(-64.1009963756935, 96.376485146978, 71.8197477127767));
  aCrvChart.myPoints->SetValue(8, gp_XYZ(-63.7174864703188, 93.1457451938308, 71.2073239231896));
  //
  gp_Dir T1(-216604.25448034232, -7.3119292791787354e-007, -6.6720271436586147e-008);
  gp_Dir T2(-24.173117027278657, -0.013091332441418592, -0.0011950778781740488);
  gp_Dir T3(-36964.411460859883, -14733.661069011772, -1427.4583441875998);
  gp_Dir T4(-28601.464865167934, -26636.634072160887, -2957.2426045123520);
  gp_Dir T5(-14664.453305107543, -32503.192498234443, -4248.8831439433598);
  gp_Dir T6(-1376.5823942200577, -31882.644499282884, -4814.2956623589507);
  gp_Dir T7(1.1360579011864260, -11.459251380628993, -1.8192102357191207);
  gp_Dir T8(23047.841753910630, -167781.05080932155, -37032.553153923756);
  //
  aCrvChart.myTangents->SetValue(1, T1.XYZ());
  aCrvChart.myTangents->SetValue(2, T2.XYZ());
  aCrvChart.myTangents->SetValue(3, T3.XYZ());
  aCrvChart.myTangents->SetValue(4, T4.XYZ());
  aCrvChart.myTangents->SetValue(5, T5.XYZ());
  aCrvChart.myTangents->SetValue(6, T6.XYZ());
  aCrvChart.myTangents->SetValue(7, T7.XYZ());
  aCrvChart.myTangents->SetValue(8, T8.XYZ());

  Standard_Integer i;
  //
  aCrvChart.myScales->SetValue(1, 0.98215119675684404);
  aCrvChart.myParams->SetValue(1, -0.0062594710800271403);
  gp_XYZ aPnti1, aPnti2, aPnti3;
  aPnti1 = aCrvChart.myPoints->Value(1);
  aPnti2 = aCrvChart.myPoints->Value(2);
  Standard_Real aFi, aTN;
  aFi = aCrvChart.myScales->Value(1);
  aTN = aCrvChart.myParams->Value(1);
  Standard_Real aCi = (aPnti2 - aPnti1).Modulus();
  Standard_Boolean IsHaveCompute = Standard_False;
  for (Standard_Integer anIndex = 2; anIndex < nbp; anIndex++) {

    aPnti1 = aCrvChart.myPoints->Value(anIndex - 1);
    aPnti2 = aCrvChart.myPoints->Value(anIndex);
    aPnti3 = aCrvChart.myPoints->Value(anIndex + 1);

    gp_XYZ aXYZ1 = aCrvChart.myTangents->Value(anIndex);
    gp_XYZ aXYZ2 = aPnti3 - aPnti2;
    gp_XYZ aXYZ3 = aPnti2 - aPnti1;

    if (aXYZ1.Modulus() > gp::Resolution())
      aXYZ1 = aXYZ1 / aXYZ1.Modulus();
    if (aXYZ2.Modulus() > gp::Resolution())
      aXYZ2 = aXYZ2 / aXYZ2.Modulus();
    if (aXYZ3.Modulus() > gp::Resolution())
      aXYZ3 = aXYZ3 / aXYZ3.Modulus();

    Standard_Real cosA = aXYZ1 * aXYZ2;
    Standard_Real cosB = aXYZ1 * aXYZ3;

    aTN += aCi*aFi;
    // remember the parameters for interpolation
    aCrvChart.myParams->SetValue(anIndex, aTN);

    // next step`s values
    if (Abs(cosA) < gp::Resolution())
      continue;
    aFi = (cosB / cosA) * aFi;
    aCrvChart.myScales->SetValue(anIndex, aFi);
    aCi = (aPnti3 - aPnti2).Modulus();
    IsHaveCompute = Standard_True;
  }
  aTN += aCi*aFi;
  aCrvChart.myParams->SetValue(nbp, aTN);
  aCrvChart.myScales->SetValue(nbp, aFi); //To avoid not inicialized value only

  //
  //
  if (ToPrint != 0)
  {
    cout << "Parameters and scales : " << endl;
    for (i = 1; i <= nbp; ++i)
    {
      cout << aCrvChart.myParams->Value(i) << " " << aCrvChart.myScales->Value(i) << endl;
    }

    cout << endl;
    //
    Handle(TColgp_HArray1OfPnt) aPnts = new TColgp_HArray1OfPnt(1, nbp);
    TColgp_Array1OfVec aTans(1, nbp);
    Handle(TColStd_HArray1OfBoolean) aTFlg = new TColStd_HArray1OfBoolean(1, nbp);
    for (i = 1; i <= nbp; ++i)
    {
      gp_Pnt aP(aCrvChart.myPoints->Value(i));
      gp_Vec aT(aCrvChart.myTangents->Value(i));
      aPnts->SetValue(i, aP);
      aTans(i) = aT;
      aTFlg->SetValue(i, Standard_True);
    }
    //
    GeomAPI_Interpolate Interpol(aPnts, aCrvChart.myParams, Standard_False, Precision::Confusion());
    Standard_Boolean ToScale = Standard_False;
    Interpol.Load(aTans, aTFlg, ToScale);
    Interpol.Perform();
    if (Interpol.IsDone())
    {
      const Handle(Geom_BSplineCurve)& IntCrv = Interpol.Curve();
      TCollection_AsciiString name(theArgv[1]);
      name.AssignCat("_int");
      DrawTrSurf::Set(name.ToCString(), IntCrv);
    }
    else
    {
      cout << "Interpolation fails" << endl;
    }
    //

  }

  GeomAPI_CurveChartParameter Repar(C3d, aCrvChart, tol);
  //
  Standard_Boolean samepar = Repar.IsCurveChartParameter();
  const Handle(Geom_BSplineCurve)& ReparCrv = Repar.ReparCurve();
  const Handle(Law_BSpline)& ReparLaw = Repar.ReparLaw();
  Standard_Real tolreached = Repar.TolReached();
  cout << endl;
  cout << "samepar, tolreached : " << samepar << " " << tolreached << endl;
  TCollection_AsciiString name(theArgv[1]);
  name.AssignCat("_rep");
  DrawTrSurf::Set(name.ToCString(), ReparCrv);
  //
  // Test parameters
  cout << endl;
  cout << "-------- Test samepar --------" << endl;
  for (i = 1; i <= nbp; ++i)
  {
    gp_XYZ aP = ReparCrv->Value(aCrvChart.myParams->Value(i)).XYZ();
    Standard_Real d = (aP - aCrvChart.myPoints->Value(i)).Modulus();
    cout << i << " " << d << endl;
  }

  tolreached = 0.;
  Handle(Geom_BSplineCurve) ReparCrv1;
  Standard_Boolean isdone =
    GeomAPI_CurveChartParameter::ReparWithLaw(C3d, ReparLaw, tol, ReparCrv1, tolreached);
  if (isdone)
  {
    name.AssignCat("_1");
    DrawTrSurf::Set(name.ToCString(), ReparCrv1);
    cout << "Test ReparWithLaw: tol = " << tolreached << endl;
  }

  //
  //Display ReparLaw
  Standard_Integer np = ReparLaw->NbPoles();
  TColStd_Array1OfReal lawpoles(1, np);
  ReparLaw->Poles(lawpoles);
  TColgp_Array1OfPnt2d poles(1, np);
  TColStd_Array1OfReal schoenberg_points(1, np);
  Standard_Integer deg = ReparLaw->Degree();
  TColStd_Array1OfInteger mults(1, ReparLaw->NbKnots());
  TColStd_Array1OfReal knots(1, ReparLaw->NbKnots());
  ReparLaw->Multiplicities(mults);
  ReparLaw->Knots(knots);
  Standard_Integer nbfk = BSplCLib::KnotSequenceLength(mults, deg, Standard_False);
  TColStd_Array1OfReal flat_knots(1, nbfk);
  BSplCLib::KnotSequence(knots, mults, deg, Standard_False, flat_knots);
  BSplCLib::BuildSchoenbergPoints(deg, flat_knots, schoenberg_points);
  for (i = 1; i <= np; i++) {
    poles(i).SetCoord(schoenberg_points(i), lawpoles(i));
  }
  //  
  Handle(Geom2d_BSplineCurve) replaw =
    new Geom2d_BSplineCurve(poles, knots, mults, deg, Standard_False);
  TCollection_AsciiString name1(theArgv[1]);
  name1.AssignCat("_law");
  DrawTrSurf::Set(name1.ToCString(), replaw);

  return 0;
}

//=======================================================================
//function : CurveCommands
//purpose  : 
//=======================================================================
void  GeometryTest::CurveCommands(Draw_Interpretor& theCommands)
{
  
  static Standard_Boolean loaded = Standard_False;
  if (loaded) return;
  loaded = Standard_True;
  
  DrawTrSurf::BasicCommands(theCommands);
  
  const char* g;
  
  g = "GEOMETRY curves creation";

  theCommands.Add("law",
		  "law  name degree nbknots  knot, umult  value",
		  __FILE__,
		  polelaw,g);

  theCommands.Add("to2d","to2d c2dname c3d [plane (XOY)]",
		  __FILE__,
		  to2d,g);

  theCommands.Add("to3d","to3d c3dname c2d [plane (XOY)]",
		  __FILE__,
		  to3d,g);

  theCommands.Add("gproject",
		  "gproject : [projectname] curve surface",
		  __FILE__,
		  gproject,g);
  
  theCommands.Add("project",
		  "project : no args to have help",
		  __FILE__,
		  project,g);
  
  theCommands.Add("projonplane",
		  "projonplane r C3d Plane [dx dy dz] [0/1]",
		  projonplane);

  theCommands.Add("bisec",
		  "bisec result line/circle/point line/circle/point",
		  __FILE__,
		  bisec, g);

  g = "GEOMETRY Curves and Surfaces modification";


  theCommands.Add("movelaw",
                  "movelaw name u  x  tx [ constraint = 0]",
		  __FILE__,
		  movelaw,g) ;



  g = "GEOMETRY intersections";

  theCommands.Add("intersect",
		  "intersect result surf1/curv1 surf2 [tolerance]\n\t\t  "
                  "intersect result surf1 surf2 [u1 v1 u2 v2] [U1F U1L V1F V1L U2F U2L V2F V2L] [tolerance]",
		  __FILE__,
		  intersection,g);

  theCommands.Add("crvpoints",
		  "crvpoints result curv deflection",
		  __FILE__,
		  crvpoints,g);

  theCommands.Add("crvtpoints",
		  "crvtpoints result curv deflection angular deflection - tangential deflection points",
		  __FILE__,
		  crvtpoints,g);
  
  theCommands.Add("uniformAbscissa",
		  "uniformAbscissa Curve nbPnt",
		  __FILE__,
                  uniformAbscissa,g);

  theCommands.Add("uniformAbscissaEl",
		  "uniformAbscissaEl maxR minR nbPnt",
		  __FILE__,  EllipsUniformAbscissa,g);

  theCommands.Add("discrCurve",
    "discrCurve polyline curve params\n"
    "Approximates a curve by a polyline (first degree B-spline).\n"
    "nbPnts number - creates polylines with the number points\n"
    "uniform 0 | 1 - creates polyline with equal length segments",
    __FILE__,  discrCurve, g);

  theCommands.Add("mypoints",
		  "mypoints result curv deflection",
		  __FILE__,
		  mypoints,g);
  theCommands.Add("surfpoints",
		  "surfoints result surf deflection",
		  __FILE__,
		  surfpoints,g);

  theCommands.Add("getcurvcontinuity",
		  "getcurvcontinuity {curve or 2dcurve}: \n\tReturns the continuity of the given curve",
		  __FILE__,
		  GetCurveContinuity,g);

  theCommands.Add("repar",
		  "repar curve",
		  __FILE__,
		  repar,g);
  theCommands.Add("repar1",
    "repar1 curve [toprint]",
    __FILE__,
    repar1, g);

  theCommands.Add("repar2",
    "repar2 curve [toprint]",
    __FILE__,
    repar2, g);

}

