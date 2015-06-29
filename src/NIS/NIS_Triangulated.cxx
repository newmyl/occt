// Created on: 2007-07-17
// Created by: Alexander GRIGORIEV
// Copyright (c) 2007-2014 OPEN CASCADE SAS
//
// This file is part of Open CASCADE Technology software library.
//
// This library is free software; you can redistribute it and / or modify it
// under the terms of the GNU Lesser General Public version 2.1 as published
// by the Free Software Foundation, with special exception defined in the file
// OCCT_LGPL_EXCEPTION.txt. Consult the file LICENSE_LGPL_21.txt included in OCCT
// distribution for complete text of the license and disclaimer of any warranty.
//
// Alternatively, this file may be used under the terms of Open CASCADE
// commercial license or contractual agreement.

#include <NIS_Triangulated.hxx>
#include <NIS_TriangulatedDrawer.hxx>
#include <NIS_InteractiveContext.hxx>
#include <TopoDS.hxx>
#include <gp_Ax1.hxx>
#include <Precision.hxx>
#include <NCollection_DataMap.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <TopExp_Explorer.hxx>
#include <BRep_Tool.hxx>

IMPLEMENT_STANDARD_HANDLE  (NIS_Triangulated, NIS_InteractiveObject)
IMPLEMENT_STANDARD_RTTIEXT (NIS_Triangulated, NIS_InteractiveObject)

static Standard_Real aTolConf = Precision::Confusion() * 0.0001;

/**
 * Checking the given value if it is considerably greater than zero.
 */
static inline Standard_Boolean IsPositive (const Standard_Real theVal)
{
  return (theVal > aTolConf);
}

/**
 * Compute the size in bytes of an index array.
 */ 
static inline Standard_Size    NBytesInd  (const Standard_Integer nInd,
                                           const unsigned int     theIndType)
{
  Standard_Size nBytes = static_cast<Standard_Size>(nInd);
  if (theIndType) {
    nBytes *= 2;
    if (theIndType > 1u)
      nBytes *= 2;
  }
  return nBytes;
}

//=======================================================================
//function : IsEqual
//purpose  : 
//=======================================================================
inline Standard_Boolean IsEqual(const TopoDS_Shape& theS1,
                                const TopoDS_Shape& theS2)
{
  return theS1.IsEqual(theS2);
}

//=======================================================================
//function : NIS_Triangulated()
//purpose  : Constructor
//=======================================================================

NIS_Triangulated::NIS_Triangulated
                        (const Standard_Integer                   nNodes,
                         const Standard_Boolean                   is2D,
                         const Handle(NCollection_BaseAllocator)& theAlloc)
  : myAlloc             (0L),
    myType              (Type_None),
    mypNodes            (0L),
    mypTriangles        (0L),    
    myNNodes            (0),
    myNTriangles        (0),    
    myIsDrawPolygons    (Standard_False),
    myIsCloned          (Standard_False),
    myIndexType         (2u),
    myNodeCoord         (is2D ? 2 : 3),
    myPolygonType       (static_cast<unsigned int>(Polygon_Default)),
    myTriMode           (static_cast<unsigned int>(TriMode_Triangles))
{
  
  if (theAlloc.IsNull())
    myAlloc = NCollection_BaseAllocator::CommonBaseAllocator().operator->();
  else
    myAlloc = theAlloc.operator->();
  allocateNodes (nNodes);
}

//=======================================================================
//function : NIS_Triangulated()
//purpose  : Constructor
//=======================================================================

NIS_Triangulated::NIS_Triangulated
                        (const TopoDS_Shape&                      theShape, 
                         const Standard_Real                      theDeflection,
                         const Handle(NCollection_BaseAllocator)& theAlloc)
  : myAlloc             (0L),
    myType              (Type_None),
    mypNodes            (0L),
    mypTriangles        (0L),    
    myNNodes            (0),
    myNTriangles        (0),    
    myIsDrawPolygons    (Standard_False),
    myIsCloned          (Standard_False),
    myIndexType         (2u),
    myNodeCoord         (3),
    myPolygonType       (static_cast<unsigned int>(Polygon_Default)),
    myTriMode           (static_cast<unsigned int>(TriMode_Triangles))
{
  
  if (theAlloc.IsNull())
    myAlloc = NCollection_BaseAllocator::CommonBaseAllocator().operator->();
  else
    myAlloc = theAlloc.operator->();

  Init(theShape, theDeflection);
}

//=======================================================================
//function : Init
//purpose  : Initialize the instance with a TopoDS_Shape.
//=======================================================================

void NIS_Triangulated::Init (const TopoDS_Shape& theShape,
                             const Standard_Real theDeflection,
                             const Standard_Real theAngularDeflection)
{
  Clear();

  NCollection_DataMap<TopoDS_Shape, GCPnts_TangentialDeflection>
    mapEdges(100, myAlloc);
  TopExp_Explorer eexp (theShape, TopAbs_EDGE);
  Standard_Integer nbNodes = 0;
  for (; eexp.More(); eexp.Next())
  {
    const TopoDS_Edge& anEdge = TopoDS::Edge(eexp.Current());
    if (BRep_Tool::Degenerated(anEdge)) {
      continue;
    }

    // perform discretization
    BRepAdaptor_Curve anAdapCurve(anEdge);
    GCPnts_TangentialDeflection aTD(anAdapCurve, theAngularDeflection, theDeflection);
    Standard_Integer nPoints = aTD.NbPoints();
    if (nPoints < 2)
      continue;

    if (!mapEdges.IsBound(anEdge))
    {
      mapEdges.Bind(anEdge, aTD);
      nbNodes += nPoints;
    }
  }

  Standard_Integer nbE = mapEdges.Extent();
  if (nbE > 0)
  {
    myType |= Type_Line;
    allocateNodes (nbNodes);  
    myLines.InitArray(myIndexType, nbE, myAlloc);
  
    Standard_Integer aCurEdge = 0, aStartNode = 0;  
    NCollection_DataMap<TopoDS_Shape, GCPnts_TangentialDeflection>::Iterator
      anIt(mapEdges);
    for (; anIt.More(); anIt.Next())
    {
      const GCPnts_TangentialDeflection& aTD = anIt.Value();
      Standard_Integer nPoints = aTD.NbPoints();
      myLines.SetArray(aCurEdge, nPoints, myAlloc);
      for (Standard_Integer i = 0; i < nPoints; i++)
      {
        gp_XYZ aCurr = aTD.Value(i+1).XYZ();
        SetNode(i + aStartNode, aCurr);
        myLines.SetArrayIndex(aCurEdge, i, i + aStartNode);
      }
      aCurEdge++;
      aStartNode += nPoints;
    }
  }
}

//=======================================================================
//function : Clear
//purpose  : Reset all internal data members and structures
//=======================================================================

void NIS_Triangulated::Clear ()
{
  if (myNNodes) {
    myNNodes = 0;
    myAlloc->Free(mypNodes);
    mypNodes = 0L;
  }
  if (myNTriangles) {
    myNTriangles = 0;
    myAlloc->Free(mypTriangles);
    mypTriangles = 0L;
  }  

  myLines.Clear(myAlloc);
  myPolygons.Clear(myAlloc);
  myFans.Clear(myAlloc);
 
  myType = Type_None;
  myIsDrawPolygons = Standard_False;
  myPolygonType = static_cast<unsigned int>(Polygon_Default);
  myTriMode = static_cast<unsigned int>(TriMode_Triangles);
  if (GetDrawer().IsNull() == Standard_False) {
    GetDrawer()->SetUpdated(NIS_Drawer::Draw_Normal,
                            NIS_Drawer::Draw_Top,
                            NIS_Drawer::Draw_Transparent,
                            NIS_Drawer::Draw_Hilighted);
  }
}

//=======================================================================
//function : SetPolygonsPrs
//purpose  : 
//=======================================================================

void NIS_Triangulated::SetPolygonsPrs (const Standard_Integer nPolygons,
                                       const Standard_Integer nNodes)
{
  if (nPolygons <= 0)
    myType &= ~Type_Polygons;
  else {
    myType |= Type_Polygons;    
    
    allocateNodes (nNodes);
    myPolygons.InitArray(myIndexType, nPolygons, myAlloc);    
  }
}

//=======================================================================
//function : SetTriangulationPrs
//purpose  : 
//=======================================================================

