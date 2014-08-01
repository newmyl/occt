// Created on: 1995-06-20
// Created by: Stagiaire Alain JOURDAIN
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

#include <BRepMesh_IncrementalMesh.hxx>

#include <Precision.hxx>
#include <Standard_Mutex.hxx>

#include <BRepMesh_FaceChecker.hxx>
#include <BRepMesh_ShapeTool.hxx>
#include <BRepMesh_Edge.hxx>
#include <BRepMesh_PluginMacro.hxx>

#include <Bnd_Box.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepLib.hxx>
#include <BRepBndLib.hxx>
#include <BRepAdaptor_Curve.hxx>

#include <Poly_Triangulation.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>

#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopAbs.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>

#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_MutexForShapeProvider.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TopTools_HArray1OfShape.hxx>

#include <GCPnts_TangentialDeflection.hxx>
#include <Message_ProgressIndicator.hxx>
#include <Standard_ErrorHandler.hxx>

#ifdef HAVE_TBB
  // paralleling using Intel TBB
  #include <tbb/parallel_for_each.h>
  #include <tbb/tbb_exception.h>
#endif

namespace
{
  //! Default flag to control parallelization for BRepMesh_IncrementalMesh
  //! tool returned for Mesh Factory
  static Standard_Boolean IS_IN_PARALLEL = Standard_False;
};

IMPLEMENT_STANDARD_HANDLE (BRepMesh_IncrementalMesh, BRepMesh_DiscretRoot)
IMPLEMENT_STANDARD_RTTIEXT(BRepMesh_IncrementalMesh, BRepMesh_DiscretRoot)

//=======================================================================
//function : Default constructor
//purpose  : 
//=======================================================================
BRepMesh_IncrementalMesh::BRepMesh_IncrementalMesh()
  : myRelative  (Standard_False),
    myInParallel(Standard_False)
{
}

//=======================================================================
//function : Constructor
//purpose  : 
//=======================================================================
BRepMesh_IncrementalMesh::BRepMesh_IncrementalMesh(
  const TopoDS_Shape&                      theShape,
  const Standard_Real                      theLinDeflection,
  const Standard_Boolean                   isRelative,
  const Standard_Real                      theAngDeflection,
  const Standard_Boolean                   isInParallel,
  const Handle(Message_ProgressIndicator)& theProgress)
  : myRelative  (isRelative),
    myInParallel(isInParallel)
{
  SetShape            (theShape);
  SetDeflection       (theLinDeflection);
  SetAngle            (theAngDeflection);
  ProgressIndicator()->SetProgressIndicator(theProgress);

  Perform();
}

//=======================================================================
//function : Destructor
//purpose  : 
//=======================================================================
BRepMesh_IncrementalMesh::~BRepMesh_IncrementalMesh()
{
}

//=======================================================================
//function : init
//purpose  : 
//=======================================================================
void BRepMesh_IncrementalMesh::init() 
{
  myStatus   = 0;
  myModified = Standard_False;

  myEdgeDeflection.Clear();
  mySharedFaces.Clear();
  myFaces.clear();

  setDone();

  if (!isCorrectPolyData())
    BRepTools::Clean(Shape());

  Bnd_Box aBox;
  BRepBndLib::Add(Shape(), aBox, Standard_False);

  if (aBox.IsVoid())
  {
    // Nothing to mesh.
    myMesher.Nullify();
    return;
  }

  BRepMesh_ShapeTool::BoxMaxDimension(aBox, myMaxShapeSize);
  TopExp::MapShapesAndAncestors(Shape(), TopAbs_EDGE, TopAbs_FACE, mySharedFaces);

  myMesher = new BRepMesh_FastDiscret(Deflection(), Angle(), aBox,
    Standard_True, Standard_True, myRelative, Standard_True, myInParallel,
    ProgressIndicator());
}

//=======================================================================
//function : isCorrectPolyData
//purpose  : 
//=======================================================================
Standard_Boolean BRepMesh_IncrementalMesh::isCorrectPolyData()
{
  collectFaces();

  BRepMesh_FaceChecker aFaceChecker(myInParallel);

#ifdef HAVE_TBB
  if (myInParallel)
  {
    // check faces in parallel threads using TBB
    tbb::parallel_for_each(myFaces.begin(), myFaces.end(), aFaceChecker);
  }
  else
  {
#endif
    std::vector<TopoDS_Face>::iterator aFaceIt = myFaces.begin();
    for (; aFaceIt != myFaces.end(); aFaceIt++)
      aFaceChecker(*aFaceIt);
#ifdef HAVE_TBB
  }
#endif

  return aFaceChecker.IsValid();
}

