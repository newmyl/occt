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

#ifndef _Transfer_ResultFromModel_HeaderFile
#define _Transfer_ResultFromModel_HeaderFile

#include <Standard.hxx>
#include <Standard_DefineHandle.hxx>
#include <Handle_Interface_InterfaceModel.hxx>
#include <TCollection_AsciiString.hxx>
#include <Standard_Integer.hxx>
#include <Interface_CheckStatus.hxx>
#include <MMgt_TShared.hxx>
#include <Standard_CString.hxx>
#include <Standard_Boolean.hxx>
#include <Transfer_ResultFromTransient.hxx>
#include <Transfer_TransientProcess.hxx>
#include <Handle_Standard_Transient.hxx>
#include <Handle_TColStd_HSequenceOfTransient.hxx>
#include <Handle_MMgt_TShared.hxx>
class Interface_InterfaceModel;
class Transfer_ResultFromTransient;
class Transfer_TransientProcess;
class Standard_Transient;
class TColStd_HSequenceOfTransient;
class Interface_CheckIterator;
class Handle(Standard_Type);
class Handle(MMgt_TShared);
class Transfer_ResultFromModel;

DEFINE_STANDARD_HANDLE(Transfer_ResultFromModel,MMgt_TShared)

//! ResultFromModel is used to store a final result stored in a <br>
//!           TransientProcess, respectfully to its structuration in scopes <br>
//!           by using a set of ResultFromTransient <br>
//!           Hence, it can be regarded as a passive equivalent of the <br>
//!           stored data in the TransientProcess, while an Iterator gives <br>
//!           a flat view of it. <br>
//! <br>
//!           A ResultFromModel is intended to be attached to the transfer <br>
//!           of one entity (typically root entity but it is not mandatory) <br>
//! <br>
//!           It is then possible to : <br>
//!           - Create and fill a ResultFromModel from a TransientProcess, <br>
//!             by designating a starting entity <br>
//!           - Fill back the TransientProcess from a ResultFromModel, as it <br>
//!             were filled by the operation which filled it the first time <br>
class Transfer_ResultFromModel : public MMgt_TShared {

public:

  //! Creates a ResultFromModel, empty <br>
  Standard_EXPORT   Transfer_ResultFromModel();
  //! Sets starting Model <br>
  Standard_EXPORT     void SetModel(const Handle(Interface_InterfaceModel)& model) ;
  //! Sets starting File Name <br>
  Standard_EXPORT     void SetFileName(const Standard_CString filename) ;
  //! Returns starting Model (null if not set) <br>
  Standard_EXPORT     Handle_Interface_InterfaceModel Model() const;
  //! Returns starting File Name (empty if not set) <br>
  Standard_EXPORT     Standard_CString FileName() const;
  //! Fills from a TransientProcess, with the result attached to <br>
//!           a starting entity. Considers its Model if it is set. <br>
//!           This action produces a structured set of ResultFromTransient, <br>
//!           considering scopes, starting by that of <ent>. <br>
//!           If <ent> has no recorded result, it remains empty <br>
//!           Returns True if a result is recorded, False else <br>
  Standard_EXPORT     Standard_Boolean Fill(const Handle(Transfer_TransientProcess)& TP,const Handle(Standard_Transient)& ent) ;
  //! Clears some data attached to binders used by TransientProcess, <br>
//!           which become useless once the transfer has been done, <br>
//!           by calling Strip on its ResultFromTransient <br>
//! <br>
//!           mode = 0 : minimum, clears data remaining from TransferProcess <br>
//!           mode = 10 : just keeps file name, label, check status ..., <br>
//!             and MainResult but only the result (Binder) <br>
//!           mode = 11 : also clears MainResult (status and names remain) <br>
  Standard_EXPORT     void Strip(const Standard_Integer mode) ;
  //! Fills back a TransientProcess from the structured set of <br>
//!           binders. Also sets the Model. <br>
  Standard_EXPORT     void FillBack(const Handle(Transfer_TransientProcess)& TP) const;
  //! Returns True if a Result is recorded <br>
  Standard_EXPORT     Standard_Boolean HasResult() const;
  //! Returns the main recorded ResultFromTransient, or a null <br>
  Standard_EXPORT     Handle_Transfer_ResultFromTransient MainResult() const;
  //! Sets a new value for the main recorded ResultFromTransient <br>
  Standard_EXPORT     void SetMainResult(const Handle(Transfer_ResultFromTransient)& amain) ;
  //! Returns the label in starting model attached to main entity <br>
//!           (updated by Fill or SetMainResult, if Model is known) <br>
  Standard_EXPORT     Standard_CString MainLabel() const;
  //! Returns the label in starting model attached to main entity <br>
  Standard_EXPORT     Standard_Integer MainNumber() const;
  //! Searches for a key (starting entity) and returns its result <br>
//!           Returns a null handle if not found <br>
  Standard_EXPORT     Handle_Transfer_ResultFromTransient ResultFromKey(const Handle(Standard_Transient)& start) const;
  //! Internal method which returns the list of ResultFromTransient, <br>
//!           according level (2:complete; 1:sub-level 1; 0:main only) <br>
  Standard_EXPORT     Handle_TColStd_HSequenceOfTransient Results(const Standard_Integer level) const;
  //! Returns the list of recorded starting entities, ending by the <br>
//!           root. Entities with check but no transfer result are ignored <br>
//!           <level> = 2 (D), considers the complete list <br>
//!           <level> = 1      considers the main result plus immediate subs <br>
//!           <level> = 0      just the main result <br>
  Standard_EXPORT     Handle_TColStd_HSequenceOfTransient TransferredList(const Standard_Integer level = 2) const;
  //! Returns the list of starting entities to which a check status <br>
//!           is attached. <br>
//!           <check> = -2 , all entities whatever the check (see result) <br>
//!           <check> = -1 , entities with no fail (warning allowed) <br>
//!           <check> =  0 , entities with no check at all <br>
//!           <check> =  1 , entities with warning but no fail <br>
//!           <check> =  2 , entities with fail <br>
//!           <result> : if True, only entities with an attached result <br>
//!           Remark : result True and check=0 will give an empty list <br>
  Standard_EXPORT     Handle_TColStd_HSequenceOfTransient CheckedList(const Interface_CheckStatus check,const Standard_Boolean result) const;
  //! Returns the check-list of this set of results <br>
//!           <erronly> true : only fails are considered <br>
//!           <level> = 0 : considers only main binder <br>
//!           <level> = 1 : considers main binder plus immediate subs <br>
//!           <level> = 2 (D) : considers all checks <br>
  Standard_EXPORT     Interface_CheckIterator CheckList(const Standard_Boolean erronly,const Standard_Integer level = 2) const;
  //! Returns the check status with corresponds to the content <br>
//!           of this ResultFromModel; considers all levels of transfer <br>
//!           (worst status). Returns CheckAny if not yet computed <br>
//!           Reads it from recorded status if already computed, else <br>
//!           recomputes one <br>
  Standard_EXPORT     Interface_CheckStatus CheckStatus() const;
  //! Computes and records check status (see CheckStatus) <br>
//!           Does not computes it if already done and <enforce> False <br>
  Standard_EXPORT     Interface_CheckStatus ComputeCheckStatus(const Standard_Boolean enforce) ;

  DEFINE_STANDARD_RTTI(Transfer_ResultFromModel)

private: 


Handle_Interface_InterfaceModel themodel;
TCollection_AsciiString thename;
Handle_Transfer_ResultFromTransient themain;
TCollection_AsciiString themlab;
Standard_Integer themnum;
Interface_CheckStatus themchk;


};
#endif
