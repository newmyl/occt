// Created on: 2003-08-15
// Created by: Sergey ZARITCHNY
// Copyright (c) 2003-2014 OPEN CASCADE SAS
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


#include <DDocStd.hxx>
#include <DDocStd_DrawDocument.hxx>
#include <Draw.hxx>
#include <Draw_Interpretor.hxx>
#include <IFSelect_SessionPilot.hxx>
#include <IGESCAFControl_Reader.hxx>
#include <IGESCAFControl_Writer.hxx>
#include <IGESControl_Controller.hxx>
#include <Interface_Macros.hxx>
#include <STEPCAFControl_ExternFile.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <STEPControl_Controller.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TDataStd_Name.hxx>
#include <TDF_Data.hxx>
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <XDEDRAW.hxx>
#include <XDEDRAW_Common.hxx>
#include <XSControl_WorkSession.hxx>
#include <XSDRAW.hxx>
#include <XSDRAW_Vars.hxx>
#include <XSDRAWIGES.hxx>
#include <XSDRAWSTEP.hxx>
#include <DDF.hxx>

#include <DBRep.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_Editor.hxx>
#include <TDF_Tool.hxx>
#include <TopoDS_Shape.hxx>
#include <Interface_Static.hxx>

#include <BRep_Builder.hxx>
#include <OSD_Path.hxx>
#include <OSD_DirectoryIterator.hxx>
#include <OSD_FileIterator.hxx>
#include <TopoDS_Compound.hxx>
#include <VrmlData_Scene.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <Poly_Triangulation.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Geom_CylindricalSurface.hxx>

#include <VrmlData_ShapeNode.hxx>
#include <VrmlData_Group.hxx>

#include <StlAPI.hxx>

#include <stdio.h>
//============================================================
// Support for several models in DRAW
//============================================================
static NCollection_DataMap<TCollection_AsciiString, Handle(Standard_Transient)> thedictws;

static Standard_Boolean ClearDicWS()
{
  thedictws.Clear();
  return Standard_True;
}

static void AddWS(TCollection_AsciiString filename,
		  const Handle(XSControl_WorkSession)& WS)
{
  WS->SetVars ( new XSDRAW_Vars ); // support of DRAW variables
  thedictws.Bind( filename, WS );
}


static Standard_Boolean FillDicWS(NCollection_DataMap<TCollection_AsciiString, Handle(STEPCAFControl_ExternFile)>& dicFile)
{
  ClearDicWS();
  if ( dicFile.IsEmpty() ) {
    return Standard_False;
  }
  Handle(STEPCAFControl_ExternFile) EF;
  NCollection_DataMap<TCollection_AsciiString, Handle(STEPCAFControl_ExternFile)>::Iterator DicEFIt(dicFile);
  for (; DicEFIt.More(); DicEFIt.Next() ) {
    TCollection_AsciiString filename = DicEFIt.Key();
    EF = DicEFIt.Value();
    AddWS ( filename, EF->GetWS() );
  }
  return Standard_True;  
}

static Standard_Boolean SetCurrentWS (TCollection_AsciiString filename)
{
  if ( !thedictws.IsBound(filename) ) return Standard_False;
  Handle(XSControl_WorkSession) CurrentWS = 
    Handle(XSControl_WorkSession)::DownCast( thedictws.ChangeFind(filename) );
  XSDRAW::Pilot()->SetSession( CurrentWS );
  
  return Standard_True;
}


//=======================================================================
//function : SetCurWS
//purpose  : Set current file if many files are read
//=======================================================================

static Standard_Integer SetCurWS (Draw_Interpretor& di , Standard_Integer argc, const char** argv)
{
  if (argc <2) {
    di<<"Use: "<<argv[0]<<" filename \n";
    return 1;
  }
  TCollection_AsciiString filename (argv[1]);
  SetCurrentWS( filename );
  return 0;
}


//=======================================================================
//function : GetDicWSList
//purpose  : List all files recorded after translation
//=======================================================================

static Standard_Integer GetDicWSList (Draw_Interpretor& di, Standard_Integer /*argc*/, const char** /*argv*/)
{
  NCollection_DataMap<TCollection_AsciiString, Handle(Standard_Transient)> DictWS = thedictws;
  if ( DictWS.IsEmpty() ) return 1;
  NCollection_DataMap<TCollection_AsciiString, Handle(Standard_Transient)>::Iterator DicIt(DictWS);
  di << " The list of last translated files:\n";
  Standard_Integer num = 0;
  for (; DicIt.More() ; DicIt.Next(), num++ ) {
    TCollection_AsciiString strng ( DicIt.Key() );
    if ( num ) di << "\n";
    di << "\"" << strng.ToCString() << "\"";
  }
  return 0;
}

//=======================================================================
//function : GetCurWS
//purpose  : Return name of file which is current
//=======================================================================

static Standard_Integer GetCurWS (Draw_Interpretor& di, Standard_Integer /*argc*/, const char** /*argv*/)
{
  Handle(XSControl_WorkSession) WS = XSDRAW::Session();
  di << "\"" << WS->LoadedFile() << "\"";
  return 0;
}

//=======================================================================
//function : FromShape
//purpose  : Apply fromshape command to all the loaded WSs
//=======================================================================

