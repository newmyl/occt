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

#include <BRepMesh.hxx>
#include <BRepMesh_Edge.hxx>
#include <BRepMesh_Triangle.hxx>
#include <BRepMesh_FastDiscret.hxx>
#include <BRepMesh_FastDiscretFace.hxx>
#include <BRepMesh_PluginMacro.hxx>

#include <Bnd_Box.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <BRepTools.hxx>
#include <BRepLib.hxx>
#include <BRepBndLib.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <Precision.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopTools_MutexForShapeProvider.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS.hxx>
#include <TopTools_HArray1OfShape.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_Polygon3D.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Standard_Mutex.hxx>
#include <BRepMesh_FaceChecker.hxx>

#ifdef HAVE_TBB
  // paralleling using Intel TBB
  #include <tbb/parallel_for_each.h>
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
//function : isCorrectPolyData
//purpose  : 
//=======================================================================
Standard_Boolean BRepMesh_IncrementalMesh::isCorrectPolyData()
{
  collectFaces();

  BRepMesh_FaceChecker aFaceChecker(myInParallel);
  if (myInParallel)
  {
  #ifdef HAVE_TBB
    // check faces in parallel threads using TBB
    tbb::parallel_for_each(myFaces.begin(), myFaces.end(), aFaceChecker);
  #else
    // alternative parallelization not yet available
    for (std::vector<TopoDS_Face>::iterator it(myFaces.begin()); it != myFaces.end(); it++)
      aFaceChecker(*it);
  #endif
  }
  else
  {
    for (std::vector<TopoDS_Face>::iterator it(myFaces.begin()); it != myFaces.end(); it++)
      aFaceChecker(*it);
  }

  return aFaceChecker.IsValid();
}

//=======================================================================
//function : BRepMesh_IncrementalMesh
//purpose  : 
//=======================================================================
BRepMesh_IncrementalMesh::BRepMesh_IncrementalMesh() 
: myRelative (Standard_False),
  myInParallel (Standard_False),
  myProgress (0)
{
  Init();
}

//=======================================================================
//function : BRepMesh_IncrementalMesh
//purpose  : 
//=======================================================================
BRepMesh_IncrementalMesh::BRepMesh_IncrementalMesh (const TopoDS_Shape& theShape,
                                                    const Standard_Real theDeflection,
                                                    const Standard_Boolean theRelative,
                                                    const Standard_Real theAngle,
                                                    const Standard_Boolean theInParallel,
                                                    const Handle_Message_ProgressIndicator& theProgress)
: myRelative (theRelative),
  myInParallel (theInParallel),
  myProgress (theProgress)
{
  Init();
  myDeflection = theDeflection;
  myAngle = theAngle;
  myShape = theShape;

  //
  Perform();
}

//=======================================================================
//function : ~
//purpose  : 
//=======================================================================
BRepMesh_IncrementalMesh::~BRepMesh_IncrementalMesh() 
{
}

//=======================================================================
//function : SetParallel
//purpose  :
//=======================================================================
void BRepMesh_IncrementalMesh::SetParallel (const Standard_Boolean theInParallel)
{
  myInParallel = theInParallel;
}

//=======================================================================
//function : IsParallel
//purpose  :
//=======================================================================
Standard_Boolean BRepMesh_IncrementalMesh::IsParallel() const
{
  return myInParallel;
}

//=======================================================================
//function : Init
//purpose  : 
//=======================================================================
void BRepMesh_IncrementalMesh::Init() 
{
  myStatus = 0;
  myModified = Standard_False;
  mymapedge.Clear();
  myancestors.Clear();
  myFaces.clear();
}

//=======================================================================
//function : SetRelative
//purpose  : 
//=======================================================================
void BRepMesh_IncrementalMesh::SetRelative(const Standard_Boolean theFlag)
{
  myRelative = theFlag;
}

//=======================================================================
//function : Relative
//purpose  : 
//=======================================================================
Standard_Boolean BRepMesh_IncrementalMesh::Relative()const
{
  return myRelative;
}

//=======================================================================
//function : IsModified
//purpose  : 
//=======================================================================
Standard_Boolean BRepMesh_IncrementalMesh::IsModified() const
{
  return myModified;
}

