//-Copyright: Open CASCADE 2016
// Created on: 2016-08-01
// Created by: Mikhail Sazonov
// Copyright (c) 2016 OPEN CASCADE SAS
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

#include <ShapeUpgrade_CombineToCylinder.hxx>
#include <BRep_Builder.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepTools.hxx>
#include <BRepTools_ReShape.hxx>
#include <ElCLib.hxx>
#include <GCPnts_AbscissaPoint.hxx>
#include <Geom_Circle.hxx>
#include <Geom_BSplineCurve.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <gce_MakeCirc.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax3.hxx>
#include <gp_Circ.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Pln.hxx>
#include <IntAna_IntConicQuad.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <ShapeFix_Shape.hxx>
#include <TColStd_MapOfInteger.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_MapOfShape.hxx>
#include <Precision.hxx>

//=======================================================================
//function : ShapeUpgrade_CombineToCylinder
//purpose  : 
//=======================================================================
ShapeUpgrade_CombineToCylinder::ShapeUpgrade_CombineToCylinder()
  : myLinTol(Precision::Confusion()),
    myAngTol(Precision::Angular()),
    myNbNewFaces(0),
    myNbReplacedFaces(0),
    myDone(Standard_False)
{
  myContext = new BRepTools_ReShape;
}

//=======================================================================
//function : sortEdges
//purpose  : 
//=======================================================================
static void sortEdges(const TopoDS_Shape& theShape,
                            const TopTools_IndexedDataMapOfShapeListOfShape& theMapEF,
                            const Standard_Real theLinTol,
                            const Standard_Real theAngTol,
                            TopTools_MapOfShape& theStopEdges,
                            NCollection_DataMap<TopoDS_Shape,gp_Cylinder>& theMapECyl)
{
  TopExp_Explorer anEx(theShape, TopAbs_EDGE);
  for (; anEx.More(); anEx.Next())
  {
    const TopoDS_Edge& anEdge = TopoDS::Edge(anEx.Current());
    const TopTools_ListOfShape* pFaces = theMapEF.Seek(anEdge);
    if (!pFaces)
      continue;

    // check for manifold edge
    if (pFaces->Extent() != 2)
    {
      theStopEdges.Add(anEdge);
      continue;
    }

    // Get curve of the edge and check if it is a line
    BRepAdaptor_Curve aCurve(anEdge);
    if (aCurve.GetType() != GeomAbs_Line)
    {
      theStopEdges.Add(anEdge);
      continue;
    }
    // Check if the edge has enough length
    Standard_Real aLength = GCPnts_AbscissaPoint::Length(aCurve);
    if (aLength < 2.*theLinTol)
    {
      theStopEdges.Add(anEdge);
      continue;
    }

    // check that both faces are planar, if no then add the edge to stop list
    TopoDS_Face aFaces[2] = { TopoDS::Face(pFaces->First()), TopoDS::Face(pFaces->Last()) };
    BRepAdaptor_Surface aSurf[2] = {
      BRepAdaptor_Surface(aFaces[0], Standard_False),
      BRepAdaptor_Surface(aFaces[1], Standard_False) };
    if (aSurf[0].GetType() != GeomAbs_Plane || aSurf[1].GetType() != GeomAbs_Plane)
    {
      theStopEdges.Add(anEdge);
      continue;
    }

    // check if edge is sharp, if yes then add it to stop list
    gp_Pln aPl1 = aSurf[0].Plane();
    gp_Pln aPl2 = aSurf[1].Plane();
    if (aPl1.Axis().Direction().Angle(aPl2.Axis().Direction()) > theAngTol)
    {
      theStopEdges.Add(anEdge);
      continue;
    }

    // Create plane perpendicular to the edge
    gp_Lin aLin = aCurve.Line();
    gp_Pln aPln(aLin.Location(), aLin.Direction());

    // Get opposite edges on each face
    TopoDS_Edge anOpEdges[2];
    gp_Lin anOpLines[2];
    Standard_Integer i;
    for (i = 0; i < 2; i++)
    {
      TopExp_Explorer anEx1(aFaces[i], TopAbs_EDGE);
      Standard_Integer nbE = 0;
      for (; anEx1.More(); anEx1.Next(), nbE++)
      {
        if (anOpEdges[i].IsNull())
        {
          const TopoDS_Edge& aE = TopoDS::Edge(anEx1.Current());
          if (!aE.IsSame(anEdge))
          {
            BRepAdaptor_Curve aC(aE);
            if (aC.GetType() == GeomAbs_Line)
            {
              gp_Lin aL = aC.Line();
              if (aL.Direction().IsParallel(aLin.Direction(), theAngTol))
              {
                anOpEdges[i] = aE;
                anOpLines[i] = aL;
              }
            }
          }
        }
      }
      if (nbE != 4)
        break; // face has non-rectangular form
    }
    if (anOpEdges[0].IsNull() || anOpEdges[1].IsNull())
    {
      theStopEdges.Add(anEdge);
      continue;
    }

    // intersect all 3 edges with the plane
    gp_Lin aLines[3] = { anOpLines[0], aLin, anOpLines[1] };
    gp_Pnt anIntPnt[3];
    for (i = 0; i < 3; i++)
    {
      IntAna_IntConicQuad anInter(aLines[i], aPln, theAngTol);
      if (!anInter.IsDone() || anInter.NbPoints() != 1)
        break;
      anIntPnt[i] = anInter.Point(1);
    }
    if (i < 3)
    {
      theStopEdges.Add(anEdge);
      continue;
    }

    // check that distances to points on opposite edges are equal
    Standard_Real aDist1 = anIntPnt[0].Distance(anIntPnt[1]);
    Standard_Real aDist2 = anIntPnt[2].Distance(anIntPnt[1]);
    if (fabs(aDist1 - aDist2) > theLinTol)
    {
      theStopEdges.Add(anEdge);
      continue;
    }

    // compute the circle radius
    gce_MakeCirc aCircMaker(anIntPnt[0], anIntPnt[1], anIntPnt[2]);
    if (!aCircMaker.IsDone())
    {
      theStopEdges.Add(anEdge);
      continue;
    }
    Standard_Real aRadius = aCircMaker.Value().Radius();
    const gp_Ax1& aCylAxis = aCircMaker.Value().Axis();
    gp_Cylinder aCyl(gp_Ax3(aCylAxis.Location(), aCylAxis.Direction()), aRadius);
    theMapECyl.Bind(anEdge, aCyl);
  }
}

