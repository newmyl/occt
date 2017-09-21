// Created on: 2006-11-04
// Created by: Alexander GRIGORIEV
// Copyright (c) 2006-2014 OPEN CASCADE SAS
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

#include <VrmlData_IndexedFaceSet.hxx>
#include <VrmlData_InBuffer.hxx>
#include <VrmlData_UnknownNode.hxx>
#include <Poly_Triangulation.hxx>
#include <BRep_TFace.hxx>
#include <VrmlData_Coordinate.hxx>
#include <VrmlData_Color.hxx>
#include <VrmlData_Normal.hxx>
#include <VrmlData_TextureCoordinate.hxx>
#include <VrmlData_Scene.hxx>
#include <Precision.hxx>
#include <NCollection_Vector.hxx>
#include <NCollection_DataMap.hxx>
#include <Poly.hxx>
#include <TShort_HArray1OfShortReal.hxx>

#include <TopoDS_Face.hxx>
#include <BRep_Tool.hxx>
#include <TopoDS_Vertex.hxx>
#include <TopoDS_Wire.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRepBuilderAPI_CellFilter.hxx>
//#include <StlMesh_Mesh.hxx>
//#include <StlMesh_MeshExplorer.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <StlAPI.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <ShapeUpgrade_CombineToCylinder.hxx>

IMPLEMENT_STANDARD_RTTIEXT(VrmlData_IndexedFaceSet,VrmlData_Faceted)

#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning (disable:4996)
#endif

