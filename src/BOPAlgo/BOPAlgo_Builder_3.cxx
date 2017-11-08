// Created by: Peter KURNEV
// Copyright (c) 2010-2014 OPEN CASCADE SAS
// Copyright (c) 2007-2010 CEA/DEN, EDF R&D, OPEN CASCADE
// Copyright (c) 2003-2007 OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN, CEDRAT,
//                         EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
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
//
#include <BOPAlgo_Builder.hxx>
//
#include <Precision.hxx>
//
#include <NCollection_UBTreeFiller.hxx>
//
#include <Bnd_Box.hxx>
#include <TopAbs_State.hxx>
//
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Solid.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Compound.hxx>
//
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
//
#include <BRep_Builder.hxx>
#include <BRepTools.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepBndLib.hxx>
//
#include <BOPCol_IndexedMapOfShape.hxx>
#include <BOPCol_MapOfShape.hxx>
#include <BOPCol_IndexedDataMapOfShapeListOfShape.hxx>
#include <BOPCol_ListOfShape.hxx>
#include <BOPCol_BoxBndTree.hxx>
#include <BOPCol_ListOfInteger.hxx>
#include <BOPCol_DataMapOfIntegerShape.hxx>
#include <BOPCol_NCVector.hxx>
#include <BOPCol_Parallel.hxx>
//
#include <IntTools_Context.hxx>
//
#include <BOPDS_DS.hxx>
#include <BOPDS_ShapeInfo.hxx>
//
#include <BOPTools.hxx>
#include <BOPTools_AlgoTools.hxx>
#include <BOPTools_MapOfSet.hxx>
#include <BOPTools_Set.hxx>
//
#include <BOPAlgo_BuilderSolid.hxx>
#include <NCollection_Array1.hxx>
#include <NCollection_IncAllocator.hxx>

#include <algorithm>
#include <BOPAlgo_Algo.hxx>

static
  void OwnInternalShapes(const TopoDS_Shape& ,
                         BOPCol_IndexedMapOfShape& );

static
  void TreatCompound(const TopoDS_Shape& theS,
                     BOPCol_MapOfShape& aMFence,
                     BOPCol_ListOfShape& theLS);

//=======================================================================
// BOPAlgo_BuilderSolid
//
typedef BOPCol_NCVector
  <BOPAlgo_BuilderSolid> BOPAlgo_VectorOfBuilderSolid;
//
typedef BOPCol_Functor 
  <BOPAlgo_BuilderSolid,
  BOPAlgo_VectorOfBuilderSolid> BOPAlgo_BuilderSolidFunctor;
//
typedef BOPCol_Cnt 
  <BOPAlgo_BuilderSolidFunctor,
  BOPAlgo_VectorOfBuilderSolid> BOPAlgo_BuilderSolidCnt;
//
//=======================================================================
// class:  BOPAlgo_ShapeBox
//
//=======================================================================
//class     : BOPAlgo_ShapeBox
//purpose   : Auxiliary class
//=======================================================================
class BOPAlgo_ShapeBox {
 public:
  BOPAlgo_ShapeBox() {
  };
  //
  ~BOPAlgo_ShapeBox() {
  };
  //
  void SetShape(const TopoDS_Shape& aS) {
    myShape=aS;
  };
  //
  const TopoDS_Shape& Shape()const {
    return myShape;
  };
  //
  void SetBox(const Bnd_Box& aBox) {
    myBox=aBox;
  };
  //
  const Bnd_Box& Box()const {
    return myBox;
  };
  //
 protected:
  TopoDS_Shape myShape;
  Bnd_Box myBox;
};
//
typedef BOPCol_NCVector<BOPAlgo_ShapeBox> BOPAlgo_VectorOfShapeBox;
//
//=======================================================================
// class:  BOPAlgo_FillIn3DParts
//
//=======================================================================
//class : BOPAlgo_FillIn3DParts
//purpose  : 
//=======================================================================
class BOPAlgo_FillIn3DParts : public BOPAlgo_Algo  {
 public:
  DEFINE_STANDARD_ALLOC
  
  BOPAlgo_FillIn3DParts(){
    myHasImage=Standard_False;
    myBBTree=NULL;
    myVSB=NULL;
  };
  //
  virtual ~BOPAlgo_FillIn3DParts(){
  };
  //
  void SetSolid(const TopoDS_Solid& aS) {
    mySolid=aS;
  };
  //
  const TopoDS_Solid& Solid()const {
    return mySolid;
  };
  //
  void SetDraftSolid(const TopoDS_Solid& aS) {
    myDraftSolid=aS;
  };
  //
  const TopoDS_Solid& DraftSolid()const {
    return myDraftSolid;
  };
  //
  void SetHasImage(const Standard_Boolean bFlag) {
    myHasImage=bFlag;
  };
  //
  Standard_Boolean HasImage()const {
    return myHasImage;
  };
  //
  void SetBoxS(const Bnd_Box& aBox) {
    myBoxS=aBox;
  };
  //
  const Bnd_Box& BoxS()const {
    return myBoxS;
  };
  //
  void SetLIF(const BOPCol_ListOfShape& aLIF) {
    myLIF=aLIF;
  };
  //
  const BOPCol_ListOfShape& LIF()const {
    return myLIF;
  };
  //
  void SetBBTree(const BOPCol_BoxBndTree& aBBTree) {
    myBBTree=(BOPCol_BoxBndTree*)&aBBTree;
  };
  //
  void SetVSB(const BOPAlgo_VectorOfShapeBox& aVSB) {
    myVSB=(BOPAlgo_VectorOfShapeBox*)&aVSB;
  };
  //
  //
  void SetContext(const Handle(IntTools_Context)& aContext) {
    myContext=aContext;
  }
  //
  const Handle(IntTools_Context)& Context()const {
    return myContext;
  }
  //
  virtual void Perform();
  //
  