//=======================================================================
//function : collectFaces
//purpose  : 
//=======================================================================
void BRepMesh_IncrementalMesh::collectFaces()
{
  TopTools_ListOfShape aFaceList;
  BRepLib::ReverseSortFaces(Shape(), aFaceList);
  TopTools_MapOfShape aFaceMap;
  myFaces.reserve(aFaceList.Extent());

  // make array of faces suitable for processing (excluding faces without surface)
  TopLoc_Location aDummyLoc;
  const TopLoc_Location aEmptyLoc;
  TopTools_ListIteratorOfListOfShape aFaceIter(aFaceList);
  for (; aFaceIter.More(); aFaceIter.Next())
  {
    TopoDS_Shape aFaceNoLoc = aFaceIter.Value();
    aFaceNoLoc.Location(aEmptyLoc);
    if (!aFaceMap.Add (aFaceNoLoc))
      continue; // already processed

    TopoDS_Face aFace = TopoDS::Face(aFaceIter.Value());
    const Handle(Geom_Surface)& aSurf = BRep_Tool::Surface(aFace, aDummyLoc);
    if (aSurf.IsNull())
      continue;

    myFaces.push_back(aFace);
  }
}

//=======================================================================
//function : Perform
//purpose  : 
//=======================================================================
void BRepMesh_IncrementalMesh::Perform()
{
  init();

  if (myMesher.IsNull())
    return;

  try
  {
    OCC_CATCH_SIGNALS

    update();
    return;
  }
  catch (const BRepMesh_UserBreak&)
  {
    myStatus |= BRepMesh_Break;
  }
  catch (...)
  {
    myStatus |= BRepMesh_Failure;
  }
}

//=======================================================================
//function : update()
//purpose  : 
//=======================================================================
void BRepMesh_IncrementalMesh::update()
{
  // Update edges data
  TopExp_Explorer aExplorer(Shape(), TopAbs_EDGE);
  for (; aExplorer.More(); aExplorer.Next())
  {
    const TopoDS_Edge& aEdge = TopoDS::Edge(aExplorer.Current());
    if(!BRep_Tool::IsGeometric(aEdge))
      continue;

    update(aEdge);
  }

  // Update faces data
  std::vector<TopoDS_Face>::iterator aFaceIt(myFaces.begin());
  for (; aFaceIt != myFaces.end(); aFaceIt++)
    update(*aFaceIt);

  ProgressIndicator()->SetScale(0., (Standard_Real)myFaces.size(), 1.);

  // Mesh faces
#ifdef HAVE_TBB
  if (myInParallel)
  {
    try
    {
      OCC_CATCH_SIGNALS

      myMesher->CreateMutexesForSubShapes(Shape(), TopAbs_EDGE);
      tbb::parallel_for_each(myFaces.begin(), myFaces.end(), *myMesher);
      myMesher->RemoveAllMutexes();
    }
    catch (BRepMesh_UserBreak)
    {
      myMesher->RemoveAllMutexes();
      Raise_BRepMesh_UserBreak();
    }
    catch (tbb::captured_exception)
    {
      myMesher->RemoveAllMutexes();
      Raise_BRepMesh_UserBreak();
    }
    catch (...)
    {
      myMesher->RemoveAllMutexes();
      Standard_Failure::Raise("Unknown exception");
    }
  }
  else
  {
#endif
    for (aFaceIt = myFaces.begin(); aFaceIt != myFaces.end(); aFaceIt++)
      myMesher->Process(*aFaceIt);
#ifdef HAVE_TBB
  }
#endif

  discretizeFreeEdges();
}