//// Auxiliary tools
//namespace
//{
//  // Tool to get triangles from triangulation taking into account face
//  // orientation and location
//  class TriangleAccessor
//  {
//  public:
//    TriangleAccessor (const TopoDS_Face& aFace)
//    {
//      TopLoc_Location aLoc;
//      myPoly = BRep_Tool::Triangulation (aFace, aLoc);
//      myTrsf = aLoc.Transformation();
//      myNbTriangles = (myPoly.IsNull() ? 0 : myPoly->Triangles().Length());
//      myInvert = (aFace.Orientation() == TopAbs_REVERSED);
//      if (myTrsf.IsNegative())
//        myInvert = ! myInvert;
//    }
//
//    int NbTriangles () const { return myNbTriangles; } 
//
//    // get i-th triangle and outward normal
//    void GetTriangle (int iTri, gp_Vec &theNormal, gp_Pnt &thePnt1, gp_Pnt &thePnt2, gp_Pnt &thePnt3)
//    {
//      // get positions of nodes
//      int iNode1, iNode2, iNode3;
//      myPoly->Triangles()(iTri).Get (iNode1, iNode2, iNode3);
//      thePnt1 = myPoly->Nodes()(iNode1);
//      thePnt2 = myPoly->Nodes()(myInvert ? iNode3 : iNode2);
//      thePnt3 = myPoly->Nodes()(myInvert ? iNode2 : iNode3);
//
//      // apply transormation if not identity
//      if (myTrsf.Form() != gp_Identity)
//      {
//        thePnt1.Transform (myTrsf);
//        thePnt2.Transform (myTrsf);
//        thePnt3.Transform (myTrsf);
//      }
//
//      // calculate normal
//      theNormal = (thePnt2.XYZ() - thePnt1.XYZ()) ^ (thePnt3.XYZ() - thePnt1.XYZ());
//      Standard_Real aNorm = theNormal.Magnitude();
//      if (aNorm > gp::Resolution())
//        theNormal /= aNorm;
//    }
//
//  private:
//    Handle(Poly_Triangulation) myPoly;
//    gp_Trsf myTrsf;
//    int myNbTriangles;
//    bool myInvert;
//  };
//
//  // convert to float and, on big-endian platform, to little-endian representation
//  inline float convertFloat (Standard_Real aValue)
//  {
//#ifdef OCCT_BINARY_FILE_DO_INVERSE
//    return OSD_BinaryFile::InverseShortReal ((float)aValue);
//#else
//    return (float)aValue;
//#endif
//  }
//
//  // A static method adding nodes to a mesh and keeping coincident (sharing) nodes.
//  static Standard_Integer AddVertex(Handle(StlMesh_Mesh)& mesh,
//                                    BRepBuilderAPI_CellFilter& filter, 
//                                    BRepBuilderAPI_VertexInspector& inspector,
//                                    const gp_XYZ& p)
//  {
//    Standard_Integer index;
//    inspector.SetCurrent(p);
//    gp_XYZ minp = inspector.Shift(p, -Precision::Confusion());
//    gp_XYZ maxp = inspector.Shift(p, +Precision::Confusion());
//    filter.Inspect(minp, maxp, inspector);
//    else
//    const TColStd_ListOfInteger& indices = inspector.ResInd();
//    if (indices.IsEmpty() == Standard_False)
//    {
//      index = indices.First(); // it should be only one
//      inspector.ClearResList();
//    }
//    {
//      index = mesh->AddVertex(p.X(), p.Y(), p.Z());
//      filter.Add(index, p);
//      inspector.Add(p);
//    }
//    return index;
//  }
//
//  void createFromMesh(Handle(TopoDS_TShape)& theShapeWithMesh)
//  {
//    TopoDS_Shape aShape;
//    //aShape.Orientation(TopAbs_FORWARD);
//
//    aShape.TShape(theShapeWithMesh);
//    if (aShape.IsNull())
//      return;
//
//    // Write to STL and then read again to get BRep model (vrml fills only triangulation)
//    //StlAPI::Write(aShape, "D:/Temp/tempfile");
//    //StlAPI::Read(aShape, "D:/Temp/tempfile");
//
//    gp_XYZ p1, p2, p3;
//    TopoDS_Vertex Vertex1, Vertex2, Vertex3;
//    TopoDS_Face AktFace;
//    TopoDS_Wire AktWire;
//    BRepBuilderAPI_Sewing aSewingTool;
//    Standard_Real x1, y1, z1;
//    Standard_Real x2, y2, z2;
//    Standard_Real x3, y3, z3;
//    Standard_Integer i1,i2,i3;
//  
//    aSewingTool.Init(1.0e-06,Standard_True);
//  
//    TopoDS_Compound aComp;
//    BRep_Builder BuildTool;
//    BuildTool.MakeCompound( aComp );
//
//    Handle(StlMesh_Mesh) aSTLMesh = new StlMesh_Mesh();
//    aSTLMesh->AddDomain();
//
//    // Filter unique vertices to share the nodes of the mesh.
//    BRepBuilderAPI_CellFilter uniqueVertices(Precision::Confusion());
//    BRepBuilderAPI_VertexInspector inspector(Precision::Confusion());
//
//    // Read mesh
//    for (TopExp_Explorer exp (aShape, TopAbs_FACE); exp.More(); exp.Next())
//    {
//      TriangleAccessor aTool (TopoDS::Face (exp.Current()));
//      for (int iTri = 1; iTri <= aTool.NbTriangles(); iTri++)
//      {
//        gp_Vec aNorm;
//        gp_Pnt aPnt1, aPnt2, aPnt3;
//        aTool.GetTriangle(iTri, aNorm, aPnt1, aPnt2, aPnt3);
//
//        i1 = AddVertex(aSTLMesh, uniqueVertices, inspector, aPnt1.XYZ());
//        i2 = AddVertex(aSTLMesh, uniqueVertices, inspector, aPnt2.XYZ());
//        i3 = AddVertex(aSTLMesh, uniqueVertices, inspector, aPnt3.XYZ());
//        aSTLMesh->AddTriangle(i1, i2, i3, aNorm.X(), aNorm.Y(), aNorm.Z());
//      }
//    }
//
//    StlMesh_MeshExplorer aMExp (aSTLMesh);
//    Standard_Integer NumberDomains = aSTLMesh->NbDomains();
//    Standard_Integer iND;
//    for (iND=1;iND<=NumberDomains;iND++) 
//    {
//      for (aMExp.InitTriangle (iND); aMExp.MoreTriangle (); aMExp.NextTriangle ()) 
//      {
//        aMExp.TriangleVertices (x1,y1,z1,x2,y2,z2,x3,y3,z3);
//        p1.SetCoord(x1,y1,z1);
//        p2.SetCoord(x2,y2,z2);
//        p3.SetCoord(x3,y3,z3);
//      
//        if ((!(p1.IsEqual(p2,0.0))) && (!(p1.IsEqual(p3,0.0))))
//        {
//          Vertex1 = BRepBuilderAPI_MakeVertex(p1);
//          Vertex2 = BRepBuilderAPI_MakeVertex(p2);
//          Vertex3 = BRepBuilderAPI_MakeVertex(p3);
//        
//          AktWire = BRepBuilderAPI_MakePolygon( Vertex1, Vertex2, Vertex3, Standard_True);
//        
//          if( !AktWire.IsNull())
//          {
//            AktFace = BRepBuilderAPI_MakeFace( AktWire);
//            if(!AktFace.IsNull())
//              BuildTool.Add( aComp, AktFace );
//          }
//        }
//      }
//    }
//    aSTLMesh->Clear();
//
//    aSewingTool.Init();
//    aSewingTool.Load( aComp );
//    aSewingTool.Perform();
//    aShape = aSewingTool.SewedShape();
//    if ( aShape.IsNull() )
//      aShape = aComp;
//
//    ShapeUpgrade_UnifySameDomain anUSD(aShape);
//    anUSD.SetLinearTolerance(1e-5);
//    anUSD.Build();
//    aShape = anUSD.Shape();
//
//    if (aShape.ShapeType() == TopAbs_SHELL && TopoDS::Shell(aShape).Closed())
//    {
//      TopoDS_Solid aSolid;
//      TopoDS_Shell aShell = TopoDS::Shell (aShape);
//      if (!aShell.Free ()) {
//        aShell.Free(Standard_True);
//      }
//      BRep_Builder aBuilder;
//      aBuilder.MakeSolid (aSolid);
//      aBuilder.Add (aSolid, aShell);
//
//      Standard_Boolean isOk = Standard_True;
//      try {
//        OCC_CATCH_SIGNALS
//        BRepClass3d_SolidClassifier bsc3d (aSolid);
//        Standard_Real t = Precision::Confusion();
//        bsc3d.PerformInfinitePoint(t);
//
//        if (bsc3d.State() == TopAbs_IN) {
//          TopoDS_Solid aSolid2;
//          aBuilder.MakeSolid (aSolid2);
//          aShell.Reverse();
//          aBuilder.Add (aSolid2, aShell);
//          aSolid = aSolid2;
//        }
//      }
//      catch (Standard_Failure) { isOk = Standard_False; }
//      if (isOk) aShape = aSolid;
//    }
//
//    // Trying to apply "Combine to Cylinder"
//
//    //ShapeUpgrade_CombineToCylinder cmb2Cyl;
//    //Standard_Boolean isOk = Standard_True;
//    //try {
//    //  OCC_CATCH_SIGNALS
//    //  cmb2Cyl.SetShape(aShape);
//    //  cmb2Cyl.SetAngularTolerance(20);
//    //  cmb2Cyl.SetLinearTolerance(3);
//    //  cmb2Cyl.Build();
//    //}
//    //catch (Standard_Failure) { isOk = Standard_False; }
//    //if (isOk && cmb2Cyl.IsDone())
//    //  aShape = cmb2Cyl.Shape();
//
//    theShapeWithMesh = aShape.TShape();
//  }
//}
//
//=======================================================================
//function : readData
//purpose  : 
//=======================================================================