//=======================================================================
//function : updateBoundary
//purpose  : 
//=======================================================================
static void updateBoundary(TopTools_ListOfShape& theBoundary,
                           const TopoDS_Shape& theFace,
                           const TopoDS_Shape& theConnectingEdge,
                           const TopTools_MapOfShape& theStopEdges)
{
  // iterate on all edges of the face
  TopExp_Explorer anEx(theFace, TopAbs_EDGE);
  for (; anEx.More(); anEx.Next())
  {
    const TopoDS_Edge& anEdge = TopoDS::Edge(anEx.Current());
    if (anEdge.IsSame(theConnectingEdge) || theStopEdges.Contains(anEdge))
      continue; // skip stop edges
    // find this edge in the boundary
    TopTools_ListIteratorOfListOfShape anIt(theBoundary);
    while (anIt.More())
    {
      if (anIt.Value().IsSame(anEdge))
        break;
      anIt.Next();
    }
    if (anIt.More())
    {
      // found, remove it from boundary
      theBoundary.Remove(anIt);
    }
    else
    {
      // not found, add to boundary
      theBoundary.Append(anEdge);
    }
  }
}

//=======================================================================
//function : Auxiliary structures
//purpose  : 
//=======================================================================
namespace CombineToCylinder
{
  struct Area
  {
    TopoDS_Compound    Faces;
    const gp_Cylinder* pCylinder;
    Standard_Integer   nbFaces;

    Area() : pCylinder(0L), nbFaces(0)
    {}
    void Init()
    {
      Faces.Nullify();
      nbFaces = 0;
      BRep_Builder().MakeCompound(Faces);
    }
    void AddFace(const TopoDS_Shape& theFace)
    {
      BRep_Builder().Add(Faces, theFace);
      nbFaces++;
    }
  };
}