void NIS_Triangulated::SetTriangulationPrs (const Standard_Integer nTri,
                                            const Standard_Integer nNodes)
{
  if (nTri <= 0)
    myType &= ~Type_Triangulation;
  else {
    myType |= Type_Triangulation;
    if (myNTriangles)
      myAlloc->Free(mypTriangles);
    allocateNodes (nNodes);

    myNTriangles = nTri;
    const Standard_Size nBytes = NBytesInd(3 * nTri, myIndexType);
    mypTriangles = static_cast<Standard_Integer *> (myAlloc->Allocate(nBytes));
  }
}

//=======================================================================
//function : SetTriangulationFanPrs
//purpose  : 
//=======================================================================

void NIS_Triangulated::SetTriangulationFanPrs
                                        (const Standard_Integer nFanTriangles,
                                         const Standard_Integer nNodes)
{  
  if (nFanTriangles <= 0)
    myType &= ~Type_Triangulation;
  else {
    myTriMode = static_cast<unsigned int>(TriMode_Triangle_Fan);
    if (!IsTriangulation())
      myType |= Type_Triangulation;    
    
    if (nNodes != myNNodes)
      allocateNodes (nNodes);

    myFans.InitArray(myIndexType, nFanTriangles, myAlloc);    
  }
}

//=======================================================================
//function : SetLinePrs
//purpose  : 
//=======================================================================

void NIS_Triangulated::SetLinePrs (const Standard_Integer nPoints,
                                   const Standard_Boolean isClosed,
                                   const Standard_Integer nNodes)
{
  if (nPoints <= 0)
    myType &= ~(Type_Loop | Type_Line);
  else {
    myType |= Type_Line;
    if (isClosed)
      myType |= Type_Loop;
   
    myType &= ~Type_Segments;
    allocateNodes (nNodes);

    myLines.InitArray(myIndexType, 1, myAlloc);
    myLines.SetArray(0, nPoints, myAlloc);   
  }
}

//=======================================================================
//function : SetSegmentPrs
//purpose  : 
//=======================================================================

void NIS_Triangulated::SetSegmentPrs (const Standard_Integer nSegments,
                                      const Standard_Integer nNodes)
{
  if (nSegments <= 0)
    myType &= ~(Type_Loop | Type_Segments);
  else {
    myType |= Type_Segments;

    myType &= ~(Type_Line | Type_Loop);
    allocateNodes (nNodes);
    
    myLines.InitArray(myIndexType, 1, myAlloc);
    myLines.SetArray(0, nSegments*2, myAlloc);    
  }
}

//=======================================================================
//function : ~NIS_Triangulated
//purpose  : Destructor
//=======================================================================

NIS_Triangulated::~NIS_Triangulated ()
{
  Clear();
}

//=======================================================================
//function : DefaultDrawer
//purpose  : 
//=======================================================================

NIS_Drawer * NIS_Triangulated::DefaultDrawer (NIS_Drawer * theDrawer) const
{
  NIS_TriangulatedDrawer * aDrawer =
    theDrawer ? static_cast<NIS_TriangulatedDrawer *>(theDrawer)
              : new NIS_TriangulatedDrawer (Quantity_NOC_RED);
  aDrawer->myIsDrawPolygons = myIsDrawPolygons;
  aDrawer->myPolygonType = myPolygonType;
  return aDrawer;
}

//=======================================================================
//function : ComputeBox
//purpose  : 
//=======================================================================

void NIS_Triangulated::ComputeBox (Bnd_B3f&                  theBox,
                                   const Standard_Integer    nNodes,
                                   const Standard_ShortReal* pNodes,
                                   const Standard_Integer    nCoord)
{
  theBox.Clear();
  if (nNodes > 0) {
    Standard_ShortReal aBox[6] = {
      pNodes[0], pNodes[1], 0.,
      pNodes[0], pNodes[1], 0.
    };
    if (nCoord > 2) {
      aBox[2] = pNodes[2];
      aBox[5] = pNodes[2];
    }
    for (Standard_Integer i = 1; i < nNodes; i++) {
      const Standard_ShortReal * pNode = &pNodes[i * nCoord];
      if (aBox[0] > pNode[0])
        aBox[0] = pNode[0];
      else if (aBox[3] < pNode[0])
        aBox[3] = pNode[0];
      if (aBox[1] > pNode[1])
        aBox[1] = pNode[1];
      else if (aBox[4] < pNode[1])
        aBox[4] = pNode[1];
      if (nCoord > 2) {
        if (aBox[2] > pNode[2])
          aBox[2] = pNode[2];
        else if (aBox[5] < pNode[2])
          aBox[5] = pNode[2];
      }
    }
    theBox.Add (gp_XYZ (Standard_Real(aBox[0]),
                        Standard_Real(aBox[1]),
                        Standard_Real(aBox[2])));
    theBox.Add (gp_XYZ (Standard_Real(aBox[3]),
                        Standard_Real(aBox[4]),
                        Standard_Real(aBox[5])));
  }
}

//=======================================================================
//function : computeBox
//purpose  : 
//=======================================================================

void NIS_Triangulated::computeBox ()
{
  ComputeBox (myBox, myNNodes, mypNodes, myNodeCoord);
}

//=======================================================================
//function : SetNode
//purpose  : 
//=======================================================================

void NIS_Triangulated::SetNode       (const Standard_Integer  ind,
                                      const gp_XYZ&           thePnt)
{
  if (ind >= myNNodes)
    Standard_OutOfRange::Raise ("NIS_Triangulated::SetNode");
  Standard_ShortReal * pNode = &mypNodes[myNodeCoord * ind];
  pNode[0] = Standard_ShortReal(thePnt.X());
  pNode[1] = Standard_ShortReal(thePnt.Y());
  if (myNodeCoord > 2)
    pNode[2] = Standard_ShortReal(thePnt.Z());
  setIsUpdateBox(Standard_True);
}

//=======================================================================
//function : SetNode
//purpose  : 
//=======================================================================

void NIS_Triangulated::SetNode       (const Standard_Integer  ind,
                                      const gp_XY&            thePnt)
{
  if (ind >= myNNodes)
    Standard_OutOfRange::Raise ("NIS_Triangulated::SetNode");
  Standard_ShortReal * pNode = &mypNodes[myNodeCoord * ind];
  pNode[0] = Standard_ShortReal(thePnt.X());
  pNode[1] = Standard_ShortReal(thePnt.Y());
  if (myNodeCoord > 2)
    pNode[2] = 0.f;
  setIsUpdateBox(Standard_True);
}

//=======================================================================
//function : SetTriangle
//purpose  : 
//=======================================================================

void NIS_Triangulated::SetTriangle   (const Standard_Integer  ind,
                                      const Standard_Integer  iNode0,
                                      const Standard_Integer  iNode1,
                                      const Standard_Integer  iNode2)
{
  if (ind >= myNTriangles)
    Standard_OutOfRange::Raise ("NIS_Triangulated::SetTriangle");
  switch (myIndexType) {
    case 0: // 8bit
    {
      unsigned char * pTri =
        reinterpret_cast<unsigned char *>(mypTriangles) + (3 * ind);
      pTri[0] = static_cast<unsigned char>(iNode0);
      pTri[1] = static_cast<unsigned char>(iNode1);
      pTri[2] = static_cast<unsigned char>(iNode2);
    }
    break;
    case 1: // 16bit
    {
      unsigned short * pTri =
        reinterpret_cast<unsigned short *>(mypTriangles) + (3 * ind);
      pTri[0] = static_cast<unsigned short>(iNode0);
      pTri[1] = static_cast<unsigned short>(iNode1);
      pTri[2] = static_cast<unsigned short>(iNode2);
    }
    break;
    default: // 32bit
    {
      Standard_Integer * pTri = &mypTriangles[3*ind];
      pTri[0] = iNode0;
      pTri[1] = iNode1;
      pTri[2] = iNode2;
    }
  }
}

//=======================================================================
//function : NFanInList 
//purpose  : 
//=======================================================================

Standard_Integer NIS_Triangulated::NFanInList (const Standard_Integer ind) const
{
  return myFans.NArrayIndexes(ind);  
}

//=======================================================================
//function : SetFanTriangles
//purpose  : 
//=======================================================================

void NIS_Triangulated::SetFanTriangles(const Standard_Integer ind,
                                       const Standard_Integer nbTris,
                                       const Standard_Integer startNodeIdx,
                                       const int* triList)
{  
  myFans.SetArray (ind, nbTris+2, triList, myAlloc, startNodeIdx);
}