  //
  const BOPCol_ListOfShape& LFIN()const {
    return myLFIN;
  };
  
 protected:
  void MapEdgesAndFaces
    (const TopoDS_Shape& ,
     BOPCol_IndexedDataMapOfShapeListOfShape& ,
     const Handle(NCollection_BaseAllocator)& );
  
  void MakeConnexityBlock 
    (const TopoDS_Face& ,
     const BOPCol_IndexedMapOfShape& ,
     const BOPCol_IndexedDataMapOfShapeListOfShape& ,
     BOPCol_MapOfShape& ,
     BOPCol_ListOfShape& );
  //
 protected:
  TopoDS_Solid mySolid;
  TopoDS_Solid myDraftSolid;
  Standard_Boolean myHasImage;
  Bnd_Box myBoxS;
  BOPCol_ListOfShape myLIF;
  BOPCol_ListOfShape myLFIN;
  //
  BOPCol_BoxBndTree* myBBTree;
  BOPAlgo_VectorOfShapeBox* myVSB;
  //
  TopoDS_Iterator myItF;
  TopoDS_Iterator myItW;
  
  Handle(IntTools_Context) myContext;
};

//=======================================================================
//function : BOPAlgo_FillIn3DParts::Perform
//purpose  : 
//=======================================================================
void BOPAlgo_FillIn3DParts::Perform() 
{
  BOPAlgo_Algo::UserBreak();

  myLFIN.Clear();

  Handle(NCollection_BaseAllocator) aAlr1 = new NCollection_IncAllocator;

  BOPAlgo_VectorOfShapeBox& aVSB = *myVSB;

  // 1. Fill maps of edges and faces of myDraftSolid
  BOPCol_IndexedMapOfShape aME(1, aAlr1), aMF(1, aAlr1);
  BOPTools::MapShapes(myDraftSolid, TopAbs_EDGE, aME);
  BOPTools::MapShapes(myDraftSolid, TopAbs_FACE, aMF);
  // Check if the Draft Solid contains any faces
  Standard_Boolean bIsEmpty = aMF.IsEmpty();
  // Add own internal faces of myDraftSolid into aMF
  BOPCol_ListIteratorOfListOfShape aItLS(myLIF);
  for (; aItLS.More(); aItLS.Next())
    aMF.Add(aItLS.Value());

  // 2. Select boxes of faces that are not out of aBoxS
  BOPCol_BoxBndTreeSelector aSelector;
  aSelector.SetBox(myBoxS);
  //
  myBBTree->Select(aSelector);
  const BOPCol_ListOfInteger& aLIFP = aSelector.Indices();
  //
  // 3. aIVec - faces to process.
  //    Filter the selected faces with faces of the solid.
  BOPCol_NCVector<Standard_Integer> aIVec(256, aAlr1);

  BOPCol_ListIteratorOfListOfInteger aItLI(aLIFP);
  for (; aItLI.More(); aItLI.Next()) {
    Standard_Integer nFP = aItLI.Value();
    const TopoDS_Shape& aFP = aVSB(nFP).Shape();
    if (!aMF.Contains(aFP))
      aIVec.Append1() = nFP;
  }

  // 4. Classify faces relatively solid.
  //    Store faces that are IN mySolid into <myLFIN>

  Standard_Integer k, aNbFP = aIVec.Extent();
  // Sort indices if necessary
  if (aNbFP > 1)
    std::sort(aIVec.begin(), aIVec.end());

  if (bIsEmpty)
  {
    // The Draft solid is empty as it does not contain any faces.
    // It could happen when the input solid consists of INTERNAL faces only.
    // Classification of any point relatively empty solid would always give IN status.
    // Thus, we consider all selected faces as IN without real classification.
    for (k = 0; k < aNbFP; ++k)
      myLFIN.Append(aVSB(aIVec(k)).Shape());

    return;
  }

  // Prepare EF map of faces to process for building connexity blocks
  BOPCol_IndexedDataMapOfShapeListOfShape aMEFP(1, aAlr1);
  if (aNbFP > 1)
  {
    for (k = 0; k < aNbFP; ++k)
      MapEdgesAndFaces(aVSB(aIVec(k)).Shape(), aMEFP, aAlr1);
  }

  // Map of Edge-Face connection, necessary for solid classification.
  // It will be filled when first classification is performed.
  BOPCol_IndexedDataMapOfShapeListOfShape aMEFDS(1, aAlr1);

  // Fence map to avoid processing of the same faces twice
  BOPCol_MapOfShape aMFDone(1, aAlr1);

  for (k = 0; k < aNbFP; ++k)
  {
    Standard_Integer nFP = aIVec(k);
    const TopoDS_Face& aFP = (*(TopoDS_Face*)&aVSB(nFP).Shape());
    if (!aMFDone.Add(aFP))
      continue;

    // Make connexity blocks of faces, avoiding passing through the
    // borders of the solid. It helps to reduce significantly the
    // number of classified faces.
    BOPCol_ListOfShape aLCBF(aAlr1);
    MakeConnexityBlock(aFP, aME, aMEFP, aMFDone, aLCBF);

    // First, try fast classification of the whole block by additional
    // check on bounding boxes - check that bounding boxes of all vertices
    // of the block interfere with the box of the solid.
    // If not, the faces are out.
    Standard_Boolean bOut = Standard_False;
    aItLS.Initialize(aLCBF);
    for (; aItLS.More() && !bOut; aItLS.Next())
    {
      TopExp_Explorer anExpV(aItLS.Value(), TopAbs_VERTEX);
      for (; anExpV.More() && !bOut; anExpV.Next())
      {
        const TopoDS_Vertex& aV = TopoDS::Vertex(anExpV.Current());
        Bnd_Box aBBV;
        aBBV.Add(BRep_Tool::Pnt(aV));
        aBBV.SetGap(BRep_Tool::Tolerance(aV));
        bOut = myBoxS.IsOut(aBBV);
      }
    }

    if (bOut)
      continue;

    if (aMEFDS.IsEmpty())
      // Fill EF map for myDraftSolid
      BOPTools::MapShapesAndAncestors(myDraftSolid, TopAbs_EDGE, TopAbs_FACE, aMEFDS);

    // All vertices are interfere with the solids box, run classification.
    Standard_Boolean bIsIN = BOPTools_AlgoTools::IsInternalFace
      (aFP, myDraftSolid, aMEFDS, Precision::Confusion(), myContext);
    if (bIsIN)
    {
      aItLS.Initialize(aLCBF);
      for (; aItLS.More(); aItLS.Next())
        myLFIN.Append(aItLS.Value());
    }
  }
}
//=======================================================================
// function: MapEdgesAndFaces
// purpose: 
//=======================================================================
void BOPAlgo_FillIn3DParts::MapEdgesAndFaces
  (const TopoDS_Shape& aF,
   BOPCol_IndexedDataMapOfShapeListOfShape& aMEF,
   const Handle(NCollection_BaseAllocator)& theAllocator)
{
  myItF.Initialize(aF);
  for (; myItF.More(); myItF.Next()) {
    const TopoDS_Shape& aW=myItF.Value();
    if (aW.ShapeType()!=TopAbs_WIRE) {
      continue;
    }
    //
    myItW.Initialize(aW);
    for (; myItW.More(); myItW.Next()) {
      const TopoDS_Shape& aE=myItW.Value();
      //
      BOPCol_ListOfShape* pLF = aMEF.ChangeSeek(aE);
      if (!pLF)
        pLF = &aMEF(aMEF.Add(aE, BOPCol_ListOfShape(theAllocator)));
      pLF->Append(aF);
    }
  }
}
//=======================================================================
// function: MakeConnexityBlock
// purpose: 
//=======================================================================
void BOPAlgo_FillIn3DParts::MakeConnexityBlock
  (const TopoDS_Face& theFStart,
   const BOPCol_IndexedMapOfShape& theMEAvoid,
   const BOPCol_IndexedDataMapOfShapeListOfShape& theMEF,
   BOPCol_MapOfShape& theMFDone,
   BOPCol_ListOfShape& theLCB)
{
  // Add start element
  theLCB.Append(theFStart);
  if (theMEF.IsEmpty())
    return;

  BOPCol_ListIteratorOfListOfShape aItCB(theLCB);
  for (; aItCB.More(); aItCB.Next())
  {
    const TopoDS_Shape& aF = aItCB.Value();
    myItF.Initialize(aF);
    for (; myItF.More(); myItF.Next())
    {
      const TopoDS_Shape& aW = myItF.Value();
      if (aW.ShapeType() != TopAbs_WIRE)
        continue;

      myItW.Initialize(aW);
      for (; myItW.More(); myItW.Next())
      {
        const TopoDS_Shape& aE = myItW.Value();
        if (theMEAvoid.Contains(aE))
          continue;

        const BOPCol_ListOfShape* pLF = theMEF.Seek(aE);
        if (!pLF)
          continue;
        BOPCol_ListIteratorOfListOfShape aItLF(*pLF);
        for (; aItLF.More(); aItLF.Next())
        {
          const TopoDS_Shape& aFx = aItLF.Value();
          if (!aFx.IsSame(aF) && theMFDone.Add(aFx))
            theLCB.Append(aFx);
        }
      }
    }
  }
}
//
typedef BOPCol_NCVector<BOPAlgo_FillIn3DParts> \
  BOPAlgo_VectorOfFillIn3DParts;