//=======================================================================
//function : EqualCylinders
//purpose  : 
//=======================================================================
inline Standard_Boolean EqualCylinders(const gp_Cylinder& theCyl1,
                                       const gp_Cylinder& theCyl2,
                                       const Standard_Real theLinTol,
                                       const Standard_Real theAngTol)
{
  // check radii
  if (fabs(theCyl1.Radius() - theCyl2.Radius()) > theLinTol)
    return Standard_False;
  // check if parallel
  if (!theCyl1.Axis().Direction().IsParallel(theCyl2.Axis().Direction(), theAngTol))
    return Standard_False;
  // check distance between axes
  gp_XYZ aVecO1O2 = theCyl2.Location().XYZ() - theCyl1.Location().XYZ();
  Standard_Real aDist1 = aVecO1O2.CrossSquareMagnitude(theCyl2.Axis().Direction().XYZ());
  Standard_Real aDist2 = aVecO1O2.CrossSquareMagnitude(theCyl1.Axis().Direction().XYZ());
  Standard_Real aTol = theLinTol * theLinTol;
  return aDist1 <= aTol && aDist2 < aTol;
}

//=======================================================================
//function : collectAreas
//purpose  : 
//=======================================================================
static void collectAreas(const TopoDS_Shape& theShape,
                         const TopTools_IndexedDataMapOfShapeListOfShape& theMapEF,
                         const TopTools_MapOfShape& theStopEdges,
                         const NCollection_DataMap<TopoDS_Shape, gp_Cylinder>& theMapECyl,
                         const Standard_Real theLinTol,
                         const Standard_Real theAngTol,
                         NCollection_List<CombineToCylinder::Area>& theAreas)
{
  TopTools_MapOfShape aPassedFaces;

  // Repeat while there are unprocessed faces
  for (;;)
  {
    // get first unprocessed face
    TopoDS_Shape aCurFace;
    TopExp_Explorer anEx(theShape, TopAbs_FACE);
    for (; anEx.More(); anEx.Next())
    {
      aCurFace = anEx.Current();
      if (!aPassedFaces.Contains(aCurFace))
        break;
    }
    if (!anEx.More())
      break; // all are processed, stop

    // Start area with it
    CombineToCylinder::Area anArea;
    anArea.Init();
    anArea.AddFace(aCurFace);
    aPassedFaces.Add(aCurFace);

    // Define edges of boundary of area
    TopTools_ListOfShape aBoundary;
    updateBoundary(aBoundary, aCurFace, TopoDS_Shape(), theStopEdges);

    // Add all neighbors to the area while there are unprocessed edges of boundary
    while (!aBoundary.IsEmpty())
    {
      TopTools_ListIteratorOfListOfShape anIt(aBoundary);
      while (anIt.More())
      {
        const TopoDS_Shape& anEdge = anIt.Value();
        const gp_Cylinder* pCyl = theMapECyl.Seek(anEdge);
        const TopTools_ListOfShape* pFaces = theMapEF.Seek(anEdge);
        Standard_ASSERT_RAISE(pCyl && pFaces && pFaces->Extent() == 2, "collectAreas");
        if (!anArea.pCylinder || EqualCylinders(*anArea.pCylinder, *pCyl, theLinTol, theAngTol))
        {
          anArea.pCylinder = pCyl;
          aCurFace = (aPassedFaces.Contains(pFaces->First()) ? pFaces->Last() : pFaces->First());
          if (!aPassedFaces.Contains(aCurFace))
          {
            // Update boundary adding the next face
            updateBoundary(aBoundary, aCurFace, anEdge, theStopEdges);
            anArea.AddFace(aCurFace);
            aPassedFaces.Add(aCurFace);
          }
        }
        aBoundary.Remove(anIt);
      }
    }

    // check if area is well formed
    if (anArea.pCylinder && anArea.nbFaces > 2)
    {
      // Add area to the result list
      theAreas.Append(anArea);
    }
  }
}