//=======================================================================
//function : SetLineNode
//purpose  : 
//=======================================================================

void NIS_Triangulated::SetLineNode      (const Standard_Integer ind,
                                         const Standard_Integer iNode)
{
  myLines.SetArrayIndex(0, ind, iNode);
}

//=======================================================================
//function : SetPolygonNode
//purpose  : 
//=======================================================================

void NIS_Triangulated::SetPolygonNode         (const Standard_Integer indPoly,
                                               const Standard_Integer ind,
                                               const Standard_Integer iNode)
{
  myPolygons.SetArrayIndex(indPoly, ind, iNode);
}

//=======================================================================
//function : NPolygonNodes
//purpose  : 
//=======================================================================
Standard_Integer NIS_Triangulated::NPolygonNodes
                                         (const Standard_Integer indPoly)const
{  
  return myPolygons.NArrayIndexes(indPoly);  
}

//=======================================================================
//function : PolygonNode
//purpose  : 
//=======================================================================

Standard_Integer NIS_Triangulated::PolygonNode 
                                            (const Standard_Integer indPoly,
                                             const Standard_Integer ind)const
{  
  return myPolygons.ArrayIndex(indPoly, ind);
}

//=======================================================================
//function : SetPolygon
//purpose  : 
//=======================================================================

void NIS_Triangulated::SetPolygon (const Standard_Integer  ind,
                                   const Standard_Integer  theSz)
{  
  myPolygons.SetArray (ind, theSz, myAlloc);
}

//=======================================================================
//function : SetDrawPolygons
//purpose  : 
//=======================================================================

void NIS_Triangulated::SetDrawPolygons (const Standard_Boolean isDrawPolygons)
{
  if (GetDrawer().IsNull())
    myIsDrawPolygons = isDrawPolygons;
  else {
    if (myIsDrawPolygons != isDrawPolygons) {
      const Handle(NIS_TriangulatedDrawer) aDrawer =
        static_cast<NIS_TriangulatedDrawer *>(DefaultDrawer(0L));
      aDrawer->Assign (GetDrawer());
      aDrawer->myIsDrawPolygons = isDrawPolygons;
      SetDrawer (aDrawer);
      myIsDrawPolygons = isDrawPolygons;
    }
  }
}

//=======================================================================
//function : SetPolygonType
//purpose  : Set the type of polygon rendering
//=======================================================================

void NIS_Triangulated::SetPolygonType
                                (const NIS_Triangulated::PolygonType theType)
{
  if (GetDrawer().IsNull())
    myPolygonType = static_cast<unsigned int>(theType);
  else {
    if (myPolygonType != static_cast<unsigned int>(theType)) {
      const Handle(NIS_TriangulatedDrawer) aDrawer =
        static_cast<NIS_TriangulatedDrawer *>(DefaultDrawer(0L));
      aDrawer->Assign (GetDrawer());
      aDrawer->myPolygonType = theType;
      SetDrawer (aDrawer);
      myPolygonType = static_cast<unsigned int>(theType);
    }
  }
}

//=======================================================================
//function : SetLineWidth
//purpose  : Set the width of line presentations in pixels.
//=======================================================================

void NIS_Triangulated::SetLineWidth (const Standard_Real    theWidth)
{
  const Handle(NIS_TriangulatedDrawer) aDrawer =
    static_cast<NIS_TriangulatedDrawer *>(DefaultDrawer(0L));
  aDrawer->Assign (GetDrawer());
  aDrawer->myLineWidth = (Standard_ShortReal) theWidth;
  SetDrawer (aDrawer);
}

//=======================================================================
//function : GetLineWidth
//purpose  : Get the width of line presentation in pixels.
//=======================================================================

Standard_Real NIS_Triangulated::GetLineWidth () const
{
    const Handle(NIS_TriangulatedDrawer) aDrawer =
        static_cast<NIS_TriangulatedDrawer*>(DefaultDrawer(0L));
    aDrawer->Assign(GetDrawer());
    return (Standard_Real)(aDrawer->myLineWidth);
}

//=======================================================================
//function : SetLineStyle
//purpose  : Set the type of line presentation.
//=======================================================================

void NIS_Triangulated::SetLineStyle  (const Standard_Integer theStyle)
{
    const Handle(NIS_TriangulatedDrawer) aDrawer =
        static_cast<NIS_TriangulatedDrawer*>(DefaultDrawer(0L));
    aDrawer->Assign(GetDrawer());
    aDrawer->myLineType = theStyle;
    SetDrawer(aDrawer);
}

//=======================================================================
//function : Clone
//purpose  : 
//=======================================================================

void NIS_Triangulated::Clone (const Handle_NCollection_BaseAllocator& theAlloc,
                              Handle_NIS_InteractiveObject& theDest) const
{
  Handle(NIS_Triangulated) aNewObj;
  if (theDest.IsNull()) {
    aNewObj = new (theAlloc) NIS_Triangulated(myNNodes, myNodeCoord == 2,
                                              theAlloc);
    aNewObj->myIsCloned = Standard_True;
    theDest = aNewObj;
  } else {
    aNewObj = reinterpret_cast<NIS_Triangulated*> (theDest.operator->());
    aNewObj->myAlloc = theAlloc.operator->();
    aNewObj->myNNodes = 0;
    aNewObj->allocateNodes(myNNodes);
  }
  NIS_InteractiveObject::Clone(theAlloc, theDest);
  if (myNNodes > 0)
  {
    // copy nodes
    memcpy(aNewObj->mypNodes, mypNodes,
           myNNodes * myNodeCoord * sizeof(Standard_ShortReal));
    // copy triangles
    aNewObj->myNTriangles = myNTriangles;
    if (myNTriangles) {
      const Standard_Size nBytes = NBytesInd(3 * myNTriangles, myIndexType);
      aNewObj->mypTriangles = static_cast<Standard_Integer *>
        (theAlloc->Allocate(nBytes));
      memcpy(aNewObj->mypTriangles, mypTriangles, nBytes);
    }
    // copy lines/segments
    myLines.Clone(theAlloc, aNewObj->myLines);
    
    // copy polygons
    myPolygons.Clone(theAlloc, aNewObj->myPolygons);
    aNewObj->myTriMode = myTriMode;

    // copy fans
    myFans.Clone(theAlloc, aNewObj->myFans);    
  }
  aNewObj->myType = myType;
  aNewObj->myIsDrawPolygons = myIsDrawPolygons;
  aNewObj->myIndexType = myIndexType;
  aNewObj->myPolygonType = myPolygonType;
}

//=======================================================================
//function : Delete
//purpose  : 
//=======================================================================

void NIS_Triangulated::Delete () const
{
  if (myIsCloned == Standard_False)
    Standard_Transient::Delete();
  else {
    // Call the destructor and then release the memory occupied by the instance.
    // This is required when the NIS_Triangulated instance is allocated in
    // the same allocator as its internal arrays.
    NIS_Triangulated* pThis = const_cast<NIS_Triangulated*>(this);
    pThis->~NIS_Triangulated();
    myAlloc->Free(pThis);
  }
}

//=======================================================================
//function : Intersect
//purpose  : 
//=======================================================================