VrmlData_ErrorStatus VrmlData_Faceted::readData (VrmlData_InBuffer& theBuffer)
{
  VrmlData_ErrorStatus aStatus (VrmlData_EmptyData);
  Standard_Boolean aBool;
  if        (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "ccw")) {
    if (OK(aStatus, ReadBoolean (theBuffer, aBool)))
      myIsCCW = aBool;
  } else if (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "convex")) {
    if (OK(aStatus, ReadBoolean (theBuffer, aBool)))
      myIsConvex = aBool;
  } else if (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "solid")) {
    if (OK(aStatus, ReadBoolean (theBuffer, aBool)))
      myIsSolid = aBool;
  } else if (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "creaseAngle")) {
    Standard_Real anAngle;
    if (OK(aStatus, Scene().ReadReal (theBuffer, anAngle,
                                      Standard_False, Standard_False))) {
      if (anAngle < -Precision::Confusion()*0.001)
        aStatus = VrmlData_IrrelevantNumber;
      else
        myCreaseAngle = anAngle;
    }
  }
  return aStatus;
}

//=======================================================================
//function : VrmlData_IndexedFaceSet::TShape
//purpose  : 
//=======================================================================

const Handle(TopoDS_TShape)& VrmlData_IndexedFaceSet::TShape ()
{
  if (myNbPolygons == 0)
    myTShape.Nullify();
  else if (myIsModified) {
    // Create an empty topological Face
    const gp_XYZ * arrNodes = myCoords->Values();
    Standard_Integer i, nTri(0);

    NCollection_DataMap <int, int> mapNodeId;

    // Count non-degenerated triangles
    const int nNodes = (int)myCoords->Length();
    for (i = 0; i < (int)myNbPolygons; i++) {
      const Standard_Integer * arrIndice;
      if (Polygon(i, arrIndice) == 3) {
        //Check indices for out of bound
        if (arrIndice[0] < 0 ||
            arrIndice[0] >= nNodes ||
            arrIndice[1] >= nNodes ||
            arrIndice[2] >= nNodes)
        {
            continue;
        }
        const gp_XYZ aVec[2] = {
          arrNodes[arrIndice[1]] - arrNodes[arrIndice[0]],
          arrNodes[arrIndice[2]] - arrNodes[arrIndice[0]]
        };
        if ((aVec[0] ^ aVec[1]).SquareModulus() >
            Precision::SquareConfusion())
          ++nTri;
        else {
          const_cast<Standard_Integer&> (arrIndice[0]) = -1;
          continue;
        }
      }
      if (mapNodeId.IsBound (arrIndice[0]) == Standard_False)
        mapNodeId.Bind (arrIndice[0], 0);
      if (mapNodeId.IsBound (arrIndice[1]) == Standard_False)
        mapNodeId.Bind (arrIndice[1], 0);
      if (mapNodeId.IsBound (arrIndice[2]) == Standard_False)
        mapNodeId.Bind (arrIndice[2], 0);
    }
    const Standard_Integer nbNodes (mapNodeId.Extent());
    if (!nbNodes)
    {
        myIsModified = Standard_False;
        myTShape.Nullify();
        return myTShape;
    }

    Handle(Poly_Triangulation) aTriangulation =
      new Poly_Triangulation (nbNodes, nTri, Standard_False);
    Handle(BRep_TFace) aFace = new BRep_TFace();
    aFace->Triangulation (aTriangulation);
    myTShape = aFace;

    // Copy the triangulation vertices
    TColgp_Array1OfPnt& aNodes = aTriangulation->ChangeNodes();
    NCollection_DataMap <int, int>::Iterator anIterN(mapNodeId);
    for (i = 1; anIterN.More(); anIterN.Next()) {
      const int aKey = anIterN.Key();
      const gp_XYZ& aNodePnt = arrNodes[aKey];
      aNodes(i) = gp_Pnt (aNodePnt);
      anIterN.ChangeValue() = i++;
    }

    // Copy the triangles. Only the triangle-type polygons are supported.
    // In this loop we also get rid of any possible degenerated triangles.
    Poly_Array1OfTriangle& aTriangles = aTriangulation->ChangeTriangles();
    nTri = 0;
    for (i = 0; i < (int)myNbPolygons; i++) {
      const Standard_Integer * arrIndice;
      if (Polygon (i, arrIndice) == 3)
        if (arrIndice[0] >= 0 &&
            arrIndice[0] < nNodes &&
            arrIndice[1] < nNodes &&
            arrIndice[2] < nNodes)  // check to avoid previously skipped faces
          aTriangles(++nTri).Set (mapNodeId(arrIndice[0]),
                                  mapNodeId(arrIndice[1]),
                                  mapNodeId(arrIndice[2]));
    }

    // Normals should be defined; if they are not, compute them
    if (myNormals.IsNull ()) {
      //aTriangulation->ComputeNormals();
      Poly::ComputeNormals(aTriangulation);
    }
    else {
      // Copy the normals. Currently only normals-per-vertex are supported.
      Handle(TShort_HArray1OfShortReal) Normals =
        new TShort_HArray1OfShortReal (1, 3*nbNodes);
      if (myNormalPerVertex) {
        if (myArrNormalInd == 0L) {
          NCollection_DataMap <int, int>::Iterator anIterNN (mapNodeId);
          for (; anIterNN.More (); anIterNN.Next ()) {
            Standard_Integer anIdx = (anIterNN.Value() - 1) * 3 + 1;
            const gp_XYZ& aNormal = myNormals->Normal (anIterNN.Key ());
            Normals->SetValue (anIdx + 0, Standard_ShortReal (aNormal.X ()));
            Normals->SetValue (anIdx + 1, Standard_ShortReal (aNormal.Y ()));
            Normals->SetValue (anIdx + 2, Standard_ShortReal (aNormal.Z ()));
          }
        }
        else
        {
          for (i = 0; i < (int)myNbPolygons; i++) 
          {
            const Standard_Integer * anArrNodes;
            if (Polygon(i, anArrNodes) == 3 &&
              anArrNodes[0] >= 0 &&
              anArrNodes[0] < nNodes &&
              anArrNodes[1] < nNodes &&
              anArrNodes[2] < nNodes)  // check to avoid previously skipped faces
            {
              const Standard_Integer * arrIndice;
              if (IndiceNormals(i, arrIndice) == 3) {
                for (Standard_Integer j = 0; j < 3; j++) {
                  const gp_XYZ& aNormal = myNormals->Normal (arrIndice[j]);
                  Standard_Integer anInd = (mapNodeId(anArrNodes[j]) - 1) * 3 + 1;
                  Normals->SetValue (anInd + 0, Standard_ShortReal (aNormal.X()));
                  Normals->SetValue (anInd + 1, Standard_ShortReal (aNormal.Y()));
                  Normals->SetValue (anInd + 2, Standard_ShortReal (aNormal.Z()));
                }
              }
            }
          }
        }
      } else {
        //TODO ..
      }
      aTriangulation->SetNormals(Normals);
    }

    myIsModified = Standard_False;
  }
  //createFromMesh(myTShape);
  return myTShape;
}