//=======================================================================
//function : getNextEdge
//purpose  : 
//=======================================================================
static Standard_Boolean getNextEdge(const TopTools_ListOfShape& theChain,
                                    const Standard_Boolean theFwd,
                                    const TopTools_IndexedDataMapOfShapeListOfShape& theMapVE,
                                    const Standard_Real theSmoothAng,
                                    TopoDS_Edge& theNextEdge)
{
  // Get end edge and vertex
  TopoDS_Edge anEndE, anOppoEdge;
  TopoDS_Vertex anEndV;
  if (theFwd)
  {
    anEndE = TopoDS::Edge(theChain.Last());
    anOppoEdge = TopoDS::Edge(theChain.First());
    anEndV = TopExp::LastVertex(anEndE, Standard_True);
  }
  else
  {
    anEndE = TopoDS::Edge(theChain.First());
    anOppoEdge = TopoDS::Edge(theChain.Last());
    anEndV = TopExp::FirstVertex(anEndE, Standard_True);
  }

  // Get next edge using topology map; reject the case of multi-connected vertex,
  // when there are more than one candidate edge
  const TopTools_ListOfShape& aConnEdges = theMapVE.FindFromKey(anEndV);
  // Note! aConnEdges can contain duplicates
  theNextEdge.Nullify();
  TopTools_ListIteratorOfListOfShape itL(aConnEdges);
  for (; itL.More(); itL.Next())
  {
    const TopoDS_Shape& aE = itL.Value();
    if (aE.IsSame(anEndE))
      continue;
    if (theNextEdge.IsNull())
      theNextEdge = TopoDS::Edge(aE);
    else if(!aE.IsSame(theNextEdge))
      return Standard_False; // multi-connected vertex
  }

  // check for closed chain
  if (theNextEdge.IsSame(anOppoEdge))
    return Standard_False;

  // Orient the next edge
  TopoDS_Vertex aNextEndV = (theFwd ? TopExp::FirstVertex(theNextEdge, Standard_True) :
    TopExp::LastVertex(theNextEdge, Standard_True));
  if (!aNextEndV.IsSame(anEndV))
    theNextEdge.Reverse();

  if (theSmoothAng > 0.)
  {
    // Check that the edge is linear
    BRepAdaptor_Curve aNextC(theNextEdge);
    if (aNextC.GetType() != GeomAbs_Line)
      return Standard_False;

    // Verify smoothness of connection
    BRepAdaptor_Curve anEndC(anEndE);
    gp_Lin anEndLin = anEndC.Line();
    gp_Lin aNextLin = aNextC.Line();
    const gp_Dir& anEndDir = anEndLin.Direction();
    const gp_Dir& aNextDir = aNextLin.Direction();
    if (!anEndDir.IsParallel(aNextDir, theSmoothAng))
      return Standard_False;
  }
  return Standard_True;
}

//=======================================================================
//function : makeEdgesChain
//purpose  : 
//=======================================================================
static void makeEdgesChain(TopTools_ListOfShape& theChain,
                           const TopTools_IndexedDataMapOfShapeListOfShape& theMapVE,
                           const Standard_Real theSmoothAng)
{
  // Cycle for both ends of the chain
  Standard_Boolean isFwd[2] = { Standard_True, Standard_False };
  for (Standard_Integer i = 0; i < 2; i++)
  {
    TopoDS_Edge aNextEdge;
    while (getNextEdge(theChain, isFwd[i], theMapVE, theSmoothAng, aNextEdge))
    {
      // add next edge to the chain
      if (isFwd[i])
        theChain.Append(aNextEdge);
      else
        theChain.Prepend(aNextEdge);
    }
  }
}