Standard_Boolean NIS_Triangulated::Intersect
                                        (const Bnd_B3f&         theBox,
                                         const gp_Trsf&         theTrf,
                                         const Standard_Boolean isFullIn) const
{
  Standard_Boolean aResult (isFullIn);

  if ((myType & Type_Triangulation) && myIsDrawPolygons == Standard_False) {
    unsigned int iNode = 0;
    for (; iNode < myNNodes * myNodeCoord; iNode += myNodeCoord)
    {
      gp_XYZ aPnt (static_cast<Standard_Real>(mypNodes[iNode+0]),
                   static_cast<Standard_Real>(mypNodes[iNode+1]), 0.);
      if (myNodeCoord > 2)
        aPnt.SetZ (static_cast<Standard_Real>(mypNodes[iNode+2]));
      theTrf.Transforms (aPnt);
      if (theBox.IsOut (aPnt)) {
        if (isFullIn) {
          aResult = Standard_False;
          break;
        }
      } else {
        if (isFullIn == Standard_False) {
          aResult = Standard_True;
          break;
        }
      }
    }
  }
  if (aResult == isFullIn) {
    if ((myType & Type_Segments) && (NLineNodes() > 0)) {
      const Standard_Integer * anSegs = static_cast <Standard_Integer *>(myLines.ArrayIndexes(0));
      const Standard_Integer nNodes = myLines.NArrayIndexes(0);
      aResult = intersectArray (anSegs, nNodes, theBox, theTrf, isFullIn, 2);      
    } else if (myType & Type_Line) {
      for (Standard_Integer iLine = 0; iLine < NLines(); iLine++) {
        const Standard_Integer* aLine = static_cast <Standard_Integer *>(myLines.ArrayIndexes(iLine));
        const Standard_Integer nNodes = myLines.NArrayIndexes(iLine);
        aResult = intersectArray (aLine, nNodes, theBox, theTrf, isFullIn);
        
        if (aResult == isFullIn && (myType & Type_Loop)) {          
          // check edge between first and last points
          const gp_Pnt aPntLast[2] = {
            nodeAtInd(aLine, nNodes-1).Transformed(theTrf),
            nodeAtInd(aLine, 0).Transformed(theTrf)
          };
          if (isFullIn) {
            if (seg_box_included (theBox, aPntLast) == 0)
              aResult = Standard_False;
          } else {
            if (seg_box_intersect (theBox, aPntLast))
              aResult = Standard_True;
          }
        }
        
        if (aResult != isFullIn) {
          // aResult==true, isFullIn==false, so at least one element is included
          // or
          // aResult==false, isFullIn==true, so at least one element is NOT included
          break;
        }
      }
    } else if ((myType & Type_Polygons) && myIsDrawPolygons) {      
      for (Standard_Integer iPoly = 0; iPoly < NPolygons(); iPoly++) {
        const Standard_Integer* aPoly = static_cast <Standard_Integer *>(myPolygons.ArrayIndexes(iPoly));
        const Standard_Integer nNodes = myPolygons.NArrayIndexes(iPoly);
        aResult = intersectArray (aPoly, nNodes, theBox, theTrf, isFullIn);

        if (aResult == isFullIn) {          
          // check edge between first and last points
          const gp_Pnt aPntLast[2] = {
            nodeAtInd(aPoly, nNodes-1).Transformed(theTrf),
            nodeAtInd(aPoly, 0).Transformed(theTrf)
          };
          if (isFullIn) {
            if (seg_box_included (theBox, aPntLast) == 0)
              aResult = Standard_False;
          } else {
            if (seg_box_intersect (theBox, aPntLast))
              aResult = Standard_True;
          }
        }
        
        if (aResult != isFullIn) {
          // aResult==true, isFullIn==false, so at least one element is included
          // or
          // aResult==false, isFullIn==true, so at least one element is NOT included
          break;
        }
      }
    }
  }
  return aResult;
}

//=======================================================================
//function : intersectArray 
//purpose  : 
//=======================================================================

Standard_Boolean NIS_Triangulated::intersectArray
                        (const Standard_Integer* theArray,
                         const Standard_Integer theNNodes,
                         const Bnd_B3f&         theBox,
                         const gp_Trsf&         theTrf,
                         const Standard_Boolean isFullIn,
                         const Standard_Integer theIncremIdx) const 
{
  Standard_Boolean aResult = Standard_False;
  if (!theArray)
    return aResult;

  for (Standard_Integer i = 0; i < theNNodes-1; i += theIncremIdx) {
    const gp_Pnt aPnt[2] = {
      nodeAtInd(theArray, i+0).Transformed(theTrf),
      nodeAtInd(theArray, i+1).Transformed(theTrf)
    };
    if (isFullIn) {
      if (seg_box_included (theBox, aPnt) == 0) {
        aResult = Standard_False;
        break;
      }
    } else {
      if (seg_box_intersect (theBox, aPnt)) {
        aResult = Standard_True;
        break;
      }
    }
  }
  
  return aResult;
}

//=======================================================================
//function : Intersect
//purpose  : 
//=======================================================================

Standard_Real NIS_Triangulated::Intersect (const gp_Ax1&       theAxis,
                                           const Standard_Real theOver) const
{
  Standard_Real aResult (RealLast());
  Standard_Real start[3], dir[3];
  theAxis.Location().Coord(start[0], start[1], start[2]);
  theAxis.Direction().Coord(dir[0], dir[1], dir[2]);
  double anInter (RealLast());
  const Standard_Real anOver2 = theOver*theOver;

  if ((myType & Type_Triangulation) && (myIsDrawPolygons == Standard_False))
  {
    for (Standard_Integer i = 0; i < myNTriangles; i++) {
      Standard_Boolean isIntersect(Standard_False);
      Standard_Integer idx1 = -1, idx2 = -1, idx3 = -1;
      if (myIndexType == 0) {
        const unsigned char * pTri =
          reinterpret_cast<unsigned char *>(mypTriangles) + (3 * i);
        idx1 = myNodeCoord * pTri[0];
        idx2 = myNodeCoord * pTri[1];
        idx3 = myNodeCoord * pTri[2];
      } else if (myIndexType == 1) {
        const unsigned short * pTri =
          reinterpret_cast<unsigned short *>(mypTriangles) + (3 * i);
        idx1 = myNodeCoord * pTri[0];
        idx2 = myNodeCoord * pTri[1];
        idx3 = myNodeCoord * pTri[2];
      } else {
        const Standard_Integer * pTri = &mypTriangles[3 * i];
        idx1 = myNodeCoord * pTri[0];
        idx2 = myNodeCoord * pTri[1];
        idx3 = myNodeCoord * pTri[2];
      }

      if (myNodeCoord > 2)
        isIntersect = tri_line_intersect (start, dir,
                                          &mypNodes[idx1],
                                          &mypNodes[idx2],
                                          &mypNodes[idx3],
                                          &anInter);
      else
        isIntersect = tri2d_line_intersect (start, dir, anOver2,
                                            &mypNodes[idx1],
                                            &mypNodes[idx2],
                                            &mypNodes[idx3],
                                            &anInter);
      if (isIntersect && anInter < aResult)
        aResult = anInter;
    }
    for (Standard_Integer i = 0; i < myFans.NArrays(); i++) 
    {   
       Standard_Integer idx1 = myFans.ArrayIndex(i, 0);
       Standard_Integer nbTriIdx = myFans.NArrayIndexes(i);
       for (Standard_Integer j = 1; j < nbTriIdx-1; j++) {
         Standard_Boolean isIntersect(Standard_False);
         Standard_Integer idx2 = myFans.ArrayIndex(i, j);
         Standard_Integer idx3 = myFans.ArrayIndex(i, j+1);
       
         if (myNodeCoord > 2)
           isIntersect = tri_line_intersect (start, dir,
                                             Node(idx1),
                                             Node(idx2),
                                             Node(idx3),
                                             &anInter);
         else{
           isIntersect = tri2d_line_intersect (start, dir, anOver2,
                                               Node(idx1),
                                               Node(idx2),
                                               Node(idx3),
                                               &anInter);           
         }
         if (isIntersect && anInter < aResult)
           aResult = anInter;
       }
     }     
  }
  if ((myType & Type_Segments) && (NLineNodes() > 0)) {
    Standard_Integer i = 0;
    Standard_Integer * aSegs = static_cast <Standard_Integer *>(myLines.ArrayIndexes(0));
    if (myNodeCoord > 2) {      
      for (; i < NLineNodes()-1; i+=2) {
        if (seg_line_intersect (theAxis.Location().XYZ(),
                                theAxis.Direction().XYZ(), anOver2,
                                nodeArrAtInd(aSegs, i+0),
                                nodeArrAtInd(aSegs, i+1),
                                &anInter))
          if (anInter < aResult)
            aResult = anInter;
      }
    }
    else
      for (; i < NLineNodes()-1; i+=2) {
        if (seg2d_line_intersect (theAxis.Location().XYZ(),
                                  theAxis.Direction().XYZ(), anOver2,
                                  nodeArrAtInd(aSegs, i+0),
                                  nodeArrAtInd(aSegs, i+1),
                                  &anInter))
          if (anInter < aResult)
            aResult = anInter;
      }
  } else if (myType & Type_Line) {
    Standard_Integer i = 1;
    if (myNodeCoord > 2) {
      for (Standard_Integer iLine = 0; iLine < NLines(); iLine++) {
        const Standard_Integer* aLine = static_cast <Standard_Integer *>(myLines.ArrayIndexes(iLine));
        const Standard_Integer nNodes = NLineNodes(iLine);
        for (; i < nNodes; i++) {      
          if (seg_line_intersect (theAxis.Location().XYZ(),
                                  theAxis.Direction().XYZ(), anOver2,
                                  nodeArrAtInd(aLine, i-1),
                                  nodeArrAtInd(aLine, i-0),
                                  &anInter))
            if (anInter < aResult)
              aResult = anInter;
        }
        if (myType & Type_Loop)
          if (seg_line_intersect (theAxis.Location().XYZ(),
                                  theAxis.Direction().XYZ(), anOver2,
                                  nodeArrAtInd(aLine, nNodes-1),
                                  nodeArrAtInd(aLine, 0),
                                  &anInter))
          if (anInter < aResult)
            aResult = anInter;
      }
    } else {
      for (Standard_Integer iLine = 0; iLine < NLines(); iLine++) {
        const Standard_Integer* aLine = static_cast <Standard_Integer *>(myLines.ArrayIndexes(iLine));
        const Standard_Integer nNodes = NLineNodes(iLine);
        for (; i < nNodes; i++) {      
          if (seg2d_line_intersect (theAxis.Location().XYZ(),
                                    theAxis.Direction().XYZ(), anOver2,
                                    nodeArrAtInd(aLine, i-1),
                                    nodeArrAtInd(aLine, i-0),
                                    &anInter))
            if (anInter < aResult)
              aResult = anInter;
        }
        if (myType & Type_Loop)
          if (seg2d_line_intersect (theAxis.Location().XYZ(),
                                    theAxis.Direction().XYZ(), anOver2,
                                    nodeArrAtInd(aLine, nNodes-1),
                                    nodeArrAtInd(aLine, 0),
                                    &anInter))
          if (anInter < aResult)
            aResult = anInter;
      }
    }
  }

  if ((myType & Type_Polygons) && myIsDrawPolygons) {
    for (Standard_Integer iPoly = 0; iPoly < NPolygons(); iPoly++) {
      const Standard_Integer * aPoly = static_cast <Standard_Integer *>(myPolygons.ArrayIndexes(iPoly));
      const Standard_Integer nNodes = NPolygonNodes(iPoly) - 1;
      Standard_Integer i = 0;
      if (myNodeCoord > 2) {
        for (; i < nNodes; i++) {          
          if (seg_line_intersect (theAxis.Location().XYZ(),
                                  theAxis.Direction().XYZ(), anOver2,
                                  nodeArrAtInd(aPoly, i+0),
                                  nodeArrAtInd(aPoly, i+1),
                                  &anInter))
            if (anInter < aResult)
              aResult = anInter;
        }
        if (seg_line_intersect (theAxis.Location().XYZ(),
                                theAxis.Direction().XYZ(), anOver2,
                                nodeArrAtInd(aPoly, nNodes),
                                nodeArrAtInd(aPoly, 0),
                                &anInter))
          if (anInter < aResult)
            aResult = anInter;
      } else {
        for (; i < nNodes; i++) {
          // Node index is incremented for the head of polygon indice array
          if (seg2d_line_intersect (theAxis.Location().XYZ(),
                                    theAxis.Direction().XYZ(), anOver2,
                                    nodeArrAtInd(aPoly, i+0),
                                    nodeArrAtInd(aPoly, i+1),
                                    &anInter))
            if (anInter < aResult)
              aResult = anInter;
        }
        if (seg2d_line_intersect (theAxis.Location().XYZ(),
                                  theAxis.Direction().XYZ(), anOver2,
                                  nodeArrAtInd(aPoly, nNodes),
                                  nodeArrAtInd(aPoly, 0),
                                  &anInter))
          if (anInter < aResult)
            aResult = anInter;
      }
    }
  }
  return aResult;
}