//=======================================================================
//function : VrmlData_IndexedFaceSet::Clone
//purpose  : 
//=======================================================================

Handle(VrmlData_Node) VrmlData_IndexedFaceSet::Clone
                                (const Handle(VrmlData_Node)& theOther) const
{
  Handle(VrmlData_IndexedFaceSet) aResult =
    Handle(VrmlData_IndexedFaceSet)::DownCast (VrmlData_Node::Clone(theOther));
  if (aResult.IsNull())
    aResult =
      new VrmlData_IndexedFaceSet(theOther.IsNull()? Scene(): theOther->Scene(),
                                  Name());

  if (&aResult->Scene() == &Scene()) {
    aResult->SetCoordinates     (myCoords);
    aResult->SetNormals         (myNormals);
    aResult->SetColors          (myColors);
    aResult->SetPolygons        (myNbPolygons, myArrPolygons);
    aResult->SetNormalInd       (myNbNormals, myArrNormalInd);
    aResult->SetColorInd        (myNbColors, myArrColorInd);
    aResult->SetTextureCoordInd (myNbTextures, myArrTextureInd);
  } else {
    // Create a dummy node to pass the different Scene instance to methods Clone
    const Handle(VrmlData_UnknownNode) aDummyNode =
      new VrmlData_UnknownNode (aResult->Scene());
    if (myCoords.IsNull() == Standard_False)
      aResult->SetCoordinates (Handle(VrmlData_Coordinate)::DownCast
                               (myCoords->Clone (aDummyNode)));
    if (myNormals.IsNull() == Standard_False)
      aResult->SetNormals (Handle(VrmlData_Normal)::DownCast
                           (myNormals->Clone (aDummyNode)));
    if (myColors.IsNull() == Standard_False)
      aResult->SetColors (Handle(VrmlData_Color)::DownCast
                          (myColors->Clone (aDummyNode)));
    //TODO: Replace the following lines with the relevant copying
    aResult->SetPolygons        (myNbPolygons, myArrPolygons);
    aResult->SetNormalInd       (myNbNormals, myArrNormalInd);
    aResult->SetColorInd        (myNbColors, myArrColorInd);
    aResult->SetTextureCoordInd (myNbTextures, myArrTextureInd);
  }
  aResult->SetNormalPerVertex (myNormalPerVertex);
  aResult->SetColorPerVertex  (myColorPerVertex);
  return aResult;
}