//=======================================================================
//function : createCylFaceFromArea
//purpose  : 
//=======================================================================
static TopoDS_Face createCylFaceFromArea(const TopoDS_Compound& theFaces,
                                         const gp_Cylinder& theCylinder)
{
  // Find boundary edges of the area
  TopTools_IndexedDataMapOfShapeListOfShape aMapEF, aMapVE;
  TopExp::MapShapesAndAncestors(theFaces, TopAbs_EDGE, TopAbs_FACE, aMapEF);
  TopTools_ListOfShape aBoundary;
  Standard_Integer i;
  for (i = 1; i <= aMapEF.Extent(); i++)
  {
    if (aMapEF(i).Extent() == 1)
    {
      const TopoDS_Shape& aE = aMapEF.FindKey(i);
      aBoundary.Append(aE);
      TopExp::MapShapesAndAncestors(aE, TopAbs_VERTEX, TopAbs_EDGE, aMapVE);
    }
  }

  // Check the case of closed cylinder. In this case there will be two 
  // distinct chains of boundary edges, and we need to create a seam edge
  TopoDS_Edge aSeamEdge;
  TopTools_ListOfShape aChain;
  aChain.Append(aBoundary.First());
  makeEdgesChain(aChain, aMapVE, 0.);
  TopTools_MapOfShape aMapChain;
  TopTools_ListIteratorOfListOfShape itC(aChain);
  for (; itC.More(); itC.Next())
    aMapChain.Add(itC.Value());
  TopTools_ListIteratorOfListOfShape itB(aBoundary);
  for (; itB.More(); itB.Next())
  {
    if (!aMapChain.Contains(itB.Value()))
      break;
  }
  if (itB.More())
  {
    // It is the case.
    // Get any multi-connected edge and use it as a seam
    for (i = 1; i <= aMapEF.Extent(); i++)
      if (aMapEF(i).Extent() == 2)
        aSeamEdge = TopoDS::Edge(aMapEF.FindKey(i));
  }

  // Build new cylindrical face
  TopoDS_Wire aWire;
  BRep_Builder().MakeWire(aWire);
  for (itB.Init(aBoundary); itB.More(); itB.Next())
  {
    BRep_Builder().Add(aWire, itB.Value());
  }
  if (!aSeamEdge.IsNull())
  {
    BRep_Builder().Add(aWire, aSeamEdge);
    BRep_Builder().Add(aWire, aSeamEdge.Reversed());
  }
  TopoDS_Face aCylFace = BRepBuilderAPI_MakeFace(theCylinder, aWire);

  // Check orientation of an initial face, so that possibly to reverse the new face.
  // The new cylindrical surface always has normal directed outside of cylinder.
  // If an initial face has normal (taking face orientation into account) directed 
  // inside then we must reverse the new face.
  const TopoDS_Edge& aE = TopoDS::Edge(aBoundary.First());
  const TopoDS_Face& aF = TopoDS::Face(aMapEF.FindFromKey(aE).First());
  BRepAdaptor_Surface aSurf(aF, Standard_False);
  Standard_Real aParF, aParL;
  Handle(Geom2d_Curve) aPCurve = BRep_Tool::CurveOnSurface(aE, aF, aParF, aParL);
  Standard_ASSERT_RAISE(!aPCurve.IsNull(), "createCylFaceFromArea");
  gp_Pnt2d aP2d = aPCurve->Value(aParF);
  gp_Pnt aP;
  gp_Vec aDU, aDV;
  aSurf.D1(aP2d.X(), aP2d.Y(), aP, aDU, aDV);
  gp_XYZ aN = aDU.XYZ() ^ aDV.XYZ();
  if (aF.Orientation() == TopAbs_REVERSED)
    aN.Reverse();
  gp_XYZ aVecFromAxis = aP.XYZ() - theCylinder.Location().XYZ();
  if (aVecFromAxis * aN < 0.)
  {
    // need reverse
    aCylFace.Reverse();
  }
  return aCylFace;
}

//=======================================================================
//function : updateVertexOnCirc
//purpose  : 
//=======================================================================
static void updateVertexOnCirc(const TopoDS_Vertex& theV,
                               const gp_Circ& theCirc,
                               Standard_Real& thePar)
{
  Standard_Real aTol = BRep_Tool::Tolerance(theV);
  gp_Pnt aP = BRep_Tool::Pnt(theV);
  thePar = ElCLib::Parameter(theCirc, aP);
  gp_Pnt aProj = ElCLib::Value(thePar, theCirc);
  Standard_Real aDist = aProj.SquareDistance(aP);
  if (aDist > aTol*aTol)
    BRep_Builder().UpdateVertex(theV, sqrt(aDist)*1.001);
}