//=======================================================================
//function : Intersect
//purpose  : 
//=======================================================================
Standard_Boolean NIS_Triangulated::Intersect
                    (const NCollection_List<gp_XY> &thePolygon,
                     const gp_Trsf                 &theTrf,
                     const Standard_Boolean         isFullIn) const
{
  Standard_Boolean aResult (isFullIn);

  if ((myType & Type_Triangulation) && myIsDrawPolygons == Standard_False) {
    unsigned int iNode = 0;
    for (; iNode < myNNodes * myNodeCoord; iNode += myNodeCoord)
    {
      gp_XYZ aPnt (static_cast<Standard_Real>(mypNodes[iNode+0]),
                   static_cast<Standard_Real>(mypNodes[iNode+1]), 0.);
      if (myNodeCoord > 2)
        aPnt.SetZ (static_cast<Standard_Real>(mypNodes[iNode+2]));
      theTrf.Transforms (aPnt);

      gp_XY aP2d(aPnt.X(), aPnt.Y());

      if (!NIS_Triangulated::IsIn(thePolygon, aP2d)) {
        if (isFullIn) {
          aResult = Standard_False;
          break;
        }
      } else {
        if (isFullIn == Standard_False) {
          aResult = Standard_True;
          break;
        }
      }
    }
  }
  if (aResult == isFullIn) {
    if ((myType & Type_Segments) && (NLineNodes() > 0)) {
      Standard_Integer * aSegs = static_cast <Standard_Integer *>(myLines.ArrayIndexes(0));
      for (Standard_Integer i = 0; i < NLineNodes()-1; i+=2) {
        const gp_Pnt aPnt[2] = {
          nodeAtInd(aSegs, i+0).Transformed(theTrf),
          nodeAtInd(aSegs, i+1).Transformed(theTrf)
        };
        const gp_XY aP2d[2] = { gp_XY(aPnt[0].X(), aPnt[0].Y()),
                                gp_XY(aPnt[1].X(), aPnt[1].Y()) };

        if (isFullIn) {
          if (seg_polygon_included (thePolygon, aP2d) == 0) {
            aResult = Standard_False;
            break;
          }
        } else {
          if (seg_polygon_intersect (thePolygon, aP2d)) {
            aResult = Standard_True;
            break;
          }
        }
      }
    } else if (myType & Type_Line) {
      for (Standard_Integer iLine = 0; iLine < NLines(); iLine++) {
        const Standard_Integer* aLine = static_cast <Standard_Integer *>(myLines.ArrayIndexes(iLine));
        const Standard_Integer nNodes = NLineNodes(iLine);
        for (Standard_Integer i = 0; i < nNodes-1; i++) {      
          const gp_Pnt aPnt[2] = {
            nodeAtInd(aLine, i+0).Transformed(theTrf),
            nodeAtInd(aLine, i+1).Transformed(theTrf)
          };
          const gp_XY aP2d[2] = { gp_XY(aPnt[0].X(), aPnt[0].Y()),
                                  gp_XY(aPnt[1].X(), aPnt[1].Y()) };
          if (isFullIn) {
            if (seg_polygon_included (thePolygon, aP2d) == 0) {
              aResult = Standard_False;
              break;
            }
          } else {
            if (seg_polygon_intersect (thePolygon, aP2d)) {
              aResult = Standard_True;
              break;
            }
          }
        }
        if (aResult == isFullIn && (myType & Type_Loop)) {
          const gp_Pnt aPntLast[2] = {
            nodeAtInd(aLine, nNodes-1).Transformed(theTrf),
            nodeAtInd(aLine, 0).Transformed(theTrf)
          };
          const gp_XY aP2dLast[2] = { gp_XY(aPntLast[0].X(), aPntLast[0].Y()),
                                      gp_XY(aPntLast[1].X(), aPntLast[1].Y()) };

          if (isFullIn) {
            if (seg_polygon_included (thePolygon, aP2dLast) == 0)
              aResult = Standard_False;
          } else {
            if (seg_polygon_intersect (thePolygon, aP2dLast))
              aResult = Standard_True;
          }
        }
      }
    } else if ((myType & Type_Polygons) && myIsDrawPolygons) {
      for (Standard_Integer iPoly = 0; iPoly < NPolygons(); iPoly++) {
        const Standard_Integer * aPoly = static_cast <Standard_Integer *>(myPolygons.ArrayIndexes(iPoly));
        const Standard_Integer nNodes = NPolygonNodes(iPoly);      
        for (Standard_Integer i = 1; i < nNodes; i++) {
          const gp_Pnt aPnt[2] = {
            nodeAtInd(aPoly, i-1).Transformed(theTrf),
            nodeAtInd(aPoly, i+0).Transformed(theTrf)
          };
          const gp_XY aP2d[2] = { gp_XY(aPnt[0].X(), aPnt[0].Y()),
                                  gp_XY(aPnt[1].X(), aPnt[1].Y()) };
          if (isFullIn) {
            if (seg_polygon_included (thePolygon, aP2d) == 0) {
              aResult = Standard_False;
              break;
            }
          } else {
            if (seg_polygon_intersect (thePolygon, aP2d)) {
              aResult = Standard_True;
              break;
            }
          }
        }
        if (aResult == isFullIn) {
          const gp_Pnt aPntLast[2] = {
            nodeAtInd(aPoly, nNodes-1).Transformed(theTrf),
            nodeAtInd(aPoly, 0).Transformed(theTrf)
          };
          const gp_XY aP2dLast[2] = { gp_XY(aPntLast[0].X(), aPntLast[0].Y()),
                                      gp_XY(aPntLast[1].X(), aPntLast[1].Y()) };
          if (isFullIn) {
            if (seg_polygon_included (thePolygon, aP2dLast) == 0)
              aResult = Standard_False;
          } else {
            if (seg_polygon_intersect (thePolygon, aP2dLast))
              aResult = Standard_True;
          }
        }
      }
    }
  }
  return aResult;
}