//=======================================================================
//function : discretizeFreeEdges
//purpose  : 
//=======================================================================
void BRepMesh_IncrementalMesh::discretizeFreeEdges()
{
  TopExp_Explorer aExplorer(Shape(), TopAbs_EDGE, TopAbs_FACE);
  for (; aExplorer.More(); aExplorer.Next())
  {
    const TopoDS_Edge& aEdge = TopoDS::Edge(aExplorer.Current());
    if(!BRep_Tool::IsGeometric(aEdge))
      continue;
    
    TopLoc_Location aLoc;
    Standard_Real aEdgeDeflection  = edgeDeflection(aEdge);
    Handle(Poly_Polygon3D) aPoly3D = BRep_Tool::Polygon3D(aEdge, aLoc);
    if (!aPoly3D.IsNull() && aPoly3D->Deflection() < 1.1 * aEdgeDeflection)
      continue;

    BRepAdaptor_Curve aCurve(aEdge);
    GCPnts_TangentialDeflection aDiscret(aCurve, aCurve.FirstParameter(),
      aCurve.LastParameter(), Angle(), aEdgeDeflection, 2);

    Standard_Integer aNodesNb = aDiscret.NbPoints();
    TColgp_Array1OfPnt   aNodes  (1, aNodesNb);
    TColStd_Array1OfReal aUVNodes(1, aNodesNb);
    for (Standard_Integer i = 1; i <= aNodesNb; ++i)
    {
      aNodes  (i) = aDiscret.Value(i);
      aUVNodes(i) = aDiscret.Parameter(i);
    }
    
    aPoly3D = new Poly_Polygon3D(aNodes, aUVNodes);
    aPoly3D->Deflection(Deflection());

    BRep_Builder aBuilder;
    aBuilder.UpdateEdge(aEdge, aPoly3D);
  }
}

//=======================================================================
//function : edgeDeflection
//purpose  : 
//=======================================================================
Standard_Real BRepMesh_IncrementalMesh::edgeDeflection(
  const TopoDS_Edge& theEdge)
{
  if (myEdgeDeflection.IsBound(theEdge))
    return myEdgeDeflection(theEdge);

  Standard_Real aEdgeDeflection;
  if (myRelative) 
  {
    Standard_Real aScale;
    aEdgeDeflection = BRepMesh_ShapeTool::RelativeEdgeDeflection(theEdge, 
      Deflection(), myMaxShapeSize, aScale);
  }
  else
    aEdgeDeflection = Deflection();

  myEdgeDeflection.Bind(theEdge, aEdgeDeflection);
  return aEdgeDeflection;
}

//=======================================================================
//function : faceDeflection
//purpose  : 
//=======================================================================
Standard_Real BRepMesh_IncrementalMesh::faceDeflection(
  const TopoDS_Face& theFace)
{
  if (!myRelative)
    return Deflection();

  Standard_Integer aEdgesNb        = 0;
  Standard_Real    aFaceDeflection = 0.;

  TopExp_Explorer aEdgeIt(theFace, TopAbs_EDGE);
  for (; aEdgeIt.More(); aEdgeIt.Next(), ++aEdgesNb)
  {
    const TopoDS_Edge& aEdge = TopoDS::Edge(aEdgeIt.Current());
    aFaceDeflection += edgeDeflection(aEdge);
  }

  return (aEdgesNb == 0) ? Deflection() : (aFaceDeflection / aEdgesNb);
}

//=======================================================================
//function : update(edge)
//purpose  : 
//=======================================================================
void BRepMesh_IncrementalMesh::update(const TopoDS_Edge& theEdge)
{
  ProgressIndicator()->UserBreak();

  Standard_Integer aPolyIndex   = 1;
  Standard_Real aEdgeDeflection = edgeDeflection(theEdge);
  Handle(Poly_PolygonOnTriangulation) aPolygon;
  do
  {
    TopLoc_Location aLoc;
    Handle(Poly_Triangulation) aTriangulation;
    BRep_Tool::PolygonOnTriangulation(theEdge, aPolygon, 
      aTriangulation, aLoc, aPolyIndex++);

    if (!aTriangulation.IsNull() && !aPolygon.IsNull())
    {
      if (aPolygon->Deflection() < 1.1 * aEdgeDeflection)
        continue;

      myModified = Standard_True;
      BRepMesh_ShapeTool::NullifyEdge(theEdge, aTriangulation, aLoc);
    }

    if (!myEmptyEdges.IsBound(theEdge))
      myEmptyEdges.Bind(theEdge, BRepMeshCol::MapOfTriangulation());

    myEmptyEdges(theEdge).Add(aTriangulation);
  }
  while (!aPolygon.IsNull());
}