//=======================================================================
//function : createCurveEdge
//purpose  : 
//=======================================================================
static TopoDS_Edge createCurveEdge(const TopTools_ListOfShape& theChain,
                                   const TopTools_IndexedDataMapOfShapeListOfShape& theMapEF,
                                   const Standard_Real theLinTol)
{
  // Find a cylindrical face common for all edges
  TopTools_ListOfShape aCylFaces;
  TopTools_ListIteratorOfListOfShape itL(theChain);
  for (; itL.More(); itL.Next())
  {
    const TopoDS_Shape& aE = itL.Value();
    const TopTools_ListOfShape& aFaces = theMapEF.FindFromKey(aE);
    // get cyl faces of the current edge
    TopTools_ListOfShape aCurCylFaces;
    TopTools_ListIteratorOfListOfShape itF(aFaces);
    for (; itF.More(); itF.Next())
    {
      const TopoDS_Face& aF = TopoDS::Face(itF.Value());
      BRepAdaptor_Surface aSurf(aF, Standard_False);
      if (aSurf.GetType() == GeomAbs_Cylinder)
        aCurCylFaces.Append(aF);
    }

    if (aCylFaces.IsEmpty())
      // initialize cyl faces with the first edge
      aCylFaces.Append(aCurCylFaces);
    else
    {
      // filter cyl faces leaving only common ones
      TopTools_ListIteratorOfListOfShape itCF(aCylFaces);
      while (itCF.More())
      {
        const TopoDS_Shape& aF = itCF.Value();
        TopTools_ListIteratorOfListOfShape itCF1(aCurCylFaces);
        for (; itCF1.More(); itCF1.Next())
          if (itCF1.Value().IsSame(aF))
            break;
        if (!itCF1.More())
          aCylFaces.Remove(itCF);
        else
          itCF.Next();
      }
    }
    if (aCylFaces.IsEmpty())
      return TopoDS_Edge(); // no common cyl faces
  }
  if (aCylFaces.IsEmpty())
    return TopoDS_Edge();
  const TopoDS_Face& aCylFace = TopoDS::Face(aCylFaces.First());
  BRepAdaptor_Surface aCylSurf(aCylFace, Standard_False);
  gp_Cylinder aCyl = aCylSurf.Cylinder();
  const gp_Dir& aCylDir = aCyl.Axis().Direction();
  const gp_Pnt& aCylLoc = aCyl.Location();
  const Standard_Real aRadius = aCyl.Radius();

  // Check if the chain is closed
  TopoDS_Vertex aVF, aVL;
  aVF = TopoDS::Vertex(TopExp::FirstVertex(TopoDS::Edge(theChain.First()), Standard_True));
  aVL = TopoDS::Vertex(TopExp::LastVertex(TopoDS::Edge(theChain.Last()), Standard_True));
  Standard_Boolean isClosed = aVF.IsSame(aVL);

  // Check if all points are in plane perpendicular to the cylinder axis;
  // in such case the curve is a circle
  TColgp_Array1OfPnt aPnts(1, theChain.Extent()+1);
  Standard_Real aTol = 0.;
  Standard_Integer i = 1;
  for (itL.Init(theChain); itL.More(); itL.Next(), i++)
  {
    const TopoDS_Edge& aE = TopoDS::Edge(itL.Value());
    TopoDS_Shape aV = TopExp::FirstVertex(aE, Standard_True);
    aPnts(i) = BRep_Tool::Pnt(TopoDS::Vertex(aV));
    aTol = Max(aTol, BRep_Tool::Tolerance(aE));
  }
  aPnts(i) = BRep_Tool::Pnt(aVL);
  gp_XYZ aMidPnt(0, 0, 0);
  for (i = 1; i <= aPnts.Length(); i++)
    aMidPnt += aPnts(i).XYZ();
  aMidPnt /= aPnts.Length();
  aTol = Min(aTol, theLinTol);
  for (i = 1; i <= aPnts.Length(); i++)
  {
    Standard_Real aDist = fabs((aPnts(i).XYZ() - aMidPnt) * aCylDir.XYZ());
    if (aDist > aTol)
      break;
  }
  Standard_Boolean isCircle = (i > aPnts.Length());

  // Create new edge
  TopoDS_Edge aNewEdge;
  if (isCircle)
  {
    // Create circular edge.
    // Find center by projecting aMidPnt to the cylinder axis
    gp_XYZ aVecOM = aMidPnt - aCylLoc.XYZ();
    gp_Pnt aCenter = aCylLoc.Translated(aCylDir.XYZ() * (aVecOM * aCylDir.XYZ()));
    gp_XYZ aXDir = aPnts(1).XYZ() - aCenter.XYZ();
    gp_XYZ aCircDir = aCylDir.XYZ();
    gp_XYZ aTanDir = aPnts(2).XYZ() - aPnts(1).XYZ();
    if ((aXDir ^ aTanDir)* aCylDir.XYZ() < 0.)
      aCircDir.Reverse();
    gp_Circ aCirc(gp_Ax2(aCenter, aCircDir, aXDir), aRadius);
    Handle(Geom_Circle) aCurve = new Geom_Circle(aCirc);
    // update vertices tolerance
    Standard_Real aPar;
    updateVertexOnCirc(aVF, aCirc, aPar);
    if (isClosed)
    {
      // full circle
      aNewEdge = BRepBuilderAPI_MakeEdge(aCurve, aVF, aVL, 0, M_PI*2.);
    }
    else
    {
      // an arc
      updateVertexOnCirc(aVL, aCirc, aPar);
      aNewEdge = BRepBuilderAPI_MakeEdge(aCurve, aVF, aVL, 0., aPar);
    }
  }
  else
  {
    gp_Pnt aPnts1[3];
    const gp_Pnt* pPnts = &aPnts(1);
    Standard_Integer np = aPnts.Length();
    if (np == 2)
    {
      gp_XYZ anEdgeDir = aPnts(2).XYZ() - aPnts(1).XYZ();
      const Standard_Real anEpsAng = 1e-4;
      if (aCylDir.IsParallel(anEdgeDir, anEpsAng))
        return aNewEdge;
      // we have only two points; create additional point in the middle of the segment
      aPnts1[0] = aPnts(1);
      aPnts1[2] = aPnts(2);
      aPnts1[1] = (aPnts1[0].XYZ() + aPnts1[2].XYZ()) * 0.5;
      pPnts = aPnts1;
      np = 3;
    }
    TColgp_Array1OfPnt aPntA(*pPnts, 1, np);
    // Create b-spline curve
    Standard_Integer aDegMin = 3;
    Standard_Integer aDegMax = 3;
    GeomAbs_Shape aCont = GeomAbs_C2;
    GeomAPI_PointsToBSpline aCreator(aPntA, aDegMin, aDegMax, aCont, aTol);
    if (aCreator.IsDone())
    {
      const Handle(Geom_BSplineCurve)& aCurve = aCreator.Curve();
      Standard_Real aTol = BRep_Tool::Tolerance(aVF);
      Standard_Real aParF = aCurve->FirstParameter();
      Standard_Real aParL = aCurve->LastParameter();
      Standard_Real aDist = aCurve->Value(aParF).SquareDistance(aPntA(1));
      if (aDist > aTol*aTol)
        BRep_Builder().UpdateVertex(aVF, sqrt(aDist));
      if (!isClosed)
      {
        aTol = BRep_Tool::Tolerance(aVL);
        aDist = aCurve->Value(aParL).SquareDistance(aPntA(np));
        if (aDist > aTol*aTol)
          BRep_Builder().UpdateVertex(aVL, sqrt(aDist));
      }
      aNewEdge = BRepBuilderAPI_MakeEdge(aCurve, aVF, aVL, aParF, aParL);
    }
  }
  return aNewEdge;
}