//=======================================================================
//function : VrmlData_IndexedFaceSet::Read
//purpose  : 
//=======================================================================

VrmlData_ErrorStatus VrmlData_IndexedFaceSet::Read(VrmlData_InBuffer& theBuffer)
{
  VrmlData_ErrorStatus aStatus;
  const VrmlData_Scene& aScene = Scene();
  while (OK(aStatus, VrmlData_Scene::ReadLine(theBuffer)))
  {
    if (OK(aStatus, VrmlData_Faceted::readData (theBuffer)))
      continue;
    if (aStatus != VrmlData_EmptyData)
      break;
    else if (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "colorPerVertex"))
      aStatus = ReadBoolean (theBuffer, myColorPerVertex);
    else if (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "normalPerVertex"))
      aStatus = ReadBoolean (theBuffer, myNormalPerVertex);
    else if (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "coordIndex"))
      aStatus = aScene.ReadArrIndex (theBuffer, myArrPolygons, myNbPolygons);
    else if (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "colorIndex"))
      aStatus = aScene.ReadArrIndex (theBuffer, myArrColorInd, myNbColors);
    else if (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "normalIndex"))
      aStatus = aScene.ReadArrIndex (theBuffer, myArrNormalInd, myNbNormals);
    else if (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "texCoordIndex"))
      aStatus = aScene.ReadArrIndex (theBuffer, myArrTextureInd, myNbTextures);
    // These four checks should be the last one to avoid their interference
    // with the other tokens (e.g., coordIndex)
    else if (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "texCoord"))
    {
      Handle(VrmlData_Node) aNode;
      aStatus = ReadNode (theBuffer, aNode,
                          STANDARD_TYPE(VrmlData_TextureCoordinate));
      myTxCoords = Handle(VrmlData_TextureCoordinate)::DownCast (aNode);
    }
    else if (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "color"))
    {
      Handle(VrmlData_Node) aNode;
      aStatus = ReadNode (theBuffer, aNode,
                          STANDARD_TYPE(VrmlData_Color));
      myColors = Handle(VrmlData_Color)::DownCast (aNode);
    }
    else if (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "coord"))
    {
      Handle(VrmlData_Node) aNode;
      aStatus = ReadNode (theBuffer, aNode,
                          STANDARD_TYPE(VrmlData_Coordinate));
      myCoords = Handle(VrmlData_Coordinate)::DownCast (aNode);
    }
    else if (VRMLDATA_LCOMPARE (theBuffer.LinePtr, "normal"))
    {
      Handle(VrmlData_Node) aNode;
      aStatus = ReadNode (theBuffer, aNode,
                          STANDARD_TYPE(VrmlData_Normal));
      myNormals = Handle(VrmlData_Normal)::DownCast (aNode);
    }
    if (!OK(aStatus))
      break;
  }
  // Read the terminating (closing) brace
  if (OK(aStatus) || aStatus == VrmlData_EmptyData)
    if (OK(aStatus, readBrace (theBuffer))) {
      // Post-processing
      ;
    }
  return aStatus;
}

