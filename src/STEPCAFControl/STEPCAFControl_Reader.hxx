// Copyright (c) 2013 OPEN CASCADE SAS
//
// The content of this file is subject to the Open CASCADE Technology Public
// License Version 6.5 (the "License"). You may not use the content of this file
// except in compliance with the License. Please obtain a copy of the License
// at http://www.opencascade.org and read it completely before using this file.
//
// The Initial Developer of the Original Code is Open CASCADE S.A.S., having its
// main offices at: 1, place des Freres Montgolfier, 78280 Guyancourt, France.
//
// The Original Code and all software distributed under the License is
// distributed on an "AS IS" basis, without warranty of any kind, and the
// Initial Developer hereby disclaims all such warranties, including without
// limitation, any warranties of merchantability, fitness for a particular
// purpose or non-infringement. Please see the License for the specific terms
// and conditions governing the rights and limitations under the License.

#ifndef _STEPCAFControl_Reader_HeaderFile
#define _STEPCAFControl_Reader_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineAlloc.hxx>
#include <Standard_Macro.hxx>
#include <STEPControl_Reader.hxx>
#include <Handle_STEPCAFControl_DictionaryOfExternFile.hxx>
#include <Standard_Boolean.hxx>
#include <XSControl_WorkSession.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Standard_CString.hxx>
#include <Standard_Integer.hxx>
#include <Handle_TDocStd_Document.hxx>
#include <Handle_STEPCAFControl_ExternFile.hxx>
#include <Handle_XCAFDoc_ShapeTool.hxx>
#include <Handle_TColStd_HSequenceOfTransient.hxx>
#include <Handle_StepRepr_RepresentationItem.hxx>
#include <Transfer_TransientProcess.hxx>
#include <Handle_StepShape_ConnectedFaceSet.hxx>
#include <Handle_StepRepr_NextAssemblyUsageOccurrence.hxx>

class STEPCAFControl_DictionaryOfExternFile;
class XSControl_WorkSession;
class TDocStd_Document;
class TCollection_AsciiString;
class STEPCAFControl_ExternFile;
class STEPControl_Reader;
class TDF_LabelSequence;
class TDF_Label;
class TopoDS_Shape;
class XCAFDoc_ShapeTool;
class TopTools_MapOfShape;
class STEPCAFControl_DataMapOfShapePD;
class STEPCAFControl_DataMapOfPDExternFile;
class XCAFDoc_DataMapOfShapeLabel;
class TColStd_HSequenceOfTransient;
class StepRepr_RepresentationItem;
class Transfer_TransientProcess;
class StepShape_ConnectedFaceSet;
class StepRepr_NextAssemblyUsageOccurrence;
class STEPConstruct_Tool;


//! Provides a tool to read STEP file and put it into <br>
//!          DECAF document. Besides transfer of shapes (including <br>
//!          assemblies) provided by STEPControl, supports also <br>
//!          colors and part names <br>
//! <br>
//!          This reader supports reading files with external references <br>
//!          i.e. multifile reading <br>
//!          It behaves as usual Reader (from STEPControl) for the main <br>
//!          file (e.g. if it is single file) <br>
//!          Results of reading other files can be accessed by name of the <br>
//!          file or by iterating on a readers <br>
class STEPCAFControl_Reader  {
public:

  DEFINE_STANDARD_ALLOC

  //! Creates a reader with an empty <br>
//! STEP model and sets ColorMode, LayerMode, NameMode and <br>
//! PropsMode to Standard_True. <br>
  Standard_EXPORT   STEPCAFControl_Reader();
  //! Creates a reader tool and attaches it to an already existing Session <br>
//! 	    Clears the session if it was not yet set for STEP <br>
  Standard_EXPORT   STEPCAFControl_Reader(const Handle(XSControl_WorkSession)& WS,const Standard_Boolean scratch = Standard_True);
  //! Clears the internal data structures and attaches to a new session <br>
//!          Clears the session if it was not yet set for STEP <br>
  Standard_EXPORT     void Init(const Handle(XSControl_WorkSession)& WS,const Standard_Boolean scratch = Standard_True) ;
  //! Loads a file and returns the read status <br>
//!          Provided for use like single-file reader <br>
  Standard_EXPORT     IFSelect_ReturnStatus ReadFile(const Standard_CString filename) ;
  //! Returns number of roots recognized for transfer <br>
//!          Shortcut for Reader().NbRootsForTransfer() <br>
  Standard_EXPORT     Standard_Integer NbRootsForTransfer() ;
  //! Translates currently loaded STEP file into the document <br>
//!          Returns True if succeeded, and False in case of fail <br>
//!          Provided for use like single-file reader <br>
  Standard_EXPORT     Standard_Boolean TransferOneRoot(const Standard_Integer num,Handle(TDocStd_Document)& doc) ;
  //! Translates currently loaded STEP file into the document <br>
//!          Returns True if succeeded, and False in case of fail <br>
//!          Provided for use like single-file reader <br>
  Standard_EXPORT     Standard_Boolean Transfer(Handle(TDocStd_Document)& doc) ;
  