//=======================================================================
//function : isToBeMeshed
//purpose  : 
//=======================================================================
Standard_Boolean BRepMesh_IncrementalMesh::toBeMeshed(
  const TopoDS_Face&     theFace,
  const Standard_Boolean isWithCheck)
{
  TopLoc_Location aLoc;
  Handle(Poly_Triangulation) aTriangulation = 
    BRep_Tool::Triangulation(theFace, aLoc);

  if (aTriangulation.IsNull())
    return Standard_True;

  if (isWithCheck)
  {
    Standard_Real aFaceDeflection = faceDeflection(theFace);
    if (aTriangulation->Deflection() < 1.1 * aFaceDeflection)
    {
      Standard_Boolean isEdgesConsistent = Standard_True;
      TopExp_Explorer aEdgeIt(theFace, TopAbs_EDGE);
      for (; aEdgeIt.More() && isEdgesConsistent; aEdgeIt.Next())
      {
        const TopoDS_Edge& aEdge = TopoDS::Edge(aEdgeIt.Current());
        if (!myEmptyEdges.IsBound(aEdge))
          continue;

        isEdgesConsistent &= myEmptyEdges(aEdge).Contains(aTriangulation);
      }

      if (isEdgesConsistent)
        return Standard_False;
    }
  }

  // Nullify edges
  TopExp_Explorer aEdgeIt(theFace, TopAbs_EDGE);
  for (; aEdgeIt.More(); aEdgeIt.Next())
  {
    const TopoDS_Edge& aEdge = TopoDS::Edge(aEdgeIt.Current());
    BRepMesh_ShapeTool::NullifyEdge(aEdge, aTriangulation, aLoc);
  }

  BRepMesh_ShapeTool::NullifyFace(theFace);
  return Standard_True;
}

//=======================================================================
//function : update(face)
//purpose  : 
//=======================================================================
void BRepMesh_IncrementalMesh::update(const TopoDS_Face& theFace)
{
  ProgressIndicator()->UserBreak();

  if (!toBeMeshed(theFace, Standard_True))
    return;

  myModified = Standard_True;
  myMesher->Add(theFace, mySharedFaces);

  BRepMesh_Status aStatus = myMesher->CurrentFaceStatus();
  myStatus |= (Standard_Integer)aStatus;
  if (aStatus != BRepMesh_ReMesh)
    return;

  BRepMeshCol::MapOfShape aUsedFaces;
  aUsedFaces.Add(theFace);

  TopExp_Explorer aEdgeIt(theFace, TopAbs_EDGE);
  for (; aEdgeIt.More(); aEdgeIt.Next())
  {
    const TopoDS_Edge& aEdge = TopoDS::Edge(aEdgeIt.Current());
    if (mySharedFaces.FindIndex(aEdge) == 0)
      continue;
     
    const TopTools_ListOfShape& aSharedFaces = mySharedFaces.FindFromKey(aEdge);
    TopTools_ListIteratorOfListOfShape aSharedFaceIt(aSharedFaces);
    for (; aSharedFaceIt.More(); aSharedFaceIt.Next())
    {
      const TopoDS_Face& aFace = TopoDS::Face(aSharedFaceIt.Value());
      if (aUsedFaces.Contains(aFace))
        continue;

      aUsedFaces.Add(aFace);
      toBeMeshed(aFace, Standard_False);

      myMesher->Add(aFace, mySharedFaces);
      myStatus |= (Standard_Integer)myMesher->CurrentFaceStatus();
    }
  }
}

//=======================================================================
//function : Discret
//purpose  :
//=======================================================================
Standard_Integer BRepMesh_IncrementalMesh::Discret(
  const TopoDS_Shape&    theShape,
  const Standard_Real    theDeflection,
  const Standard_Real    theAngle,
  BRepMesh_PDiscretRoot& theAlgo)
{
  BRepMesh_IncrementalMesh* anAlgo = new BRepMesh_IncrementalMesh();
  anAlgo->SetDeflection(theDeflection);
  anAlgo->SetAngle     (theAngle);
  anAlgo->SetShape     (theShape);
  anAlgo->SetParallel  (IS_IN_PARALLEL);
  theAlgo = anAlgo;
  return 0; // no error
}

//=======================================================================
//function : IsParallelDefault
//purpose  :
//=======================================================================
Standard_Boolean BRepMesh_IncrementalMesh::IsParallelDefault()
{
#ifdef HAVE_TBB
  return IS_IN_PARALLEL;
#else
  // no alternative parallelization yet - flag has no meaning
  return Standard_False;
#endif
}

//=======================================================================
//function : Discret
//purpose  :
//=======================================================================
void BRepMesh_IncrementalMesh::SetParallelDefault(
  const Standard_Boolean theInParallel)
{
  IS_IN_PARALLEL = theInParallel;
}

//! Export Mesh Plugin entry function
DISCRETPLUGIN(BRepMesh_IncrementalMesh)