/* =====================================================================
Function   : determinant
Purpose    : Calculate the minor of the given matrix, defined by the columns
             specified by values c1, c2, c3
Parameters : 
Returns    : determinant value
History    : 
======================================================================== */

static double determinant (const double a[3][4],
                           const int    c1,
                           const int    c2,
                           const int    c3)
{
  return a[0][c1]*a[1][c2]*a[2][c3] +
         a[0][c2]*a[1][c3]*a[2][c1] +
         a[0][c3]*a[1][c1]*a[2][c2] -
         a[0][c3]*a[1][c2]*a[2][c1] -
         a[0][c2]*a[1][c1]*a[2][c3] -
         a[0][c1]*a[1][c3]*a[2][c2];
}

/* =====================================================================
Function   : tri_line_intersect
Purpose    : Intersect a triangle with a line
Parameters : start  - coordinates of the origin of the line
             dir    - coordinates of the direction of the line (normalized)
             V0     - first vertex of the triangle
             V1     - second vertex of the triangle
             V2     - third vertex of the triangle
             tInter - output value, contains the parameter of the intersection
                      point on the line (if found). May be NULL pointer
Returns    : int = 1 if intersection found, 0 otherwise
======================================================================== */

int NIS_Triangulated::tri_line_intersect (const double      start[3],
                                          const double      dir[3],
                                          const float       V0[3],
                                          const float       V1[3],
                                          const float       V2[3],
                                          double            * tInter)
{
  int res = 0;
  const double conf = 1E-15;

  const double array[][4] = {
    { -dir[0],
      double(V1[0] - V0[0]), double(V2[0] - V0[0]), start[0] - double(V0[0]) },
    { -dir[1],
      double(V1[1] - V0[1]), double(V2[1] - V0[1]), start[1] - double(V0[1]) },
    { -dir[2],
      double(V1[2] - V0[2]), double(V2[2] - V0[2]), start[2] - double(V0[2]) }
  };

  const double d  = determinant( array, 0, 1, 2 );
  const double dt = determinant( array, 3, 1, 2 );
 
  if (d > conf) {
    const double da = determinant( array, 0, 3, 2 );
    if (da > -conf) {
      const double db = determinant( array, 0, 1, 3 );
      res = (db > -conf && da+db <= d+conf);
    }
  } else if (d < -conf) {
    const double da = determinant( array, 0, 3, 2 );
    if (da < conf) {
      const double db = determinant( array, 0, 1, 3 );
      res = (db < conf && da+db >= d-conf);
    }
  }
  if (res && tInter)
    *tInter = dt / d;

  return res;
}

/* =====================================================================
Function   : tri2d_line_intersect
Purpose    : Intersect a 2D triangle with a 3D line. Z coordinate of triangle
           : is zero
Parameters : start  - coordinates of the origin of the line
             dir    - coordinates of the direction of the line (normalized)
             over2  - maximal square distance between the line and one side of 
                      triangle in intersection state.
             V0     - first vertex of the triangle
             V1     - second vertex of the triangle
             V2     - third vertex of the triangle
             tInter - output value, contains the parameter of the intersection
                      point on the line (if found). May be NULL pointer
Returns    : int = 1 if intersection found, 0 otherwise
======================================================================== */

int NIS_Triangulated::tri2d_line_intersect (const double      start[3],
                                            const double      dir[3],
                                            const double      over2,
                                            const float       V0[2],
                                            const float       V1[2],
                                            const float       V2[2],
                                            double            * tInter)
{
  int res = 0;
  const double conf = 1E-15;

  // Parallel case is excluded
  if (dir[2] * dir[2] > conf)
  {
    // Find the 2d intersection of (start, dir) with the plane Z = 0.
    const double p2d[2] = {
      start[0] - dir[0] * start[2] / dir[2],
      start[1] - dir[1] * start[2] / dir[2]
    };

    // Classify the 2d intersection using barycentric coordinates
    // (http://www.blackpawn.com/texts/pointinpoly/)
    const double v[][2] = {
      { static_cast<double>(V1[0]-V0[0]), static_cast<double>(V1[1]-V0[1]) },
      { static_cast<double>(V2[0]-V0[0]), static_cast<double>(V2[1]-V0[1]) },
      { static_cast<double>(p2d[0]-V0[0]), static_cast<double>(p2d[1]-V0[1]) }
    };
    const double dot00 = v[0][0]*v[0][0] + v[0][1]*v[0][1];
    const double dot01 = v[0][0]*v[1][0] + v[0][1]*v[1][1];
    const double dot02 = v[0][0]*v[2][0] + v[0][1]*v[2][1];
    const double dot11 = v[1][0]*v[1][0] + v[1][1]*v[1][1];
    const double dot12 = v[1][0]*v[2][0] + v[1][1]*v[2][1];
    const double denom = (dot00 * dot11 - dot01 * dot01);
    if (denom * denom < conf * conf) {
      // Find the bounding box of the triangle
      double bbox[][2] = { { V0[0], V0[1] }, { V0[0], V0[1] }};
      if (V1[0] < V0[0])
        bbox[0][0] = V1[0];
      else
        bbox[1][0] = V1[0];
      if (V1[1] < V0[1])
        bbox[0][1] = V1[1];
      else
        bbox[1][1] = V1[1];
      if (V2[0] < bbox[0][0])
        bbox[0][0] = V2[0];
      else if (V2[0] > bbox[1][0])
        bbox[1][0] = V2[0];
      if (V2[1] < bbox[0][1])
        bbox[0][1] = V2[1];
      else if (V2[1] > bbox[1][1])
        bbox[1][1] = V2[1];
      // Classify toward the bounding box
      if (p2d[0] > bbox[0][0] - conf && p2d[0] < bbox[1][0] + conf &&
          p2d[1] > bbox[0][1] - conf && p2d[1] < bbox[1][1] + conf)
        // Point on the 1st side of the triangle
        res = (dot01 > -conf && dot00 < dot11 + conf);
    } else {
      // Barycentric coordinates of the point
      const double uc = (dot11 * dot02 - dot01 * dot12) / denom;
      const double vc = (dot00 * dot12 - dot01 * dot02) / denom;
      if ((uc > -conf) && (vc > -conf) && (uc + vc < 1. + conf))
        res = 1;
      else if (over2 > conf) {
        // seek the proximity to the border of the triangle - two sides enough
        if (dot02 > 0. && dot02 < dot00) {
          const double aD = v[0][0]*v[2][1] - v[0][1]*v[2][0];
          if (aD * aD < dot00 * over2)
            res = 1;
        }
        if (res == 0 && dot12 > 0. && dot12 < dot11) {
          const double aD = v[1][0]*v[2][1] - v[1][1]*v[2][0];
          if (aD * aD < dot11 * over2)
            res = 1;
        }
      }
    }
  }
  if (res && tInter)
    *tInter = -start[2] / dir[2];

  return res;
}

/* =====================================================================
Function   : seg_line_intersect
Purpose    : Intersect a segment with a line
Parameters : start  - coordinates of the origin of the line
             dir    - coordinates of the direction of the line (normalized)
             over2  - maximal square distance between the line and the segment
                      for intersection state
             V0     - first vertex of the segment
             V1     - second vertex of the segment
             tInter - output value, contains the parameter of the intersection
                      point on the line (if found). May be NULL pointer
Returns    : int = 1 if intersection found, 0 otherwise
======================================================================== */