  Standard_EXPORT     Standard_Boolean Perform(const TCollection_AsciiString& filename,Handle(TDocStd_Document)& doc) ;
  //! Translate STEP file given by filename into the document <br>
//!          Return True if succeeded, and False in case of fail <br>
  Standard_EXPORT     Standard_Boolean Perform(const Standard_CString filename,Handle(TDocStd_Document)& doc) ;
  //! Returns data on external files <br>
//!          Returns Null handle if no external files are read <br>
  Standard_EXPORT    const Handle_STEPCAFControl_DictionaryOfExternFile& ExternFiles() const;
  //! Returns data on external file by its name <br>
//!          Returns False if no external file with given name is read <br>
  Standard_EXPORT     Standard_Boolean ExternFile(const Standard_CString name,Handle(STEPCAFControl_ExternFile)& ef) const;
  //! Returns basic reader <br>
  Standard_EXPORT     STEPControl_Reader& ChangeReader() ;
  //! Returns basic reader as const <br>
  Standard_EXPORT    const STEPControl_Reader& Reader() const;
  //! Returns label of instance of an assembly component <br>
//!          corresponding to a given NAUO <br>
  Standard_EXPORT   static  TDF_Label FindInstance(const Handle(StepRepr_NextAssemblyUsageOccurrence)& NAUO,const Handle(XCAFDoc_ShapeTool)& STool,const STEPConstruct_Tool& Tool,const STEPCAFControl_DataMapOfPDExternFile& PDRFileMap,const XCAFDoc_DataMapOfShapeLabel& ShapeLabelMap) ;
  //! Set ColorMode for indicate read Colors or not. <br>
  Standard_EXPORT     void SetColorMode(const Standard_Boolean colormode) ;
  
  Standard_EXPORT     Standard_Boolean GetColorMode() const;
  //! Set NameMode for indicate read Name or not. <br>
  Standard_EXPORT     void SetNameMode(const Standard_Boolean namemode) ;
  
  Standard_EXPORT     Standard_Boolean GetNameMode() const;
  //! Set LayerMode for indicate read Layers or not. <br>
  Standard_EXPORT     void SetLayerMode(const Standard_Boolean layermode) ;
  
  Standard_EXPORT     Standard_Boolean GetLayerMode() const;
  //! PropsMode for indicate read Validation properties or not. <br>
  Standard_EXPORT     void SetPropsMode(const Standard_Boolean propsmode) ;
  
  Standard_EXPORT     Standard_Boolean GetPropsMode() const;
  //! Set SHUO mode for indicate write SHUO or not. <br>
  Standard_EXPORT     void SetSHUOMode(const Standard_Boolean shuomode) ;
  
  Standard_EXPORT     Standard_Boolean GetSHUOMode() const;
  //! Set GDT mode for indicate write GDT or not. <br>
  Standard_EXPORT     void SetGDTMode(const Standard_Boolean gdtmode) ;
  
  Standard_EXPORT     Standard_Boolean GetGDTMode() const;
  //! Set Material mode <br>
  Standard_EXPORT     void SetMatMode(const Standard_Boolean matmode) ;
  
  Standard_EXPORT     Standard_Boolean GetMatMode() const;





protected:

  //! Translates STEP file already loaded into the reader <br>
//!          into the document <br>
//!          If num==0, translates all roots, else only root number num <br>
//!          Returns True if succeeded, and False in case of fail <br>
//!          If asOne is True, in case of multiple results composes <br>
//!          them into assembly. Fills sequence of produced labels <br>
  Standard_EXPORT     Standard_Boolean Transfer(STEPControl_Reader& rd,const Standard_Integer num,Handle(TDocStd_Document)& doc,TDF_LabelSequence& Lseq,const Standard_Boolean asOne = Standard_False) ;
  //! Add a shape to a document <br>
//!          Depending on a case, this shape can be added as one, or <br>
//!          as assembly, or (in case if it is associated with external <br>
//!          reference) taken as that referred shape <br>
  Standard_EXPORT     TDF_Label AddShape(const TopoDS_Shape& S,const Handle(XCAFDoc_ShapeTool)& STool,const TopTools_MapOfShape& NewShapesMap,const STEPCAFControl_DataMapOfShapePD& ShapePDMap,const STEPCAFControl_DataMapOfPDExternFile& PDFileMap,XCAFDoc_DataMapOfShapeLabel& ShapeLabelMap) const;
  //! Reads (or if returns already read) extern file with <br>
//!          given name <br>
  Standard_EXPORT     Handle_STEPCAFControl_ExternFile ReadExternFile(const Standard_CString file,const Standard_CString fullpath,Handle(TDocStd_Document)& doc) ;
  //! Reads style assignments from STEP model and sets <br>
//!          corresponding color assignments in the DECAF document <br>
  Standard_EXPORT     Standard_Boolean ReadColors(const Handle(XSControl_WorkSession)& WS,Handle(TDocStd_Document)& doc,const STEPCAFControl_DataMapOfPDExternFile& PDFileMap,const XCAFDoc_DataMapOfShapeLabel& ShapeLabelMap) const;
  //! Reads names of parts defined in the STEP model and <br>
//!          assigns them to corresponding labels in the DECAF document <br>
  Standard_EXPORT     Standard_Boolean ReadNames(const Handle(XSControl_WorkSession)& WS,Handle(TDocStd_Document)& doc,const STEPCAFControl_DataMapOfPDExternFile& PDFileMap,const XCAFDoc_DataMapOfShapeLabel& ShapeLabelMap) const;
  //! Reads validation properties assigned to shapes in the STEP <br>
//!          model and assigns them to corresponding labels in the DECAF <br>
//!          document <br>
  Standard_EXPORT     Standard_Boolean ReadValProps(const Handle(XSControl_WorkSession)& WS,Handle(TDocStd_Document)& doc,const STEPCAFControl_DataMapOfPDExternFile& PDFileMap,const XCAFDoc_DataMapOfShapeLabel& ShapeLabelMap) const;
  //! Reads layers of parts defined in the STEP model and <br>
//!          set reference between shape and layers in the DECAF document <br>
  Standard_EXPORT     Standard_Boolean ReadLayers(const Handle(XSControl_WorkSession)& WS,Handle(TDocStd_Document)& doc) const;
  //! Reads SHUO for instances defined in the STEP model and <br>
//!          set reference between shape instances from different assemblyes <br>
  Standard_EXPORT     Standard_Boolean ReadSHUOs(const Handle(XSControl_WorkSession)& WS,Handle(TDocStd_Document)& doc,const STEPCAFControl_DataMapOfPDExternFile& PDFileMap,const XCAFDoc_DataMapOfShapeLabel& ShapeLabelMap) const;
  //! Reads D&GT for instances defined in the STEP model and <br>
//!          set reference between shape instances from different assemblyes <br>
  Standard_EXPORT     Standard_Boolean ReadGDTs(const Handle(XSControl_WorkSession)& WS,Handle(TDocStd_Document)& doc) const;
  //! Reads materials for instances defined in the STEP model and <br>
//!          set reference between shape instances from different assemblyes <br>
  Standard_EXPORT     Standard_Boolean ReadMaterials(const Handle(XSControl_WorkSession)& WS,Handle(TDocStd_Document)& doc,const Handle(TColStd_HSequenceOfTransient)& SeqPDS) const;
  //! Populates the sub-Label of the passed TDF Label with shape <br>
//!          data associated with the given STEP Representation Item, <br>
//!          including naming and topological information. <br>
  Standard_EXPORT     TDF_Label SettleShapeData(const Handle(StepRepr_RepresentationItem)& theItem,TDF_Label& theLab,const Handle(XCAFDoc_ShapeTool)& theShapeTool,const Handle(Transfer_TransientProcess)& theTP) const;
  //! Given the maps of already translated shapes, this method <br>
//!          expands their correspondent Labels in XDE Document so that <br>
//!          to have a dedicated sub-Label for each sub-shape coming <br>
//!          with associated name in its STEP Representation Item. <br>
  Standard_EXPORT     void ExpandSubShapes(const Handle(XCAFDoc_ShapeTool)& theShapeTool,const XCAFDoc_DataMapOfShapeLabel& theShapeLabelMap,const STEPCAFControl_DataMapOfShapePD& theShapePDMap) const;
  //! Expands the topological structure of Manifold Solid BRep <br>
//!           STEP entity to OCAF sub-tree. Entities having no names <br>
//!           associated via their Representation Items are skipped. <br>
  Standard_EXPORT     void ExpandManifoldSolidBrep(TDF_Label& theLab,const Handle(StepRepr_RepresentationItem)& theItem,const Handle(Transfer_TransientProcess)& theTP,const Handle(XCAFDoc_ShapeTool)& theShapeTool) const;
  //! Expands the topological structure of Shell-Based Surface <br>
//!           Model STEP entity to OCAF sub-tree. Entities having no names <br>
//!           associated via their Representation Items are skipped. <br>
  Standard_EXPORT     void ExpandSBSM(TDF_Label& theLab,const Handle(StepRepr_RepresentationItem)& theItem,const Handle(Transfer_TransientProcess)& theTP,const Handle(XCAFDoc_ShapeTool)& theShapeTool) const;
  //! Expands STEP Shell structure to OCAF sub-tree. Entities <br>
//!           having no names associated via their Representation Items <br>
//!           are skipped. <br>
  Standard_EXPORT     void ExpandShell(const Handle(StepShape_ConnectedFaceSet)& theShell,TDF_Label& theLab,const Handle(Transfer_TransientProcess)& theTP,const Handle(XCAFDoc_ShapeTool)& theShapeTool) const;




private:



STEPControl_Reader myReader;
Handle_STEPCAFControl_DictionaryOfExternFile myFiles;
Standard_Boolean myColorMode;
Standard_Boolean myNameMode;
Standard_Boolean myLayerMode;
Standard_Boolean myPropsMode;
Standard_Boolean mySHUOMode;
Standard_Boolean myGDTMode;
Standard_Boolean myMatMode;


};





// other Inline functions and methods (like "C++: function call" methods)


#endif