// //=======================================================================
// //function : dummyReadBrackets
// //purpose  : static (local) function
// //=======================================================================

// VrmlData_ErrorStatus dummyReadBrackets (VrmlData_InBuffer& theBuffer)
// {
//   VrmlData_ErrorStatus aStatus;
//   Standard_Integer aLevelCounter (0);
//   // This loop searches for any opening bracket.
//   // Such bracket increments the level counter. A closing bracket decrements
//   // the counter. The loop terminates when the counter becomes zero.
//   while ((aStatus = VrmlData_Scene::ReadLine(theBuffer)) == VrmlData_StatusOK)
//   {
//     int aChar;
//     while ((aChar = theBuffer.LinePtr[0]) != '\0') {
//       theBuffer.LinePtr++;
//       if        (aChar == '[') {
//         aLevelCounter++;
//         break;
//       } else if (aChar == ']') {
//         aLevelCounter--;
//         break;
//       }
//     }
//     if (aLevelCounter <= 0)
//       break;
//   }
//   return aStatus;
// }

//=======================================================================
//function : IsDefault
//purpose  : 
//=======================================================================

Standard_Boolean VrmlData_IndexedFaceSet::IsDefault () const
{
  Standard_Boolean aResult (Standard_True);
  if (myNbPolygons)
    aResult = Standard_False;
  else if (myCoords.IsNull() == Standard_False)
    aResult = myCoords->IsDefault();
  return aResult;
}