//=======================================================================
//function : mergeEdges
//purpose  : 
//=======================================================================
static void mergeEdges(const TopoDS_Shape& theShape,
                       const Standard_Real theLinTol,
                       const Standard_Real theAngTol,
                       const Handle(BRepTools_ReShape)& theContext)
{
  // Create topological data maps
  TopTools_IndexedDataMapOfShapeListOfShape aMapVE, aMapEF;
  TopExp::MapShapesAndAncestors(theShape, TopAbs_VERTEX, TopAbs_EDGE, aMapVE);
  TopExp::MapShapesAndAncestors(theShape, TopAbs_EDGE, TopAbs_FACE, aMapEF);

  // The idea is to gather chains of edges in which all intermediate vertices
  // have only two neighboring edges, and do not have connection to any other edges
  // from the shape.

  // Prepare set of all edges
  TopTools_IndexedMapOfShape anEdges;
  TopExp::MapShapes(theShape, TopAbs_EDGE, anEdges);
  TColStd_MapOfInteger anEdgesInd;  // set of indices of left edges
  Standard_Integer i;
  for (i = 1; i <= anEdges.Extent(); i++)
    anEdgesInd.Add(i);

  // Process while there are unprocessed edges
  while (!anEdgesInd.IsEmpty())
  {
    // Get an unprocessed edge
    Standard_Integer aEInd = TColStd_MapIteratorOfMapOfInteger(anEdgesInd).Value();
    anEdgesInd.Remove(aEInd);
    TopoDS_Edge aFirstEdge = TopoDS::Edge(anEdges(aEInd));
    BRepAdaptor_Curve aCurve(aFirstEdge);
    if (aCurve.GetType() != GeomAbs_Line)
      continue; // only linear edges are to be merged

    // Start a new chain
    TopTools_ListOfShape aChain;
    aChain.Append(aFirstEdge.Oriented(TopAbs_FORWARD));
    makeEdgesChain(aChain, aMapVE, theAngTol);

    // Remove it from the left edges
    TopTools_ListIteratorOfListOfShape itL(aChain);
    for (; itL.More(); itL.Next())
    {
      const TopoDS_Shape& aE = itL.Value();
      Standard_Integer aEI = anEdges.FindIndex(aE);
      anEdgesInd.Remove(aEI);
    }

    if (aChain.Extent() > 0)
    {
      // Create a new curvilinear edge from the polyline
      TopoDS_Edge aNewEdge = createCurveEdge(aChain, aMapEF, theLinTol);
      if (!aNewEdge.IsNull())
      {
        aNewEdge.Orientation(aFirstEdge.Orientation());
        // Replace edges from the chain with the new edge.
        Standard_Integer nbMerged = 0;
        for (itL.Init(aChain); itL.More(); itL.Next(), nbMerged++)
        {
          const TopoDS_Shape& aE = itL.Value();
          if (nbMerged == 0)
            theContext->Replace(aE, aNewEdge);
          else
            theContext->Remove(aE);
        }
      }
    }
  }
}