int NIS_Triangulated::seg_line_intersect (const gp_XYZ&     aStart,
                                          const gp_XYZ&     aDir,
                                          const double      over2,
                                          const float       V0[3],
                                          const float       V1[3],
                                          double            * tInter)
{
  int res = 0;
  const gp_XYZ aDirSeg (V1[0]-V0[0], V1[1]-V0[1], V1[2]-V0[2]);
  gp_XYZ aDirN = aDirSeg ^ aDir;
  Standard_Real aMod2 = aDirN.SquareModulus();
  if (aMod2 < Precision::Confusion() * 0.001) {
    const gp_XYZ aDelta0 (V0[0]-aStart.X(), V0[1]-aStart.Y(), V0[2]-aStart.Z());
    if ((aDelta0 ^ aDir).SquareModulus() < over2) {
      res = 1;
      const gp_XYZ aDelta1 (V1[0]-aStart.X(),V1[1]-aStart.Y(),V1[2]-aStart.Z());
      if (tInter)
        * tInter = Min (aDelta0 * aDir, aDelta1 * aDir);
    }
  } else {
    // distance between two unlimited lines
    const gp_XYZ aPnt0 (V0[0], V0[1], V0[2]);
    const Standard_Real aDistL = (aDirN * aPnt0) - aDirN * aStart;
    if (aDistL*aDistL < over2 * aMod2) {
      const gp_XYZ aPnt1 (V1[0], V1[1], V1[2]);
      Standard_Real aDist[3] = {
        ((aPnt0 - aStart) ^ aDir).Modulus(),
        ((aPnt1 - aStart) ^ aDir).Modulus(),
        0.
      };
      // Find the intermediate point by interpolation using the distances from
      // the end points.
      const gp_XYZ aPntI =
        (aPnt0 * aDist[1] + aPnt1 * aDist[0]) / (aDist[0] + aDist[1]);
      aDist[2] = ((aPntI - aStart) ^ aDir).Modulus();
      if (aDist[2] < aDist[0] && aDist[2] < aDist[1]) {
        if (aDist[2]*aDist[2] < over2) {
          res = 1;
          if (tInter)
            * tInter = (aPntI - aStart) * aDir;
        }
      } else if (aDist[0] < aDist[1]) {
        if (aDist[0] * aDist[0] < over2) {
          res = 1;
          if (tInter)
            * tInter = (aPnt0 - aStart) * aDir;
        }
      } else {
        if (aDist[1] * aDist[1] < over2) {
          res = 1;
          if (tInter)
            * tInter = (aPnt1 - aStart) * aDir;
        }
      }
    }
  }
  return res;
}

/* =====================================================================
Function   : seg2d_line_intersect
Purpose    : Intersect a 2D segment with a 3D line
Parameters : start  - coordinates of the origin of the line
             dir    - coordinates of the direction of the line (normalized)
             over2  - maximal square distance between the line and the segment
                      for intersection state
             V0     - first vertex of the segment
             V1     - second vertex of the segment
             tInter - output value, contains the parameter of the intersection
                      point on the line (if found). May be NULL pointer
Returns    : int = 1 if intersection found, 0 otherwise
======================================================================== */

int NIS_Triangulated::seg2d_line_intersect (const gp_XYZ&     aStart,
                                            const gp_XYZ&     aDir,
                                            const double      over2,
                                            const float       V0[2],
                                            const float       V1[2],
                                            double            * tInter)
{
  int res = 0;
  const gp_XYZ aDirSeg (V1[0]-V0[0], V1[1]-V0[1], 0.);
  gp_XYZ aDirN = aDirSeg ^ aDir;
  Standard_Real aMod2 = aDirN.SquareModulus();
  if (aMod2 < Precision::Confusion() * 0.001) {
    const gp_XYZ aDelta0 (V0[0]-aStart.X(), V0[1]-aStart.Y(), -aStart.Z());
    if ((aDelta0 ^ aDir).SquareModulus() < over2) {
      res = 1;
      const gp_XYZ aDelta1 (V1[0]-aStart.X(), V1[1]-aStart.Y(), -aStart.Z());
      if (tInter)
        * tInter = Min (aDelta0 * aDir, aDelta1 * aDir);
    }
  } else {
    // distance between two unlimited lines
    const gp_XYZ aPnt0 (V0[0], V0[1], 0.);
    const Standard_Real aDistL = (aDirN * aPnt0) - aDirN * aStart;
    if (aDistL*aDistL < over2 * aMod2) {
      const gp_XYZ aPnt1 (V1[0], V1[1], 0.);
      Standard_Real aDist[3] = {
        ((aPnt0 - aStart) ^ aDir).Modulus(),
        ((aPnt1 - aStart) ^ aDir).Modulus(),
        0.
      };
      // Find the intermediate point by interpolation using the distances from
      // the end points.
      const gp_XYZ aPntI =
        (aPnt0 * aDist[1] + aPnt1 * aDist[0]) / (aDist[0] + aDist[1]);
      aDist[2] = ((aPntI - aStart) ^ aDir).Modulus();
      if (aDist[2] < aDist[0] && aDist[2] < aDist[1]) {
        if (aDist[2]*aDist[2] < over2) {
          res = 1;
          if (tInter)
            * tInter = (aPntI - aStart) * aDir;
        }
      } else if (aDist[0] < aDist[1]) {
        if (aDist[0] * aDist[0] < over2) {
          res = 1;
          if (tInter)
            * tInter = (aPnt0 - aStart) * aDir;
        }
      } else {
        if (aDist[1] * aDist[1] < over2) {
          res = 1;
          if (tInter)
            * tInter = (aPnt1 - aStart) * aDir;
        }
      }
    }
  }
  return res;
}

//=======================================================================
//function : seg_box_intersect
//purpose  : 
//=======================================================================

int NIS_Triangulated::seg_box_intersect (const Bnd_B3f& theBox,
                                         const gp_Pnt thePnt[2])
{
  int aResult (1);
  if ((thePnt[1].XYZ() - thePnt[0].XYZ()).SquareModulus()
      < Precision::Confusion() * 0.0001)
    aResult = 0;
  else {
    const gp_Dir aDir (thePnt[1].XYZ() - thePnt[0].XYZ());
    if (theBox.IsOut (gp_Ax1(thePnt[0], aDir), Standard_True) ||
        theBox.IsOut (gp_Ax1(thePnt[1],-aDir), Standard_True))
    aResult = 0;
  }
  return aResult;
}

//=======================================================================
//function : seg_box_included
//purpose  : 
//=======================================================================

int NIS_Triangulated::seg_box_included  (const Bnd_B3f& theBox,
                                         const gp_Pnt thePnt[2])
{
  int aResult (0);
  if ((thePnt[1].XYZ() - thePnt[0].XYZ()).SquareModulus()
      > Precision::Confusion() * 0.0001)
  {
    aResult = (theBox.IsOut(thePnt[0].XYZ()) == Standard_False &&
               theBox.IsOut(thePnt[1].XYZ()) == Standard_False);
  }
  return aResult;
}

//=======================================================================
//function : seg_polygon_intersect
//purpose  : 
//=======================================================================