//=======================================================================
//function : Perform
//purpose  : 
//=======================================================================
void BRepMesh_IncrementalMesh::Perform()
{
  Init(); 

  if (!isCorrectPolyData())
    BRepTools::Clean(myShape);

  Bnd_Box aBox;
  //
  SetDone();
  //
  BRepBndLib::Add(myShape, aBox);
  myBox = aBox;
  //
  if (!myMesh.IsNull()) {
    myMesh.Nullify();
  }
  //
  myMesh = new BRepMesh_FastDiscret(myDeflection,
				    myAngle,
				    aBox,
				    Standard_True,
				    Standard_True,
				    myRelative,
				    Standard_True,
            myProgress);
  //
  Update(myShape);
}

//=======================================================================
//function : GetStatus
//purpose  : 
//=======================================================================
Standard_Integer BRepMesh_IncrementalMesh::GetStatusFlags() const
{
  return myStatus;
}

//=======================================================================
//function : collectFaces
//purpose  : 
//=======================================================================
void BRepMesh_IncrementalMesh::collectFaces()
{
  TopTools_ListOfShape aFaceList;
  BRepLib::ReverseSortFaces(myShape, aFaceList);
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
//function : Update(shape)
//purpose  : Builds the incremental mesh of the shape
//=======================================================================
void BRepMesh_IncrementalMesh::Update(const TopoDS_Shape& S)
{
  myModified = Standard_False;
  TopExp_Explorer ex;

  //AGV 080407: Since version 6.2.0 there would be exception without this check
  if (myBox.IsVoid())
    return;

  TopExp::MapShapesAndAncestors(myShape, TopAbs_EDGE, TopAbs_FACE, myancestors);
  
  BRepMesh_FastDiscret::BoxMaxDimension(myBox, mydtotale);
  
  for (ex.Init(S, TopAbs_EDGE); ex.More(); ex.Next()) {
    if(BRep_Tool::IsGeometric(TopoDS::Edge(ex.Current()))) {
      Update(TopoDS::Edge(ex.Current()));
    }
  }

  // get list of faces
  std::vector<TopoDS_Face>::iterator aFaceIt(myFaces.begin());
  for (; aFaceIt != myFaces.end(); aFaceIt++)
    Update(*aFaceIt);

  if (!myProgress.IsNull())
  {
    myProgress->SetScale(0., (Standard_Real)myFaces.size(), 1.);
  }

  if (myInParallel)
  {
  #ifdef HAVE_TBB
    myMesh->CreateMutexesForSubShapes(S, TopAbs_EDGE);
    // mesh faces in parallel threads using TBB
    tbb::parallel_for_each (myFaces.begin(), myFaces.end(), *myMesh.operator->());
  #else
    // alternative parallelization not yet available
    for (std::vector<TopoDS_Face>::iterator it(myFaces.begin()); it != myFaces.end(); it++)
    {
      myMesh->Process (*it);
    }
  #endif
    myMesh->RemoveAllMutexes();
  }
  else
  {
    for (std::vector<TopoDS_Face>::iterator it(myFaces.begin()); it != myFaces.end(); it++)
    {
      myMesh->Process (*it);
    }
  }

  if (!myProgress.IsNull() && myProgress->UserBreak())
    return;

  // maillage des edges non contenues dans les faces :
  Standard_Real f, l, defedge;
  Standard_Integer i, nbNodes;
  TopLoc_Location L;
  Standard_Real cdef = 1.;
  ex.Init(S ,TopAbs_EDGE, TopAbs_FACE);

  while (ex.More()) {
    const TopoDS_Edge& E = TopoDS::Edge(ex.Current());

    if(!BRep_Tool::IsGeometric(E)) {
      ex.Next();
      continue;
    }

    if (myRelative) 
      defedge = BRepMesh_FastDiscret::RelativeEdgeDeflection(E, myDeflection, 
                                                             mydtotale, cdef);
    else 
      defedge = myDeflection;
    
    Handle(Poly_Polygon3D) P3D = BRep_Tool::Polygon3D(E, L);
    Standard_Boolean maill = Standard_False;
    if (P3D.IsNull()) {
      maill = Standard_True;
    }
    else if (P3D->Deflection() > 1.1*defedge) {
      maill = Standard_True;
    }
    if (maill) {
      BRepAdaptor_Curve C(E);
      f = C.FirstParameter();
      l = C.LastParameter();
      
      GCPnts_TangentialDeflection TD(C, f, l, myAngle, defedge, 2);
      nbNodes = TD.NbPoints();
      
      TColgp_Array1OfPnt Nodes(1, nbNodes);
      TColStd_Array1OfReal UVNodes(1, nbNodes);
      for ( i = 1; i <= nbNodes; i++) {
        Nodes(i) = TD.Value(i);
        UVNodes(i) = TD.Parameter(i);
      }
      
      BRep_Builder B;
      Handle(Poly_Polygon3D) P = new Poly_Polygon3D(Nodes, UVNodes);
      P->Deflection(myDeflection);
      B.UpdateEdge(E, P);
    }

    ex.Next();
  }
}

//=======================================================================
//function : Update(edge)
//purpose  : Locate a correct discretisation if it exists
//           Set no one otherwise
//=======================================================================
void BRepMesh_IncrementalMesh::Update(const TopoDS_Edge& E)
{
  TopLoc_Location l;
  Standard_Integer i = 1;
  Handle(Poly_Triangulation) T, TNull;
  Handle(Poly_PolygonOnTriangulation) Poly, NullPoly;
  Standard_Boolean found = Standard_False;
  Standard_Real defedge = Precision::Confusion();
  Standard_Real cdef = 1.;
  BRep_Builder B;
  Standard_Boolean defined = Standard_False;
  
  do {
    BRep_Tool::PolygonOnTriangulation(E, Poly, T, l, i);
    i++;
    if (!T.IsNull() && !Poly.IsNull())
    {
      if (!defined)
      {
        if (myRelative) 
          defedge = BRepMesh_FastDiscret::RelativeEdgeDeflection(E, myDeflection, 
                                                                 mydtotale, cdef);
        else
          defedge = myDeflection;

        mymapedge.Bind(E, defedge);
        defined = Standard_True;
      }
      if (Poly->Deflection() <= 1.1 * defedge)
      {
        found = Standard_True;
      }
      else
      {
        myModified = Standard_True;
        B.UpdateEdge(E, NullPoly, T, l);
      }
    }
  } while (!Poly.IsNull());

  if (!found) myMap.Add(E);
}


//=======================================================================
//function : Update(face)
//purpose  : If the face is not correctly triangulated, or if one of its
//           edges is to be discretisated correctly, the triangulation
//           of this face is built.
//=======================================================================
void  BRepMesh_IncrementalMesh::Update(const TopoDS_Face& F)
{
  TopLoc_Location l;
  Handle(Geom_Surface) SS = BRep_Tool::Surface(F, l);
  if (SS.IsNull()) return;

  //Standard_Integer i;
  Standard_Boolean WillBeTriangulated = Standard_False;
  Handle(Poly_Triangulation) T, TNull;
  T = BRep_Tool::Triangulation(F, l);
  Handle(Poly_PolygonOnTriangulation) Poly, NullPoly;

  BRep_Builder B;
  TopExp_Explorer ex;
  
  Standard_Real defedge, defface, cdef = 1.;
  Standard_Integer nbEdge = 0;
  if (myRelative) {
    defface = 0.;
    
    for (ex.Init(F, TopAbs_EDGE); ex.More(); ex.Next()) {
      const TopoDS_Edge& edge = TopoDS::Edge(ex.Current());
      nbEdge++;
      if (mymapedge.IsBound(edge)) {
        defedge = mymapedge(edge);
      }
      else 
        defedge = BRepMesh_FastDiscret::RelativeEdgeDeflection(edge, myDeflection, mydtotale, cdef);
      defface = defface + defedge;
    }
    if (nbEdge != 0) defface = defface / nbEdge;
    else             defface = myDeflection;
  }
  else
    defface = myDeflection;

  if (!T.IsNull())
  {
    if (T->Deflection() <= 1.1 * defface)
    {
      for (ex.Init(F, TopAbs_EDGE); ex.More(); ex.Next())
      {
        const TopoDS_Shape& anEdge = ex.Current();
        Poly = BRep_Tool::PolygonOnTriangulation(TopoDS::Edge(anEdge), T, l);

        if (Poly.IsNull() || myMap.Contains(anEdge))
        {
          // Triangulation is built but edge hasn't representation on it.
          WillBeTriangulated = Standard_True;
          break;
        }
      }
    }
    else
    {
      WillBeTriangulated = Standard_True;
    }
  }

  if (WillBeTriangulated || T.IsNull()) {
    myModified = Standard_True;
    if (!T.IsNull()) {
      for (ex.Init(F, TopAbs_EDGE); ex.More(); ex.Next()) {
        B.UpdateEdge(TopoDS::Edge(ex.Current()), NullPoly, T, l);
        myMap.Remove(ex.Current());
      }
      B.UpdateFace(F, TNull);
    }
    myMesh->Add(F, myancestors);
    myStatus |= (Standard_Integer)(myMesh->CurrentFaceStatus());
    if (myMesh->CurrentFaceStatus() == BRepMesh_ReMesh) {
#ifdef DEB_MESH
      cout << " face remaillee + finement que prevu."<< endl;
#endif

      Standard_Integer index;
      
      TopTools_MapOfShape MShape;
      MShape.Add(F);

      TopoDS_Iterator ex(F),ex2;
      for (; ex.More(); ex.Next()) {
        const TopoDS_Shape& aWire = ex.Value();
        if (aWire.ShapeType() != TopAbs_WIRE)
          continue;
        TopoDS_Iterator exW(aWire);
        for(; exW.More(); exW.Next()) {
          const TopoDS_Edge& edge = TopoDS::Edge(exW.Value());
          index = myancestors.FindIndex(edge);
          if (index != 0) {
            const TopTools_ListOfShape& L = myancestors.FindFromKey(edge);

            TopTools_ListIteratorOfListOfShape it(L);

            for (; it.More(); it.Next()) {
              TopoDS_Face F2 = TopoDS::Face(it.Value());
              if (!MShape.Contains(F2)) {
                MShape.Add(F2);
                T = BRep_Tool::Triangulation(F2, l);	  
                if (!T.IsNull()) {
#ifdef DEB_MESH
                  cout <<"triangulation a refaire" <<endl;
#endif
                  for (ex2.Initialize(F2); ex2.More(); ex2.Next()) {
                    const TopoDS_Shape& aWire2 = ex2.Value();
                    if (aWire2.ShapeType() != TopAbs_WIRE)
                      continue;
                    TopoDS_Iterator exW2(aWire2);
                    for(; exW2.More(); exW2.Next()) {
                      TopoDS_Edge E2 = TopoDS::Edge(exW2.Value());
                      B.UpdateEdge(E2, NullPoly, T, l);
                    }
                  }
                  B.UpdateFace(F2, TNull);
                  myMesh->Add(F2, myancestors);
                }
              }
            }
          }
        }
      }
    }
  }
}


//=======================================================================
//function : Discret
//purpose  :
//=======================================================================
Standard_Integer BRepMesh_IncrementalMesh::Discret (const TopoDS_Shape&    theShape,
                                                    const Standard_Real    theDeflection,
                                                    const Standard_Real    theAngle,
                                                    BRepMesh_PDiscretRoot& theAlgo)
{
  BRepMesh_IncrementalMesh* anAlgo = new BRepMesh_IncrementalMesh();
  anAlgo->SetDeflection (theDeflection);
  anAlgo->SetAngle (theAngle);
  anAlgo->SetShape (theShape);
  anAlgo->SetParallel (IS_IN_PARALLEL);
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
void BRepMesh_IncrementalMesh::SetParallelDefault (const Standard_Boolean theInParallel)
{
  IS_IN_PARALLEL = theInParallel;
}

//! Export Mesh Plugin entry function
DISCRETPLUGIN(BRepMesh_IncrementalMesh)