//
typedef BOPCol_ContextFunctor 
  <BOPAlgo_FillIn3DParts,
  BOPAlgo_VectorOfFillIn3DParts,
  Handle(IntTools_Context), 
  IntTools_Context> BOPCol_FillIn3DPartsFunctor;
//
typedef BOPCol_ContextCnt 
  <BOPCol_FillIn3DPartsFunctor,
  BOPAlgo_VectorOfFillIn3DParts,
  Handle(IntTools_Context)> BOPAlgo_FillIn3DPartsCnt;
//
//=======================================================================
// class:  BOPAlgo_Builder
//
//=======================================================================
//function : FillImagesSolids
//purpose  : 
//=======================================================================
void BOPAlgo_Builder::FillImagesSolids()
{
  Standard_Boolean bHasSolids;
  Standard_Integer i, aNbS;
  //
  bHasSolids=Standard_False;
  aNbS=myDS->NbSourceShapes();
  for (i=0; i<aNbS; ++i) {
    const BOPDS_ShapeInfo& aSI=myDS->ShapeInfo(i);
    if (aSI.ShapeType()==TopAbs_SOLID) {
      bHasSolids=!bHasSolids;
      break;
    }
  }
  //
  if (!bHasSolids) {
    return;
  }
  // 
  Handle(NCollection_BaseAllocator) aAlr;
  //
  aAlr=NCollection_BaseAllocator::CommonBaseAllocator();
  //
  BOPCol_DataMapOfShapeListOfShape theInParts(100, aAlr);
  BOPCol_DataMapOfShapeShape theDraftSolids(100, aAlr);
  //
  FillIn3DParts(theInParts, theDraftSolids, aAlr); 
  BuildSplitSolids(theInParts, theDraftSolids, aAlr);
  FillInternalShapes();
  //
  theInParts.Clear();
  theDraftSolids.Clear();
}
//=======================================================================
//function : FillIn3DParts
//purpose  : 
//=======================================================================
void BOPAlgo_Builder::FillIn3DParts
  (BOPCol_DataMapOfShapeListOfShape& theInParts,
   BOPCol_DataMapOfShapeShape& theDraftSolids,
   const BOPCol_BaseAllocator& )
{
  Standard_Boolean bHasImage;
  Standard_Integer i, k, aNbS, aNbLIF, aNbFIN, aNbVSB, aNbVFIP;
  Handle(NCollection_BaseAllocator) aAlr0;
  TopoDS_Solid aSD;
  TopoDS_Iterator aIt;
  BRep_Builder aBB; 
  //
  BOPCol_ListIteratorOfListOfInteger aItLI, aItLI1;
  BOPCol_ListIteratorOfListOfShape aItLS;
  //
  aAlr0=
    NCollection_BaseAllocator::CommonBaseAllocator();
  //
  BOPCol_MapOfShape aMFence(100, aAlr0);
  BOPAlgo_VectorOfShapeBox aVSB(256, aAlr0);
  //
  theDraftSolids.Clear();
  //
  // 1. aVSB vector Index/FaceBox 
  aNbS=myDS->NbSourceShapes();
  for (i=0; i<aNbS; ++i) {
    const BOPDS_ShapeInfo& aSI=myDS->ShapeInfo(i);
    if (aSI.ShapeType()!=TopAbs_FACE) {
      continue;
    }
    //
    const TopoDS_Shape& aS=aSI.Shape();
    //
    if (myImages.IsBound(aS)) {
      const BOPCol_ListOfShape& aLS=myImages.Find(aS);
      aItLS.Initialize(aLS);
      for (; aItLS.More(); aItLS.Next()) {
        const TopoDS_Shape& aSx=aItLS.Value();
        if (!aMFence.Add(aSx)) {
          continue;
        }
        Bnd_Box aBox;
        BRepBndLib::Add(aSx, aBox);
        aBox.SetGap(aBox.GetGap() + Precision::Confusion());
        //
        BOPAlgo_ShapeBox& aSB=aVSB.Append1();
        aSB.SetShape(aSx);
        aSB.SetBox(aBox);
      }
    }
    else {
      const Bnd_Box& aBox=aSI.Box();
      //
      BOPAlgo_ShapeBox& aSB=aVSB.Append1();
      aSB.SetShape(aS);
      aSB.SetBox(aBox);
    }
  }//for (i=0; i<aNbS; ++i) {
  aMFence.Clear();
  //
  // 1.2. Prepare TreeFiller
  BOPCol_BoxBndTree aBBTree;
  NCollection_UBTreeFiller <Standard_Integer, Bnd_Box> 
    aTreeFiller(aBBTree);
  //
  aNbVSB=aVSB.Extent();
  for (k=0; k<aNbVSB; ++k) {
    const BOPAlgo_ShapeBox& aSBk=aVSB(k);
    const Bnd_Box& aBk=aSBk.Box();
    //
    aTreeFiller.Add(k, aBk);
  }
  //
  // 1.3. Shake TreeFiller
  aTreeFiller.Fill();
  //
  //---------------------------------------------
  // 2. Solids
  BOPAlgo_VectorOfFillIn3DParts aVFIP;
  //
  for (i=0; i<aNbS; ++i) {
    BOPDS_ShapeInfo& aSI=myDS->ChangeShapeInfo(i);
    if (aSI.ShapeType()!=TopAbs_SOLID) {
      continue;
    }
    //
    const TopoDS_Shape& aS=aSI.Shape();
    const TopoDS_Solid& aSolid=(*(TopoDS_Solid*)(&aS));
    //
    // 2.0 Flag bHasImage
    bHasImage=Standard_False;
    aIt.Initialize(aS);
    for (; aIt.More(); aIt.Next()) {
      const TopoDS_Shape& aShell=aIt.Value();
      bHasImage=myImages.IsBound(aShell);
      if (bHasImage){
        break;
      }
    }
    //
    // 2.1 Bounding box for the solid aS  [ aBoxS ]
    Bnd_Box& aBoxS = aSI.ChangeBox();
    if (aBoxS.IsVoid())
      myDS->BuildBndBoxSolid(i, aBoxS, myCheckInverted);
    //
    // 2.2 Build Draft Solid [aSD]
    BOPCol_ListOfShape aLIF;
    //
    aBB.MakeSolid(aSD);
    BuildDraftSolid(aSolid, aSD, aLIF);
    //
    BOPAlgo_FillIn3DParts& aFIP=aVFIP.Append1();
    //
    aFIP.SetSolid(aSolid);
    aFIP.SetDraftSolid(aSD);
    aFIP.SetHasImage(bHasImage);
    aFIP.SetBoxS(aBoxS);
    aFIP.SetLIF(aLIF);
    aFIP.SetBBTree(aBBTree);
    aFIP.SetVSB(aVSB);
  }//for (i=0; i<aNbS; ++i) {
  //
  aNbVFIP=aVFIP.Extent();
  //================================================================
  BOPAlgo_FillIn3DPartsCnt::Perform(myRunParallel, aVFIP, myContext);
  //================================================================
  for (k=0; k<aNbVFIP; ++k) {
    BOPAlgo_FillIn3DParts& aFIP=aVFIP(k);
    bHasImage=aFIP.HasImage();
    const TopoDS_Solid& aSolid=aFIP.Solid();
    const TopoDS_Solid& aSDraft =aFIP.DraftSolid();
    const BOPCol_ListOfShape& aLFIN=aFIP.LFIN();
    const BOPCol_ListOfShape& aLIF=aFIP.LIF();
    //
    aNbLIF=aLIF.Extent();
    //
    // Store the results in theInParts, theDraftSolids
    BOPCol_ListOfShape aLFINx;
    //
    aNbFIN=aLFIN.Extent();
    if (aNbFIN || aNbLIF) {
      aItLS.Initialize(aLFIN);
      for (; aItLS.More(); aItLS.Next()) {
        const TopoDS_Shape& aFI=aItLS.Value();
        aLFINx.Append(aFI);
      }
      aItLS.Initialize(aLIF);
      for (; aItLS.More(); aItLS.Next()) {
        const TopoDS_Shape& aFI=aItLS.Value();
        aLFINx.Append(aFI);
      }
      theInParts.Bind(aSolid, aLFINx);
    }
    //
    if (aNbFIN || bHasImage) {
      theDraftSolids.Bind(aSolid, aSDraft);
    }
  }
}
//=======================================================================
//function : BuildDraftSolid
//purpose  : 
//=======================================================================
void BOPAlgo_Builder::BuildDraftSolid(const TopoDS_Shape& theSolid,
                                      TopoDS_Shape& theDraftSolid,
                                      BOPCol_ListOfShape& theLIF)
{
  Standard_Boolean bToReverse;
  Standard_Integer iFlag;
  TopAbs_Orientation aOrF, aOrSh, aOrSd;
  TopoDS_Iterator aIt1, aIt2;
  TopoDS_Shell aShD;
  TopoDS_Shape aFSDx, aFx;
  BRep_Builder aBB;
  BOPCol_ListIteratorOfListOfShape aItS; 
  //
  aOrSd=theSolid.Orientation();
  theDraftSolid.Orientation(aOrSd);
  //
  aIt1.Initialize(theSolid);
  for (; aIt1.More(); aIt1.Next()) {
    const TopoDS_Shape& aSh=aIt1.Value();
    if(aSh.ShapeType()!=TopAbs_SHELL) {
      continue; // mb internal edges,vertices
    }
    //
    aOrSh=aSh.Orientation();
    aBB.MakeShell(aShD);
    aShD.Orientation(aOrSh);
    iFlag=0;
    //
    aIt2.Initialize(aSh);
    for (; aIt2.More(); aIt2.Next()) {
      const TopoDS_Shape& aF=aIt2.Value();
      aOrF=aF.Orientation();
      //
      if (myImages.IsBound(aF)) {
        const BOPCol_ListOfShape& aLSp=myImages.Find(aF);
        aItS.Initialize(aLSp);
        for (; aItS.More(); aItS.Next()) {
          aFx=aItS.Value();
          //
          if (myShapesSD.IsBound(aFx)) {
            aFSDx=myShapesSD.Find(aFx);
            //
            if (aOrF==TopAbs_INTERNAL) {
              aFSDx.Orientation(aOrF);
              theLIF.Append(aFSDx);
            }
            else {
              bToReverse=BOPTools_AlgoTools::IsSplitToReverse
                (aFSDx, aF, myContext); 
              if (bToReverse) {
                aFSDx.Reverse();
              }
              //
              iFlag=1;
              aBB.Add(aShD, aFSDx);
            }
          }//if (myShapesSD.IsBound(aFx)) {
          else {
            aFx.Orientation(aOrF);
            if (aOrF==TopAbs_INTERNAL) {
              theLIF.Append(aFx);
            }
            else{
              iFlag=1;
              aBB.Add(aShD, aFx);
            }
          }
        }
      } // if (myImages.IsBound(aF)) { 
      //
      else {
        if (aOrF==TopAbs_INTERNAL) {
          theLIF.Append(aF);
        }
        else{
          iFlag=1;
          aBB.Add(aShD, aF);
        }
      }
    } //for (; aIt2.More(); aIt2.Next()) {
    //
    if (iFlag) {
      aShD.Closed (BRep_Tool::IsClosed (aShD));
      aBB.Add(theDraftSolid, aShD);
    }
  } //for (; aIt1.More(); aIt1.Next()) {
}
//=======================================================================
//function : BuildSplitSolids
//purpose  : 
//=======================================================================
void BOPAlgo_Builder::BuildSplitSolids
  (BOPCol_DataMapOfShapeListOfShape& theInParts,
   BOPCol_DataMapOfShapeShape& theDraftSolids,
   const BOPCol_BaseAllocator&  )
{
  Standard_Boolean bFlagSD;
  Standard_Integer i, aNbS;
  TopExp_Explorer aExp;
  BOPCol_ListIteratorOfListOfShape aIt;
  //
  Handle(NCollection_BaseAllocator) aAlr0;
  aAlr0=NCollection_BaseAllocator::CommonBaseAllocator();
  //
  BOPCol_ListOfShape aSFS(aAlr0), aLSEmpty(aAlr0);
  BOPCol_MapOfShape aMFence(100, aAlr0);
  BOPTools_MapOfSet aMST(100, aAlr0);
  BOPAlgo_VectorOfBuilderSolid aVBS;
  //
  // 0. Find same domain solids for non-interferred solids
  aNbS=myDS->NbSourceShapes();
  for (i=0; i<aNbS; ++i) {
    const BOPDS_ShapeInfo& aSI=myDS->ShapeInfo(i);
    //
    if (aSI.ShapeType()!=TopAbs_SOLID) {
      continue;
    }
    //
    const TopoDS_Shape& aS=aSI.Shape();
    if (!aMFence.Add(aS)) {
      continue;
    }
    if(theDraftSolids.IsBound(aS)) {
      continue;
    }
    //
    BOPTools_Set aST;
    //
    aST.Add(aS, TopAbs_FACE);
    aMST.Add(aST);
    //
  } //for (i=1; i<=aNbS; ++i) 
  //
  // Build temporary map of solids images to avoid rebuilding
  // of the solids without internal faces
  BOPCol_IndexedDataMapOfShapeListOfShape aSolidsIm;
  // 1. Build solids for interfered source solids
  for (i = 0; i < aNbS; ++i) {
    const BOPDS_ShapeInfo& aSI = myDS->ShapeInfo(i);
    if (aSI.ShapeType() != TopAbs_SOLID)
      continue;

    const TopoDS_Shape& aS = aSI.Shape();
    const TopoDS_Solid& aSolid=(*(TopoDS_Solid*)(&aS));
    if (!theDraftSolids.IsBound(aS))
      continue;

    const TopoDS_Shape& aSD = theDraftSolids.Find(aS);
    const BOPCol_ListOfShape* pLFIN = theInParts.Seek(aS);
    if (!pLFIN)
    {
      aSolidsIm(aSolidsIm.Add(aS, BOPCol_ListOfShape())).Append(aSD);
      continue;
    }

    aSFS.Clear();
    //
    // 1.1 Fill Shell Faces Set
    aExp.Init(aSD, TopAbs_FACE);
    for (; aExp.More(); aExp.Next()) {
      const TopoDS_Shape& aF = aExp.Current();
      aSFS.Append(aF);
    }
    //
    // 1.2 Fill internal faces
    aIt.Initialize(*pLFIN);
    for (; aIt.More(); aIt.Next()) {
      TopoDS_Shape aF = aIt.Value();
      //
      aF.Orientation(TopAbs_FORWARD);
      aSFS.Append(aF);
      aF.Orientation(TopAbs_REVERSED);
      aSFS.Append(aF);
    }
    //
    // 1.3 Build new solids
    BOPAlgo_BuilderSolid& aBS=aVBS.Append1();
    aBS.SetSolid(aSolid);
    aBS.SetShapes(aSFS);
    aBS.SetRunParallel(myRunParallel);
    aBS.SetProgressIndicator(myProgressIndicator);
  }//for (i=0; i<aNbS; ++i) {
  //
  Standard_Integer k, aNbBS;
  //
  aNbBS=aVBS.Extent();
  //
  //===================================================
  BOPAlgo_BuilderSolidCnt::Perform(myRunParallel, aVBS);
  //===================================================
  //
  for (k = 0; k < aNbBS; ++k)
  {
    BOPAlgo_BuilderSolid& aBS = aVBS(k);
    aSolidsIm.Add(aBS.Solid(), aBS.Areas());
  }
  //
  // Add new solids to images map
  aNbBS = aSolidsIm.Extent();
  for (k = 1; k <= aNbBS; ++k)
  {
    const TopoDS_Shape& aS = aSolidsIm.FindKey(k);
    const BOPCol_ListOfShape& aLSR = aSolidsIm(k);
    //
    if (!myImages.IsBound(aS)) {
      BOPCol_ListOfShape* pLSx = myImages.Bound(aS, BOPCol_ListOfShape());
      //
      aIt.Initialize(aLSR);
      for (; aIt.More(); aIt.Next()) {
        BOPTools_Set aST;
        //
        const TopoDS_Shape& aSR=aIt.Value();
        aST.Add(aSR, TopAbs_FACE);
        //
        bFlagSD=aMST.Contains(aST);
        //
        const BOPTools_Set& aSTx=aMST.Added(aST);
        const TopoDS_Shape& aSx=aSTx.Shape();
        pLSx->Append(aSx);
        //
        BOPCol_ListOfShape* pLOr = myOrigins.ChangeSeek(aSx);
        if (!pLOr) {
          pLOr = myOrigins.Bound(aSx, BOPCol_ListOfShape());
        }
        pLOr->Append(aS);
        //
        if (bFlagSD) {
          myShapesSD.Bind(aSR, aSx);
        }
      }
    }
  }
}
//=======================================================================
//function :FillInternalShapes 
//purpose  : 
//=======================================================================
void BOPAlgo_Builder::FillInternalShapes()
{
  Standard_Integer i, j,  aNbS, aNbSI, aNbSx;
  TopAbs_ShapeEnum aType;
  TopAbs_State aState; 
  TopoDS_Iterator aItS;
  BRep_Builder aBB;
  BOPCol_ListIteratorOfListOfShape aIt, aIt1;
  //
  Handle(NCollection_BaseAllocator) aAllocator;
  //-----------------------------------------------------scope f
  aAllocator=NCollection_BaseAllocator::CommonBaseAllocator();
  //
  BOPCol_IndexedDataMapOfShapeListOfShape aMSx(100, aAllocator);
  BOPCol_IndexedMapOfShape aMx(100, aAllocator);
  BOPCol_IndexedMapOfShape aMSI(100, aAllocator);
  BOPCol_MapOfShape aMFence(100, aAllocator);
  BOPCol_MapOfShape aMSOr(100, aAllocator);
  BOPCol_ListOfShape aLSd(aAllocator);
  BOPCol_ListOfShape aLArgs(aAllocator);
  BOPCol_ListOfShape aLSC(aAllocator);
  BOPCol_ListOfShape aLSI(aAllocator);
  //
  // 1. Shapes to process
  //
  // 1.1 Shapes from pure arguments aMSI 
  // 1.1.1 vertex, edge, wire
  //
  const BOPCol_ListOfShape& aArguments=myDS->Arguments();
  aIt.Initialize(aArguments);
  for (; aIt.More(); aIt.Next()) {
    const TopoDS_Shape& aS=aIt.Value();
    TreatCompound(aS, aMFence, aLSC);
  }
  aIt.Initialize(aLSC);
  for (; aIt.More(); aIt.Next()) {
    const TopoDS_Shape& aS=aIt.Value();
    aType=aS.ShapeType();
    if (aType==TopAbs_WIRE) {
      aItS.Initialize(aS);
      for(; aItS.More(); aItS.Next()) {
        const TopoDS_Shape& aE=aItS.Value();
        if (aMFence.Add(aE)) {
          aLArgs.Append(aE);
        }
      }
    }
    else if (aType==TopAbs_VERTEX || aType==TopAbs_EDGE){
      aLArgs.Append(aS);
    } 
  }
  aMFence.Clear();
  //
  aIt.Initialize(aLArgs);
  for (; aIt.More(); aIt.Next()) {
    const TopoDS_Shape& aS=aIt.Value();
    aType=aS.ShapeType();
    if (aType==TopAbs_VERTEX || 
        aType==TopAbs_EDGE ||
        aType==TopAbs_WIRE) {
      if (aMFence.Add(aS)) {
        if (myImages.IsBound(aS)) {
          const BOPCol_ListOfShape &aLSp=myImages.Find(aS);
          aIt1.Initialize(aLSp);
          for (; aIt1.More(); aIt1.Next()) {
            const TopoDS_Shape& aSp=aIt1.Value();
            aMSI.Add(aSp);
          }
        }
        else {
          aMSI.Add(aS);
        }
      }
    }
  }
  
  aNbSI=aMSI.Extent();
  //
  // 2. Internal vertices, edges from source solids
  aMFence.Clear();
  aLSd.Clear();
  //
  aNbS=myDS->NbSourceShapes();
  for (i=0; i<aNbS; ++i) {
    const BOPDS_ShapeInfo& aSI=myDS->ShapeInfo(i);
    //
    if (aSI.ShapeType()!=TopAbs_SOLID) {
      continue;
    }
    //
    UserBreak();
    //
    const TopoDS_Shape& aS=aSI.Shape();
    //
    aMx.Clear();
    OwnInternalShapes(aS, aMx);
    //
    aNbSx=aMx.Extent();
    for (j=1; j<=aNbSx; ++j) {
      const TopoDS_Shape& aSi=aMx(j);
      if (myImages.IsBound(aSi)) {
        const BOPCol_ListOfShape &aLSp=myImages.Find(aSi);
        aIt1.Initialize(aLSp);
        for (; aIt1.More(); aIt1.Next()) {
          const TopoDS_Shape& aSp=aIt1.Value();
          aMSI.Add(aSp);
        }
      }
      else {
        aMSI.Add(aSi);
      }
    }
    //
    // build aux map from splits of solids
    if (myImages.IsBound(aS)) {
      const BOPCol_ListOfShape &aLSp=myImages.Find(aS);
      aIt.Initialize(aLSp);
      for (; aIt.More(); aIt.Next()) {
        const TopoDS_Shape& aSp=aIt.Value();
        if (aMFence.Add(aSp)) { 
          BOPTools::MapShapesAndAncestors(aSp, TopAbs_VERTEX, TopAbs_EDGE, aMSx);
          BOPTools::MapShapesAndAncestors(aSp, TopAbs_VERTEX, TopAbs_FACE, aMSx);
          BOPTools::MapShapesAndAncestors(aSp, TopAbs_EDGE  , TopAbs_FACE, aMSx);
          aLSd.Append(aSp);
        }
      }
    }
    else {
      if (aMFence.Add(aS)) {
        BOPTools::MapShapesAndAncestors(aS, TopAbs_VERTEX, TopAbs_EDGE, aMSx);
        BOPTools::MapShapesAndAncestors(aS, TopAbs_VERTEX, TopAbs_FACE, aMSx);
        BOPTools::MapShapesAndAncestors(aS, TopAbs_EDGE  , TopAbs_FACE, aMSx);
        aLSd.Append(aS);
        aMSOr.Add(aS); 
      }
    }
  }// for (i=0; i<aNbS; ++i) {
  //
  // 3. Some shapes of aMSI can be already tied with faces of 
  //    split solids
  aNbSI = aMSI.Extent();
  for (i = 1; i <= aNbSI; ++i) {
    const TopoDS_Shape& aSI = aMSI(i);
    if (aMSx.Contains(aSI)) {
      const BOPCol_ListOfShape &aLSx=aMSx.FindFromKey(aSI);
      aNbSx = aLSx.Extent();
      if (!aNbSx) {
        aLSI.Append(aSI);
      }
    }
    else {
      aLSI.Append(aSI);
    }
  }
  //
  // 4. Just check it
  aNbSI = aLSI.Extent();
  if (!aNbSI) {
    return;
  }
  //
  // 5 Settle internal vertices and edges into solids
  aMx.Clear();
  aIt.Initialize(aLSd);
  for (; aIt.More(); aIt.Next()) {
    TopoDS_Solid aSd=TopoDS::Solid(aIt.Value());
    //
    aIt1.Initialize(aLSI);
    for (; aIt1.More();) {
      TopoDS_Shape aSI = aIt1.Value();
      aSI.Orientation(TopAbs_INTERNAL);
      //
      aState=BOPTools_AlgoTools::ComputeStateByOnePoint
        (aSI, aSd, 1.e-11, myContext);
      //
      if (aState != TopAbs_IN) {
        aIt1.Next();
        continue;
      }
      //
      if (aMSOr.Contains(aSd)) {
        // make new solid
        TopoDS_Solid aSdx;
        //
        aBB.MakeSolid(aSdx);
        aItS.Initialize(aSd);
        for (; aItS.More(); aItS.Next()) {
          const TopoDS_Shape& aSh=aItS.Value();
          aBB.Add(aSdx, aSh);
        }
        //
        aBB.Add(aSdx, aSI);
        //
        // no need to check for images of aSd as aMSOr contains only original solids
        BOPCol_ListOfShape* pLS = myImages.Bound(aSd, BOPCol_ListOfShape());
        pLS->Append(aSdx);
        //
        BOPCol_ListOfShape* pLOr = myOrigins.Bound(aSdx, BOPCol_ListOfShape());
        pLOr->Append(aSd);
        //
        aMSOr.Remove(aSd);
        aSd=aSdx;
      }
      else {
        aBB.Add(aSd, aSI);
      }
      //
      aLSI.Remove(aIt1);
    }//for (; aIt1.More();) {
  }//for (; aIt.More(); aIt.Next()) {
  //
  //-----------------------------------------------------scope t
  aLArgs.Clear();
  aLSd.Clear();
  aMSOr.Clear();
  aMFence.Clear();
  aMSI.Clear();
  aMx.Clear();
  aMSx.Clear();
}
//=======================================================================
//function : OwnInternalShapes
//purpose  : 
//=======================================================================
void OwnInternalShapes(const TopoDS_Shape& theS,
                       BOPCol_IndexedMapOfShape& theMx)
{
  TopoDS_Iterator aIt;
  //
  aIt.Initialize(theS);
  for (; aIt.More(); aIt.Next()) {
    const TopoDS_Shape& aSx=aIt.Value();
    if (aSx.ShapeType()!=TopAbs_SHELL) {
      theMx.Add(aSx);
    }
  }
}
//=======================================================================
//function : TreatCompound
//purpose  : 
//=======================================================================
void TreatCompound(const TopoDS_Shape& theS,
                   BOPCol_MapOfShape& aMFence,
                   BOPCol_ListOfShape& theLS)
{
  TopAbs_ShapeEnum aType;
  //
  aType = theS.ShapeType();
  if (aType != TopAbs_COMPOUND) {
    if (aMFence.Add(theS)) {
      theLS.Append(theS);
    }
    return;
  }
  //
  TopoDS_Iterator aIt;
  //
  aIt.Initialize(theS);
  for (; aIt.More(); aIt.Next()) {
    const TopoDS_Shape& aS = aIt.Value();
    TreatCompound(aS, aMFence, theLS);
  }
}