int NIS_Triangulated::seg_polygon_intersect
                      (const NCollection_List<gp_XY> &thePolygon,
                       const gp_XY                    thePnt[2])
{
  Standard_Integer aResult = 0;

  if (thePolygon.IsEmpty())
    return aResult;

  gp_XY            aDir(thePnt[1] - thePnt[0]);
  Standard_Real    aDist   = aDir.SquareModulus();

  if (aDist > aTolConf) {
    aDist = Sqrt(aDist);
    aDir.Divide(aDist); // Normalize direction.

    // Intersect the line passed through thePnt[0] and thePnt[1] with thePolygon
    // Line is presented in form Ax + By + C = 0
    Standard_Real aA =  aDir.Y();
    Standard_Real aB = -aDir.X();
    Standard_Real aC = -(aA*thePnt[0].X() + aB*thePnt[0].Y());
    gp_XY         aSegment[2];
    Standard_Real aSignedD[2];
    Standard_Real aDelta = 0.01;

    aSegment[0] = thePolygon.Last();
    aSignedD[0] = aA*aSegment[0].X() + aB*aSegment[0].Y() + aC;

    NCollection_List<gp_XY>::Iterator anIter(thePolygon);

    for (; anIter.More(); anIter.Next()) {
      aSegment[1] = anIter.Value();
      aSignedD[1] = aA*aSegment[1].X() + aB*aSegment[1].Y() + aC;

      // Check if there is an intersection.
      if (Abs(aSignedD[0]) <= aTolConf || Abs(aSignedD[1]) <= aTolConf) {
        Standard_Integer anIndexVtx = 
                              (Abs(aSignedD[0]) > aTolConf) ? 1 :
                              (Abs(aSignedD[1]) > aTolConf) ? 0 : -1;
        if (anIndexVtx != -1) {
          // Check if the point aSegment[1] is inside the segment.
          gp_XY         aDP(aSegment[anIndexVtx] - thePnt[0]);
          Standard_Real aParam = aDP.Dot(aDir);

          if (aParam >= -aTolConf && aParam <= aDist + aTolConf) {
            // Classify a point on the line that is close to aSegment[1] with
            // respect to the polygon.
            gp_XY aPShift;

            if (aParam - aDelta >= 0.) {
              aPShift = thePnt[0] + aDir.Multiplied(aParam - aDelta);
              if (!IsIn(thePolygon, aPShift))
                aResult = 1;
            }

            // Try to shift on another direction.
            if (!aResult) {
              if (aParam + aDelta <= aDist) {
                aPShift = thePnt[0] + aDir.Multiplied(aParam + aDelta);
                if (!IsIn(thePolygon, aPShift))
                  aResult = 1;
              }
            }
          }
        }
      } else if (aSignedD[0]*aSignedD[1] < 0.) {
        // Compute intersection of the segment with the line.
        gp_XY         aDSeg(aSegment[1] - aSegment[0]);
        Standard_Real aSegLen     = aDSeg.Modulus();
        Standard_Real aParamOnSeg = aSegLen/(1 + Abs(aSignedD[1]/aSignedD[0]));
        gp_XY         aPOnLine
                         (aSegment[0] + aDSeg.Multiplied(aParamOnSeg/aSegLen));

        // Check if aPOnLine inside the segment thePnt[1] - thePnt[0]
        gp_XY         aDP(aPOnLine - thePnt[0]);
        Standard_Real aParam = aDP.Dot(aDir);

        if (aParam >= -aTolConf && aParam <= aDist + aTolConf) {
          gp_XY aPShift;

          if (aParam - aDelta >= 0.) {
            aPShift = thePnt[0] + aDir.Multiplied(aParam - aDelta);

            if (!IsIn(thePolygon, aPShift))
              aResult = 1;
          }

          // Try to shift on another direction.
          if (!aResult) {
            if (aParam + aDelta <= aDist) {
              aPShift = thePnt[0] + aDir.Multiplied(aParam + aDelta);

              if (!IsIn(thePolygon, aPShift))
                aResult = 1;
            }
          }
        }
      }

      if (aResult != 0)
        break;

      aSegment[0] = aSegment[1];
      aSignedD[0] = aSignedD[1];
    }
  }

  return aResult;
}

//=======================================================================
//function : seg_polygon_included
//purpose  : 
//=======================================================================

int NIS_Triangulated::seg_polygon_included
                      (const NCollection_List<gp_XY> &thePolygon,
                       const gp_XY                    thePnt[2])
{
  int aResult     = 0;
  int anIntersect = seg_polygon_intersect(thePolygon, thePnt);

  if (anIntersect == 0) {
    if (IsIn(thePolygon, thePnt[0]) && IsIn(thePolygon, thePnt[1]))
      aResult = 1;
  }

  return aResult;
}

//=======================================================================
//function : IsIn
//purpose  : 
//=======================================================================

Standard_Boolean NIS_Triangulated::IsIn
                      (const NCollection_List<gp_XY> &thePolygon,
                       const gp_XY                   &thePoint)
{
  if (thePolygon.IsEmpty())
    return Standard_False;

  Standard_Integer aCounter = 0; // intersections counter
  gp_XY            aSegment[2];

  aSegment[0] = thePolygon.Last();

  NCollection_List<gp_XY>::Iterator anIter(thePolygon);

  for (; anIter.More(); anIter.Next()) {
    aSegment[1] = anIter.Value();

    // Compute projection of the point onto the segment.
    Standard_Real       aParam = 0.;
    const gp_XY         aDelta = aSegment[1] - aSegment[0];
    const gp_XY         aDPP0  = thePoint - aSegment[0];
    const Standard_Real aLen2  = aDelta.SquareModulus();

    if (IsPositive(aLen2)) {
      aParam = (aDelta*aDPP0)/aLen2;

      if (aParam < 0.)
        aParam = 0.;
      else if (aParam > 1.)
        aParam = 1.;
    }
    // Check if the point lies on the segment
    gp_XY         aPOnSeg  = aSegment[0]*(1. - aParam) + aSegment[1]*aParam;
    Standard_Real aSqrDist = (thePoint - aPOnSeg).SquareModulus();

    if (aSqrDist < aTolConf) {
      // The point is on the contour.
      return Standard_True;
    }

    // Compute intersection.
    const Standard_Real aProd(aDPP0 ^ aDelta);

    if (IsPositive(thePoint.X() - aSegment[0].X())) {
      if (!IsPositive(thePoint.X() - aSegment[1].X())) {
        if (aProd > 0.)
          aCounter++;
      }
    } else {
      if (IsPositive(thePoint.X() - aSegment[1].X())) {
        if (aProd < 0.)
          aCounter++;
      }
    }

    aSegment[0] = aSegment[1];
  }

  return (aCounter & 0x1);
}

//=======================================================================
//function : allocateNodes
//purpose  : 
//=======================================================================

void NIS_Triangulated::allocateNodes (const Standard_Integer nNodes)
{
  if (nNodes > 0) {
    if (myNNodes > 0)
      myAlloc->Free(mypNodes);
    myNNodes = nNodes;
    mypNodes = static_cast<Standard_ShortReal*>
      (myAlloc->Allocate(sizeof(Standard_ShortReal) * myNodeCoord * nNodes));

    if (nNodes < 256)
      myIndexType = 0;
    else if (nNodes < 65536)
      myIndexType = 1;
    else
      myIndexType = 2;    
  }
}

//=======================================================================
//function : NodeAtInd
//purpose  : Get the node pointed by the i-th index in the array.
//=======================================================================

gp_Pnt NIS_Triangulated::nodeAtInd  (const Standard_Integer * theArray,
                                     const Standard_Integer theInd) const
{
  if (myIndexType == 0) {
    const unsigned char * pInd =
      reinterpret_cast<const unsigned char *>(theArray);
    return gp_Pnt (mypNodes[myNodeCoord * pInd[theInd] + 0],
                   mypNodes[myNodeCoord * pInd[theInd] + 1],
                   myNodeCoord < 3 ? 0. :
                   mypNodes[myNodeCoord * pInd[theInd] + 2]);
  }
  if (myIndexType == 1) {
    const unsigned short * pInd =
      reinterpret_cast<const unsigned short *>(theArray);
    return gp_Pnt (mypNodes[myNodeCoord * pInd[theInd] + 0],
                   mypNodes[myNodeCoord * pInd[theInd] + 1],
                   myNodeCoord < 3 ? 0. :
                   mypNodes[myNodeCoord * pInd[theInd] + 2]);
  }
  return gp_Pnt (mypNodes[myNodeCoord * theArray[theInd] + 0],
                 mypNodes[myNodeCoord * theArray[theInd] + 1],
                 myNodeCoord < 3 ? 0. :
                 mypNodes[myNodeCoord * theArray[theInd] + 2]);
}
  
//=======================================================================
//function : nodeArrAtInd
//purpose  : Get the node pointed by the i-th index in the array.
//=======================================================================

float* NIS_Triangulated::nodeArrAtInd (const Standard_Integer * theArray,
                                       const Standard_Integer theInd) const
{
  float* pResult = 0L; 
  if (myIndexType == 0) {
    const unsigned char * pInd =
      reinterpret_cast<const unsigned char *>(theArray);
    pResult = &mypNodes[myNodeCoord * pInd[theInd]];
  }
  else if (myIndexType == 1) {
    const unsigned short * pInd =
      reinterpret_cast<const unsigned short *>(theArray);
    pResult = &mypNodes[myNodeCoord * pInd[theInd]];
  }
  else {
    pResult = &mypNodes[myNodeCoord * theArray[theInd]];
  }
  return pResult;
}