static Standard_Integer FromShape (Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  if ( argc <2 ) {
    di << argv[0] << " shape: search for shape origin among all last tranalated files\n";
    return 0;
  }
  
  char command[256];
  Sprintf ( command, "fromshape %.200s -1", argv[1] );
  NCollection_DataMap<TCollection_AsciiString, Handle(Standard_Transient)> DictWS = thedictws;
  if ( DictWS.IsEmpty() ) return di.Eval ( command );
  
  Handle(XSControl_WorkSession) WS = XSDRAW::Session();

  NCollection_DataMap<TCollection_AsciiString, Handle(Standard_Transient)>::Iterator DicIt ( DictWS );
//  di << "Searching for shape among all the loaded files:\n";
  Standard_Integer num = 0;
  for (; DicIt.More() ; DicIt.Next(), num++ ) {
    Handle(XSControl_WorkSession) CurrentWS = 
      Handle(XSControl_WorkSession)::DownCast( DicIt.Value() );
    XSDRAW::Pilot()->SetSession( CurrentWS );
    di.Eval ( command );
  }

  XSDRAW::Pilot()->SetSession( WS );
  return 0;
}

//=======================================================================
//function : ReadIges
//purpose  : Read IGES to DECAF document
//=======================================================================

static Standard_Integer ReadIges (Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  if ( argc <3 ) {
    di << "Use: " << argv[0] << " Doc filename [mode]: read IGES file to a document\n";
    return 0;
  }
  
  DeclareAndCast(IGESControl_Controller,ctl,XSDRAW::Controller());
  if (ctl.IsNull()) XSDRAW::SetNorm("IGES");
 

  IGESCAFControl_Reader reader ( XSDRAW::Session(),Standard_True);
  Standard_Integer onlyvisible = Interface_Static::IVal("read.iges.onlyvisible");
  reader.SetReadVisible(onlyvisible == 1);
  
  if (argc == 4) {
    Standard_Boolean mode = Standard_True;
    for ( Standard_Integer i = 0; argv[3][i] ; i++ ) 
      switch (argv[3][i]) {
      case '-' : mode = Standard_False; break;
      case '+' : mode = Standard_True; break;
      case 'c' : reader.SetColorMode (mode); break;
      case 'n' : reader.SetNameMode (mode); break;
      case 'l' : reader.SetLayerMode (mode); break;
      }
  }
  TCollection_AsciiString fnom,rnom;
  Standard_Boolean modfic = XSDRAW::FileAndVar (argv[2],argv[1],"IGES",fnom,rnom);
  if (modfic) di<<" File IGES to read : "<<fnom.ToCString()<<"\n";
  else        di<<" Model taken from the session : "<<fnom.ToCString()<<"\n";
//  di<<" -- Names of variables BREP-DRAW prefixed by : "<<rnom<<"\n";
  IFSelect_ReturnStatus readstat = IFSelect_RetVoid;
  if (modfic) readstat = reader.ReadFile (fnom.ToCString());
  else  if (XSDRAW::Session()->NbStartingEntities() > 0) readstat = IFSelect_RetDone;
  if (readstat != IFSelect_RetDone) {
    if (modfic) di<<"Could not read file "<<fnom.ToCString()<<" , abandon\n";
    else di<<"No model loaded\n";
    return 1;
  }

  Handle(TDocStd_Document) doc;
  if (!DDocStd::GetDocument(argv[1],doc,Standard_False)) {  
    Handle(TDocStd_Application) A = DDocStd::GetApplication();
    A->NewDocument("BinXCAF",doc);  
    TDataStd_Name::Set(doc->GetData()->Root(),argv[1]);  
    Handle(DDocStd_DrawDocument) DD = new DDocStd_DrawDocument(doc);  
    Draw::Set(argv[1],DD);       
//     di << "Document saved with name " << argv[1];
  }
  if ( ! reader.Transfer ( doc ) ) {
    di << "Cannot read any relevant data from the IGES file\n";
    return 1;
  }
  
//  Handle(DDocStd_DrawDocument) DD = new DDocStd_DrawDocument(doc);  
//  Draw::Set(argv[1],DD);       
  di << "Document saved with name " << argv[1];
  
  return 0;
}

//=======================================================================
//function : WriteIges
//purpose  : Write DECAF document to IGES
//=======================================================================