//=======================================================================
//function : Write
//purpose  : 
//=======================================================================

VrmlData_ErrorStatus VrmlData_IndexedFaceSet::Write
                                                (const char * thePrefix) const
{
  static char header[] = "IndexedFaceSet {";
  const VrmlData_Scene& aScene = Scene();
  VrmlData_ErrorStatus aStatus;
  if (OK (aStatus, aScene.WriteLine (thePrefix, header, GlobalIndent()))) {

    // Write the attributes of interface "VrmlData_Faceted"
    if (IsCCW() == Standard_False)
      aStatus = aScene.WriteLine ("ccw         FALSE");
    if (OK(aStatus) && IsSolid() == Standard_False)
      aStatus = aScene.WriteLine ("solid       FALSE");
    if (OK(aStatus) && IsConvex() == Standard_False)
      aStatus = aScene.WriteLine ("convex      FALSE");
    if (OK(aStatus) && CreaseAngle() > Precision::Confusion()) {
      char buf[64];
      Sprintf (buf, "%.9g", CreaseAngle());
      aStatus = aScene.WriteLine ("creaseAngle", buf);
    }

    if (OK(aStatus) && myCoords.IsNull() == Standard_False)
      aStatus = aScene.WriteNode ("coord", myCoords);
    if (OK(aStatus))
      aStatus = aScene.WriteArrIndex ("coordIndex", myArrPolygons,myNbPolygons);

    if (OK(aStatus) && myNormalPerVertex == Standard_False)
      aStatus = aScene.WriteLine ("normalPerVertex FALSE");
    if (OK(aStatus) && myNormals.IsNull() == Standard_False)
      aStatus = aScene.WriteNode ("normal", myNormals);
    if (OK(aStatus))
      aStatus = aScene.WriteArrIndex ("normalIndex",myArrNormalInd,myNbNormals);

    if (OK(aStatus) && myColorPerVertex == Standard_False)
      aStatus = aScene.WriteLine ("colorPerVertex  FALSE");
    if (OK(aStatus) && myColors.IsNull() == Standard_False)
      aStatus = aScene.WriteNode ("color", myColors);
    if (OK(aStatus))
      aStatus = aScene.WriteArrIndex ("colorIndex", myArrColorInd, myNbColors);

    if (OK(aStatus) && myTxCoords.IsNull() == Standard_False)
      aStatus = aScene.WriteNode ("texCoord", myTxCoords);
    if (OK(aStatus))
      aStatus = aScene.WriteArrIndex ("texCoordIndex", myArrTextureInd,
                                      myNbTextures);

    aStatus = WriteClosing();
  }
  return aStatus;
}

//=======================================================================
//function : GetColor
//purpose  : 
//=======================================================================

Quantity_Color VrmlData_IndexedFaceSet::GetColor
                                        (const Standard_Integer /*iFace*/,
                                         const Standard_Integer /*iVertex*/)
{
  //TODO
  return Quantity_NOC_BLACK;
}