//=======================================================================
//function : Build
//purpose  : 
//=======================================================================
void ShapeUpgrade_CombineToCylinder::Build()
{
  myDone = Standard_False;
  myNbNewFaces = myNbReplacedFaces = 0;
  myShape.Nullify();

  TopTools_IndexedDataMapOfShapeListOfShape aMapEF;
  TopExp::MapShapesAndAncestors(myInitShape, TopAbs_EDGE, TopAbs_FACE, aMapEF);

  // Sort edges in the shape on two groups - stop edges and smooth edges that are 
  // to be located on a cylinder wall parallel to its axis.
  TopTools_MapOfShape aStopEdges;
  NCollection_DataMap<TopoDS_Shape, gp_Cylinder> aMapECyl;
  sortEdges(myInitShape, aMapEF, myLinTol, myAngTol, aStopEdges, aMapECyl);

#ifdef DEBUG_CombineToCylinder_StopEdges
  TopoDS_Compound aCompE;
  BRep_Builder().MakeCompound(aCompE);
  TopTools_MapIteratorOfMapOfShape aMapIt(aStopEdges);
  for (; aMapIt.More(); aMapIt.Next())
    BRep_Builder().Add(aCompE, aMapIt.Value());
  BRepTools::Write(aCompE, "stope.brep");
#endif

  // Collect faces forming same-cylinder areas
  NCollection_List<CombineToCylinder::Area> anAreas;
  collectAreas(myInitShape, aMapEF, aStopEdges, aMapECyl, myLinTol, myAngTol, anAreas);

#ifdef DEBUG_CombineToCylinder_Areas
  TopoDS_Compound aComp;
  BRep_Builder().MakeCompound(aComp);
  TopTools_ListIteratorOfListOfShape anItA(anAreas);
  for (; anItA.More(); anItA.Next())
    BRep_Builder().Add(aComp, anItA.Value());
  BRepTools::Write(aComp, "areas.brep");
#endif

  // Process each area
  NCollection_List<CombineToCylinder::Area>::Iterator anItAreas(anAreas);
  for (; anItAreas.More(); anItAreas.Next())
  {
    const CombineToCylinder::Area& anArea = anItAreas.Value();
    if (!anArea.pCylinder)
      continue;

    TopoDS_Shape aCylFace = createCylFaceFromArea(anArea.Faces, *anArea.pCylinder);

    // Replace faces from the list with the new face.
    TopExp_Explorer anEx(anArea.Faces, TopAbs_FACE);
    Standard_Integer nbMerged = 0;
    for (; anEx.More(); anEx.Next(), nbMerged++)
    {
      const TopoDS_Shape& aFace = anEx.Current();
      if (nbMerged == 0)
        myContext->Replace(aFace, aCylFace);
      else
        myContext->Remove(aFace);
    }
    myNbNewFaces++;
    myNbReplacedFaces += nbMerged;
  }

  // Build intermediate result shape using replacement context.
  // This shape will contain chains of linear edges along cylinders
  // instead of circles or whatever curves.
  // The next step is to merge them.
  TopoDS_Shape aTempShape = myContext->Apply(myInitShape);
  mergeEdges(aTempShape, myLinTol, myAngTol, myContext);

  // Rebuild the result shape using replacement context.
  myShape = myContext->Apply(myInitShape);

  // build pcurves
  ShapeFix_Shape aFix(myShape);
  aFix.Perform();
  myShape = aFix.Shape();
  myDone = Standard_True;
}