static Standard_Integer WriteIges (Draw_Interpretor& di, Standard_Integer argc, const char** argv) 
{
  if ( argc <3 ) {
    di << "Use: " << argv[0] << " Doc filename [mode]: write document to IGES file\n";
    return 0;
  }
  
  Handle(TDocStd_Document) Doc;
  DDocStd::GetDocument(argv[1], Doc);
  if ( Doc.IsNull() ) {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  
  XSDRAW::SetNorm ("IGES");
  
//  IGESControl_Writer ICW (Interface_Static::CVal("write.iges.unit"),
//			  Interface_Static::IVal("write.iges.brep.mode"));
    
  IGESCAFControl_Writer writer ( XSDRAW::Session(), Standard_True );
  if (argc == 4) {
    Standard_Boolean mode = Standard_True;
    for ( Standard_Integer i = 0; argv[3][i] ; i++ ) 
      switch (argv[3][i]) {
      case '-' : mode = Standard_False; break;
      case '+' : mode = Standard_True; break;
      case 'c' : writer.SetColorMode (mode); break;
      case 'n' : writer.SetNameMode (mode); break;
      case 'l' : writer.SetLayerMode (mode); break;
      }
  }
  writer.Transfer ( Doc );

  di << "Writig IGES model to file " << argv[2] << "\n";
  if ( writer.Write ( argv[2] ) ) di<<" Write OK\n";
  else di<<" Write failed\n";

  return 0;
}

//=======================================================================
//function : ReadStep
//purpose  : Read STEP file to DECAF document 
//=======================================================================

static Standard_Integer ReadStep (Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  if ( argc <3 ) {
    di << "Use: " << argv[0] << " Doc filename [mode]: read STEP file to a document\n";
    return 0;
  }
  
  DeclareAndCast(STEPControl_Controller,ctl,XSDRAW::Controller());
  if (ctl.IsNull()) XSDRAW::SetNorm("STEP");

  STEPCAFControl_Reader reader ( XSDRAW::Session(),Standard_True);
  
  if (argc == 4) {
    Standard_Boolean mode = Standard_True;
    for ( Standard_Integer i = 0; argv[3][i] ; i++ ) 
      switch (argv[3][i]) {
      case '-' : mode = Standard_False; break;
      case '+' : mode = Standard_True; break;
      case 'c' : reader.SetColorMode (mode); break;
      case 'n' : reader.SetNameMode (mode); break;
      case 'l' : reader.SetLayerMode (mode); break;
      case 'v' : reader.SetPropsMode (mode); break;
      }
  }
  
  TCollection_AsciiString fnom,rnom;
  Standard_Boolean modfic = XSDRAW::FileAndVar (argv[2],argv[1],"STEP",fnom,rnom);
  if (modfic) di<<" File STEP to read : "<<fnom.ToCString()<<"\n";
  else        di<<" Model taken from the session : "<<fnom.ToCString()<<"\n";
//  di<<" -- Names of variables BREP-DRAW prefixed by : "<<rnom<<"\n";
  IFSelect_ReturnStatus readstat = IFSelect_RetVoid;
  if (modfic) readstat = reader.ReadFile (fnom.ToCString());
  else  if (XSDRAW::Session()->NbStartingEntities() > 0) readstat = IFSelect_RetDone;
  if (readstat != IFSelect_RetDone) {
    if (modfic) di<<"Could not read file "<<fnom.ToCString()<<" , abandon\n";
    else di<<"No model loaded\n";
    return 1;
  }

  Handle(TDocStd_Document) doc;
  if (!DDocStd::GetDocument(argv[1],doc,Standard_False)) {  
    Handle(TDocStd_Application) A = DDocStd::GetApplication();
    A->NewDocument("BinXCAF",doc);  
    TDataStd_Name::Set(doc->GetData()->Root(),argv[1]);  
    Handle(DDocStd_DrawDocument) DD = new DDocStd_DrawDocument(doc);  
    Draw::Set(argv[1],DD);       
//     di << "Document saved with name " << argv[1];
  }
  if ( ! reader.Transfer ( doc ) ) {
    di << "Cannot read any relevant data from the STEP file\n";
    return 1;
  }
  
  Handle(DDocStd_DrawDocument) DD = new DDocStd_DrawDocument(doc);  
  Draw::Set(argv[1],DD);       
  di << "Document saved with name " << argv[1];

  NCollection_DataMap<TCollection_AsciiString, Handle(STEPCAFControl_ExternFile)> DicFile = reader.ExternFiles();
  FillDicWS( DicFile );
  AddWS ( fnom , XSDRAW::Session() );
  
  return 0;
}

//=======================================================================
//function : WriteStep
//purpose  : Write DECAF document to STEP
//=======================================================================

static Standard_Integer WriteStep (Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  if ( argc <3 ) {
    di << "Use: " << argv[0] << " Doc filename [mode [multifile_prefix [label]]]: write document to the STEP file\n";
    di << "mode can be: a or 0 : AsIs (default)\n";
    di << "             f or 1 : FacettedBRep        s or 2 : ShellBasedSurfaceModel\n";
    di << "             m or 3 : ManifoldSolidBrep   w or 4 : GeometricCurveSet/WireFrame\n";
    di << "multifile_prefix: triggers writing assembly components as separate files,\n";
    di << "                  and defines common prefix for their names\n";
    di << "label: tag of the sub-assembly label to save only that sub-assembly\n";
    return 0;
  }
  
  Handle(TDocStd_Document) Doc;   
  DDocStd::GetDocument(argv[1], Doc);
  if ( Doc.IsNull() ) {
    di << argv[1] << " is not a document\n";
    return 1;
  }
  Standard_CString multifile = 0;
  
  Standard_Integer k = 3;
  DeclareAndCast(STEPControl_Controller,ctl,XSDRAW::Controller());
  if (ctl.IsNull()) XSDRAW::SetNorm("STEP");
  STEPCAFControl_Writer writer ( XSDRAW::Session(), Standard_True );
   
  STEPControl_StepModelType mode = STEPControl_AsIs;
  if ( argc > k ) {
    switch (argv[k][0]) {
    case 'a' :
    case '0' : mode = STEPControl_AsIs;                    break;
    case 'f' :
    case '1' : mode = STEPControl_FacetedBrep;             break;
    case 's' :
    case '2' : mode = STEPControl_ShellBasedSurfaceModel;  break;
    case 'm' :
    case '3' : mode = STEPControl_ManifoldSolidBrep;       break;
    case 'w' :
    case '4' : mode = STEPControl_GeometricCurveSet;       break;
    default :  di<<"3st arg = mode, incorrect [give fsmw]\n"; return 1;
    }
    Standard_Boolean wrmode = Standard_True;
    for ( Standard_Integer i = 0; argv[k][i] ; i++ ) 
      switch (argv[3][i]) {
      case '-' : wrmode = Standard_False; break;
      case '+' : wrmode = Standard_True; break;
      case 'c' : writer.SetColorMode (wrmode); break;
      case 'n' : writer.SetNameMode (wrmode); break;
      case 'l' : writer.SetLayerMode (wrmode); break;
      case 'v' : writer.SetPropsMode (wrmode); break;
      }
    k++;
  }

  TDF_Label label;
  if( argc > k)
  {
    TCollection_AsciiString aStr(argv[k]);
    if( aStr.Search(":") ==-1)
      multifile = argv[k++];
    
  }
  if( argc > k)
  {
      
    if( !DDF::FindLabel(Doc->Main().Data(), argv[k], label) || label.IsNull()) {  
      di << "No label for entry"  << "\n";
      return 1; 
      
    }
  }
  if( !label.IsNull())
  {  
    di << "Translating label "<< argv[k]<<" of document " << argv[1] << " to STEP\n";
    if(!writer.Transfer ( label, mode, multifile )) 
    {
      di << "The label of document cannot be translated or gives no result\n";
      return 1;
    }

  }
  else
  {
    di << "Translating document " << argv[1] << " to STEP\n";
    if ( ! writer.Transfer ( Doc, mode, multifile ) ) {
      di << "The document cannot be translated or gives no result\n";
    }
  }
  

  di << "Writing STEP file " << argv[2] << "\n";
  IFSelect_ReturnStatus stat = writer.Write(argv[2]);
  switch (stat) {
    case IFSelect_RetVoid : di<<"No file written\n"; break;
    case IFSelect_RetDone : {
      di<<"File "<<argv[2]<<" written\n";

      NCollection_DataMap<TCollection_AsciiString, Handle(STEPCAFControl_ExternFile)> DicFile = writer.ExternFiles();
      FillDicWS( DicFile );
      AddWS( argv[2], XSDRAW::Session() );
      break;
    }
    default : di<<"Error on writing file\n"; break;
  }
  return 0;
}

static Standard_Integer Expand (Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  if (argc < 3) {
    di<<"Use: "<<argv[0]<<" Doc recurs(0/1) or Doc recurs(0/1) label1 label2 ... or Doc recurs(0/1 shape1 shape2 ...\n";
    return 1;
  }
  Handle(TDocStd_Document) Doc;   
  DDocStd::GetDocument(argv[1], Doc);
  if ( Doc.IsNull() ) { di << argv[1] << " is not a document\n"; return 1; }

  Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool(Doc->Main());
  Standard_Boolean recurs = Standard_False;
  if(atoi(argv[2]) != 0)
    recurs = Standard_True;

  if (argc == 3)
  {
    if(!XCAFDoc_Editor::Expand(Doc->Main(), recurs)){
      di << "No suitable labels to expand\n";
      return 1;
    }
  }
  else 
  {
    for (Standard_Integer i = 3; i < argc; i++)
    {
      TDF_Label aLabel;
      TDF_Tool::Label(Doc->GetData(), argv[i], aLabel);
      if(aLabel.IsNull()){
        TopoDS_Shape aShape;
        aShape = DBRep::Get(argv[i]);
        aLabel = aShapeTool->FindShape(aShape);
      }

      if (!aLabel.IsNull()){
        if(!XCAFDoc_Editor::Expand(Doc->Main(), aLabel, recurs)){
          di << "The shape is assembly or not compound\n";
          return 1;
        }
      }
      else
      { di << argv[i] << " is not a shape\n"; return 1; }
    }
  }
  return 0;
}

//=======================================================================
//function : ReadVrml
//purpose  : Read VRML file to DECAF document 
//=======================================================================

TDF_Label ReadVrmlRec(const OSD_Path& thePath, const Handle(TDocStd_Document)& theDoc, const TDF_Label& theLabel)
{
  TDF_Label aNewLabel;
  Handle(XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool( theDoc->Main() );
  Handle(XCAFDoc_ColorTool) aColorTool = XCAFDoc_DocumentTool::ColorTool( theDoc->Main() );

  // Get path of the VRML file.
  TCollection_AsciiString aVrmlDir(".");
  TCollection_AsciiString aDisk = thePath.Disk();
  TCollection_AsciiString aTrek = thePath.Trek();
  TCollection_AsciiString aName = thePath.Name();
  TCollection_AsciiString anExt = thePath.Extension();

  cout << "==============================================================" << endl;

  if (!aTrek.IsEmpty())
  {
    if (!aDisk.IsEmpty())
      aVrmlDir = aDisk;
    else
      aVrmlDir.Clear();
    aTrek.ChangeAll('|', '/');
    aTrek.ChangeAll('\\', '/');
    aVrmlDir += aTrek;

    if (!aName.IsEmpty())
      aVrmlDir += aName;

    if (!anExt.IsEmpty())
      aVrmlDir += anExt;
  }

  // Analize the passed Path
  if (thePath.Extension() == ".wrl")
  {
    // Read shape from vrml and add to parent as component
    TopoDS_Shape aShape ;
    VrmlData_DataMapOfShapeAppearance aShapeAppMap;
    filebuf aFic;
    istream aStream (&aFic);
    if (aFic.open(aVrmlDir.ToCString(), ios::in))
    {
      cout << "Reading file " << aVrmlDir.ToCString() << "..." << endl;

      VrmlData_Scene aScene;
      aScene.SetVrmlDir(aVrmlDir);
      aScene << aStream;
      aFic.close();
      if (aScene.Status() == VrmlData_StatusOK)
      {
        aShape = aScene.GetShape(aShapeAppMap);

        if (aShape.IsNull())
        {
          cout << "Shape was not extracted from file " << aVrmlDir.ToCString() << "..." << endl;
          return aNewLabel;
        }

        TopExp_Explorer anExp;
        Quantity_Color aFaceColor;
        Quantity_Color anEdgeColor(Quantity_NOC_BLACK);
        for (anExp.Init(aShape, TopAbs_FACE); anExp.More(); anExp.Next())
        {
          if (aShapeAppMap.IsBound(anExp.Current().TShape()))
          {
            Handle(VrmlData_Appearance) anAppearance = aShapeAppMap.Find(anExp.Current().TShape());
            if (!anAppearance.IsNull() && !anAppearance->Material().IsNull())
            {
              aFaceColor = anAppearance->Material()->DiffuseColor();
              break;
            }
          }
        }

        //Standard_Boolean isSingleShell = Standard_False;
        //TopoDS_Shell aShell;
        //for (anExp.Init(aShape, TopAbs_SHELL); anExp.More(); anExp.Next())
        //{
        //  if (!anExp.Current().IsNull() && anExp.Current().ShapeType() == TopAbs_SHELL)
        //  {
        //    if (!isSingleShell)
        //    {
        //      aShell = TopoDS::Shell(anExp.Current());
        //      isSingleShell = Standard_True;
        //    }
        //    else
        //    {
        //      isSingleShell = Standard_False;
        //      break;
        //    }
        //  }
        //}

        //Standard_Boolean hasCylindrical = Standard_False;
        //if (!aShell.IsNull() && isSingleShell)
        //{
        //  for (anExp.Init(aShell, TopAbs_FACE); anExp.More(); anExp.Next())
        //  {
        //    if (!anExp.Current().IsNull())
        //    {
        //      const TopoDS_Face& aFace = TopoDS::Face(anExp.Current());
        //      Handle(Geom_CylindricalSurface) aSurf = Handle(Geom_CylindricalSurface)::DownCast(BRep_Tool::Surface(aFace));
        //      if (!aSurf.IsNull())
        //        hasCylindrical = Standard_True;
        //    }
        //  }
        //}
        //if (hasCylindrical)
        //  aShape = aShell;
        //else
        //  isSingleShell = Standard_False;

        //if (!isSingleShell)
        //{
        //  Standard_Boolean doTriangulation = Standard_False;
        //  for (anExp.Init(aShape, TopAbs_FACE); anExp.More(); anExp.Next())
        //  {
        //    TopLoc_Location aDummy;
        //    if (BRep_Tool::Triangulation(TopoDS::Face(anExp.Current()), aDummy).IsNull())
        //    {
        //      doTriangulation = Standard_True;
        //      break;
        //    }
        //  }

        //  if (doTriangulation)
        //  {
        //    BRepMesh_IncrementalMesh aMesh(aShape, 1);
        //    aMesh.Perform();
        //    if (aMesh.IsDone())
        //      aShape = aMesh.Shape();
        //  }

        //  // Write to STL and then read again to get BRep model (vrml fills only triangulation)
        //  StlAPI::Write(aShape, vrmlTempFile);
        //  StlAPI::Read(aShape, vrmlTempFile);
        //}

        /*if (aShape.IsNull())
          return aNewLabel;*/

        /*ShapeUpgrade_UnifySameDomain anUSD(aShape);
        anUSD.SetLinearTolerance(1e-5);
        anUSD.Build();
        aShape = anUSD.Shape();

        if (aShape.ShapeType() == TopAbs_SHELL && TopoDS::Shell(aShape).Closed())
        {
          TopoDS_Solid aSolid;
          TopoDS_Shell aShell = TopoDS::Shell (aShape);
          if (!aShell.Free ()) {
            aShell.Free(Standard_True);
          }
          BRep_Builder aBuilder;
          aBuilder.MakeSolid (aSolid);
          aBuilder.Add (aSolid, aShell);

          Standard_Boolean isOk = Standard_True;
          try {
            OCC_CATCH_SIGNALS
            BRepClass3d_SolidClassifier bsc3d (aSolid);
            Standard_Real t = Precision::Confusion();
            bsc3d.PerformInfinitePoint(t);

            if (bsc3d.State() == TopAbs_IN) {
              TopoDS_Solid aSolid2;
              aBuilder.MakeSolid (aSolid2);
              aShell.Reverse();
              aBuilder.Add (aSolid2, aShell);
              aSolid = aSolid2;
            }
          }
          catch (Standard_Failure) { isOk = Standard_False; }
          if (isOk) aShape = aSolid;
        }*/

        if (theLabel.IsNull() || !aShapeTool->IsAssembly(theLabel))
        {
          // Add new shape
          aNewLabel = aShapeTool->AddShape(aShape, Standard_False);
          cout << "Created new shape " << thePath.Name() << " (free)" << endl;
        }
        else if (!theLabel.IsNull() && aShapeTool->IsAssembly(theLabel))
        {
          // Add shape as component
          aNewLabel = aShapeTool->AddComponent(theLabel, aShape);
          Handle(TDataStd_Name) anAttrName;
          theLabel.FindAttribute(TDataStd_Name::GetID(), anAttrName);
          cout << "Created new shape " << thePath.Name() << " as part of " << (anAttrName.IsNull()? "(noname)" : TCollection_AsciiString(anAttrName->Get())) << endl;

          if ( aShapeTool->IsReference(aNewLabel) )
          {
            TDF_Label RefLabel;
            if ( aShapeTool->GetReferredShape(aNewLabel, RefLabel) )
              aNewLabel = RefLabel;
          }
        }

        if (!aNewLabel.IsNull())
          TDataStd_Name::Set(aNewLabel, thePath.Name());

        aColorTool->SetColor(aNewLabel, aFaceColor, XCAFDoc_ColorSurf);
        aColorTool->SetColor(aNewLabel, anEdgeColor, XCAFDoc_ColorCurv);
        return aNewLabel;
      }
      else
      {
        cout << "Error during reading of vrml file " << aVrmlDir.ToCString() << endl;
      }
    }
    else
    {
      cout << "Cannot open file " << aVrmlDir.ToCString() << endl;
    }
  }
  else if (thePath.Extension().IsEmpty())
  {
    cout << "Scaning root " << aVrmlDir.ToCString() << "..." << endl;

    OSD_DirectoryIterator aDirIt(thePath, "*");
    OSD_FileIterator aFileIt(thePath, "*.wrl");

    // Check real dircetories
    OSD_Path aSubDirPath;
    TCollection_AsciiString aTempName;
    Standard_Boolean isSubDirExist = Standard_False;
    for (; aDirIt.More(); aDirIt.Next())
    {
      aDirIt.Values().Path(aSubDirPath);
      aTempName = aSubDirPath.Name() + aSubDirPath.Extension();
      if (aTempName != "." && aTempName != "..")
      {
        isSubDirExist = Standard_True;
        break;
      }
    }
    aDirIt.Initialize(thePath, "*"); // Re-initialize

    if (!isSubDirExist && !aFileIt.More())
    {
      cout << "No files or directories detected in " << aVrmlDir.ToCString() << endl;
    }
    else
    {
      // Create assembly
      TopoDS_Compound aComp;
      BRep_Builder aBuilder;
      aBuilder.MakeCompound(aComp);

      if (theLabel.IsNull() || !aShapeTool->IsAssembly(theLabel))
      {
        // Add new shape
        aNewLabel = aShapeTool->AddShape(aComp);
        cout << "Created new assembly " << thePath.Name() << " (free)" << endl;
      }
      else if (!theLabel.IsNull() && aShapeTool->IsAssembly(theLabel))
      {
        // Add shape as component
        aNewLabel = aShapeTool->AddComponent(theLabel, aComp, Standard_True);
        Handle(TDataStd_Name) anAttrName;
        theLabel.FindAttribute(TDataStd_Name::GetID(), anAttrName);
        cout << "Created new assembly " << thePath.Name() << " as part of " << (anAttrName.IsNull()? "(noname)" : TCollection_AsciiString(anAttrName->Get())) << endl;

        if ( aShapeTool->IsReference(aNewLabel) )
        {
          TDF_Label RefLabel;
          if ( aShapeTool->GetReferredShape(aNewLabel, RefLabel) )
            aNewLabel = RefLabel;
        }
      }

      if (!aNewLabel.IsNull())
        TDataStd_Name::Set(aNewLabel, thePath.Name());

      // Add components
      for (; aDirIt.More(); aDirIt.Next())
      {
        aDirIt.Values().Path(aSubDirPath);
        aTempName = aSubDirPath.Name() + aSubDirPath.Extension();
        if (aTempName == "." || aTempName == "..")
          continue;
        aSubDirPath.SetDisk(thePath.Disk());
        aSubDirPath.SetTrek(thePath.Trek() + thePath.Name());
        ReadVrmlRec(aSubDirPath, theDoc, aNewLabel);
      }

      for (; aFileIt.More(); aFileIt.Next())
      {
        aFileIt.Values().Path(aSubDirPath);
        aSubDirPath.SetDisk(thePath.Disk());
        aSubDirPath.SetTrek(thePath.Trek() + thePath.Name());
        ReadVrmlRec(aSubDirPath, theDoc, aNewLabel);
      }

      aShapeTool->UpdateAssemblies();
    }
    // At the end of operation update assemblies
  }
  else
  {
    cout << "Unknown file format: " << thePath.Extension() << endl;
  }

  return aNewLabel;
}

//=======================================================================
//function : HaveShapeNode
//purpose  : Check that VRML node have Shape
//=======================================================================
Standard_Boolean HaveShapeNode (const Handle (VrmlData_Node)& theNode)
{
  // Try a Shape type of node.
  const Handle (VrmlData_ShapeNode) aShapeNode = Handle (VrmlData_ShapeNode)::DownCast (theNode);
  if (!aShapeNode.IsNull ())
  {
    const Handle (VrmlData_Geometry) aGeometry = aShapeNode->Geometry ();
    if (!aGeometry.IsNull ())
    {
      const Handle (TopoDS_TShape) aTShape = aGeometry->TShape ();
      if (!aTShape.IsNull ())
      {
        return Standard_True;
      }
    }
  }

  // Try a Group type of node.
  const Handle (VrmlData_Group) aGroupNode = Handle (VrmlData_Group)::DownCast (theNode);
  if (!aGroupNode.IsNull ())
  {
    for (VrmlData_Group::Iterator anIt = aGroupNode->NodeIterator (); anIt.More (); anIt.Next ())
    {
      if (HaveShapeNode (anIt.Value ()))
      {
        return Standard_True;
      }
    }
  }

  return Standard_False;
}

//=======================================================================
//function : ReadVrmlNode
//purpose  : Read VRML node to DECAF document
//=======================================================================
TDF_Label ReadVrmlNode (const Handle(VrmlData_Node)& theNode,
                        const Handle(TDocStd_Document)& theDoc,
                        const TDF_Label& theLabel = TDF_Label())
{
  TDF_Label aNewLabel;
  Handle (XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool (theDoc->Main ());
  Handle (XCAFDoc_ColorTool) aColorTool = XCAFDoc_DocumentTool::ColorTool (theDoc->Main ());

  // Try a Shape type of node.
  const Handle (VrmlData_ShapeNode) aShapeNode = Handle (VrmlData_ShapeNode)::DownCast (theNode);
  if (!aShapeNode.IsNull ())
  {
    const Handle (VrmlData_Geometry) aGeometry = aShapeNode->Geometry ();
    if (!aGeometry.IsNull ())
    {
      const Handle (TopoDS_TShape) aTShape = aGeometry->TShape ();
      if (!aTShape.IsNull ())
      {
        TopoDS_Shape aShape;
        aShape.Orientation (TopAbs_FORWARD);
        aShape.TShape (aTShape);

        if (theLabel.IsNull () || !aShapeTool->IsAssembly (theLabel))
        {
          // Add new shape.
          aNewLabel = aShapeTool->AddShape (aShape, Standard_False);
        }
        else if (!theLabel.IsNull() && aShapeTool->IsAssembly (theLabel))
        {
          // Add shape as component.
          aNewLabel = aShapeTool->AddComponent (theLabel, aShape);

          if (aShapeTool->IsReference (aNewLabel))
          {
            TDF_Label aRefLabel;
            if (aShapeTool->GetReferredShape (aNewLabel, aRefLabel))
            {
              aNewLabel = aRefLabel;
            }
          }
        }

        if (!aNewLabel.IsNull ())
        {
          TDataStd_Name::Set (aNewLabel, aShapeNode->Name ());
          Quantity_Color aFaceColor;
          Quantity_Color anEdgeColor (Quantity_NOC_BLACK);
          const Handle (VrmlData_Appearance) anAppearance = aShapeNode->Appearance ();
          if (!anAppearance.IsNull ())
          {
            aFaceColor = anAppearance->Material ()->DiffuseColor ();
          }
          aColorTool->SetColor (aNewLabel, aFaceColor, XCAFDoc_ColorSurf);
          aColorTool->SetColor (aNewLabel, anEdgeColor, XCAFDoc_ColorCurv);
        }
      }
    }
  }

  // Try a Group type of node.
  const Handle (VrmlData_Group) aGroupNode = Handle (VrmlData_Group)::DownCast (theNode);
  if (!aGroupNode.IsNull ()/* && HaveShapeNode (aGroupNode)*/)
  {
    // Create assembly
    TopoDS_Compound aCompound;
    BRep_Builder aBuilder;
    aBuilder.MakeCompound (aCompound);
    aCompound.Location (aGroupNode->GetTransform ());

    if (theLabel.IsNull () || !aShapeTool->IsAssembly (theLabel))
    {
      // Add new shape.
      aNewLabel = aShapeTool->AddShape (aCompound);
    }
    else if (!theLabel.IsNull () && aShapeTool->IsAssembly (theLabel))
    {
      // Add shape as component.
      aNewLabel = aShapeTool->AddComponent (theLabel, aCompound, Standard_True);

      if (aShapeTool->IsReference (aNewLabel))
      {
        TDF_Label aRefLabel;
        if (aShapeTool->GetReferredShape (aNewLabel, aRefLabel))
        {
          aNewLabel = aRefLabel;
        }
      }
    }

    if (!aNewLabel.IsNull ())
    {
      TDataStd_Name::Set (aNewLabel, aGroupNode->Name ());
    }

    // Add components.
    for (VrmlData_Group::Iterator anIt = aGroupNode->NodeIterator (); anIt.More (); anIt.Next ())
    {
      ReadVrmlNode (anIt.Value (), theDoc, aNewLabel);
    }

    aShapeTool->UpdateAssemblies();
  }

  return aNewLabel;
}

//=======================================================================
//function : ReadSingleVrml
//purpose  : Read VRML file to DECAF document
//=======================================================================
bool ReadSingleVrml (const OSD_Path& thePath, const Handle (TDocStd_Document)& theDoc)
{
  // Get path of the VRML file.
  TCollection_AsciiString aPath (".");
  TCollection_AsciiString aDisk = thePath.Disk ();
  TCollection_AsciiString aTrek = thePath.Trek ();
  TCollection_AsciiString aName = thePath.Name ();
  TCollection_AsciiString anExt = thePath.Extension ();

  if (!aTrek.IsEmpty())
  {
    if (!aDisk.IsEmpty())
    {
      aPath = aDisk;
    }
    else
    {
      aPath.Clear ();
    }
    aTrek.ChangeAll ('|', '/');
    aTrek.ChangeAll ('\\', '/');
    aPath += aTrek;

    if (!aName.IsEmpty())
    {
      aPath += aName;
    }

    if (!anExt.IsEmpty())
    {
      aPath += anExt;
    }
  }

  // Analize the passed path.
  if (anExt != ".wrl")
  {
    std::cerr << "Reading failed. Unknown file format: " << anExt << std::endl;
    return false;
  }

  // Open file.
  filebuf aFileBuf;
  aFileBuf.open (aPath.ToCString (), ios::in);
  if (aFileBuf.is_open () == Standard_False)
  {
    std::cerr << "Reading failed. Can not open file: " << aPath.ToCString () << std::endl;
    return false;
  }

  // Read vrml file.
  Standard_IStream aStream (&aFileBuf);
  VrmlData_Scene aScene;
  aScene.SetVrmlDir (aPath);
  aScene << aStream;
  aFileBuf.close();
  if (aScene.Status () != VrmlData_StatusOK)
  {
    std::cerr << "Reading failed. Error: " << aScene.Status () << std::endl;
    return false;
  }

  // Read shape.
  //VrmlData_DataMapOfShapeAppearance aShapeAppearMap;
  //TopoDS_Shape aShape = aScene.GetShape (aShapeAppearMap);
  //if (aShape.IsNull ())
  //{
  //  std::cerr << "Reading failed. Getting TopoDS_Shape failed." << std::endl;
  //  return false;
  //}

  // Create assembly.
  TopoDS_Compound aCompound;
  BRep_Builder aBuilder;
  aBuilder.MakeCompound (aCompound);
  Handle (XCAFDoc_ShapeTool) aShapeTool = XCAFDoc_DocumentTool::ShapeTool (theDoc->Main ());
  TDF_Label aNewLabel = aShapeTool->AddShape (aCompound);
  if (!aNewLabel.IsNull ())
  {
    TDataStd_Name::Set (aNewLabel, aName);
  }

  // Read nodes one by one and add to document.
  for (VrmlData_Scene::Iterator anIt = aScene.GetIterator (); anIt.More (); anIt.Next ())
  {
    ReadVrmlNode (anIt.Value (), theDoc, aNewLabel);
  }
  aShapeTool->UpdateAssemblies();

  return true;
}

static Standard_Integer ReadVrml (Draw_Interpretor& di, Standard_Integer argc, const char** argv)
{
  if ( argc != 3 ) {
    di << "Use: " << argv[0] << " Doc Path: Read VRML assembly to DECAF document\n";
    return 0;
  }

  Handle(TDocStd_Document) doc;
  if (!DDocStd::GetDocument(argv[1],doc,Standard_False))
  {
    Handle(TDocStd_Application) A = DDocStd::GetApplication();
    A->NewDocument("BinXCAF",doc);
    TDataStd_Name::Set(doc->GetData()->Root(),argv[1]);
    Handle(DDocStd_DrawDocument) DD = new DDocStd_DrawDocument(doc);
    Draw::Set(argv[1], DD);
  }

  OSD_Path aPath (argv[2]);

  if (aPath.Extension () == ".wrl")
  {
    ReadSingleVrml (aPath, doc);
  }
  else if (aPath.Extension ().IsEmpty ())
  {
    ReadVrmlRec (aPath, doc, TDF_Label ());
  }

  return 0;
}

void XDEDRAW_Common::InitCommands(Draw_Interpretor& di)
{
  static Standard_Boolean initactor = Standard_False;
  if (initactor)
  {
    return;
  }
  initactor = Standard_True;

  Standard_CString g = "XDE translation commands";

  di.Add("ReadIges" , "Doc filename: Read IGES file to DECAF document" ,__FILE__, ReadIges, g);
  di.Add("WriteIges" , "Doc filename: Write DECAF document to IGES file" ,__FILE__, WriteIges, g);
  di.Add("ReadStep" , "Doc filename: Read STEP file to DECAF document" ,__FILE__, ReadStep, g);
  di.Add("WriteStep" , "Doc filename [mode=a [multifile_prefix] [label]]: Write DECAF document to STEP file" ,__FILE__, WriteStep, g);  
  
  di.Add("XFileList","Print list of files that was transfered by the last transfer" ,__FILE__, GetDicWSList , g);
  di.Add("XFileCur", ": returns name of file which is set as current",__FILE__, GetCurWS, g);
  di.Add("XFileSet", "filename: Set the specified file to be the current one",__FILE__, SetCurWS, g);
  di.Add("XFromShape", "shape: do fromshape command for all the files",__FILE__, FromShape, g);

  di.Add("XExpand", "XExpand Doc recursively(0/1) or XExpand Doc recursively(0/1) label1 label2 ..."  
          "or XExpand Doc recursively(0/1) shape1 shape2 ...",__FILE__, Expand, g);

  di.Add("ReadVrml" , "Doc Path: Read VRML assembly to DECAF document" ,__FILE__, ReadVrml, g);
}
