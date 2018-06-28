// Created on: 2017-06-16
// Created by: Natalia ERMOLAEVA
// Copyright (c) 2017 OPEN CASCADE SAS
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

#ifndef MessageModel_ItemRoot_H
#define MessageModel_ItemRoot_H

#include <Message_Alert.hxx>
#include <Message_Report.hxx>
#include <NCollection_List.hxx>
#include <Standard.hxx>
#include <TCollection_AsciiString.hxx>

#include <inspector/MessageModel_ItemBase.hxx>

class MessageModel_ItemRoot;
typedef QExplicitlySharedDataPointer<MessageModel_ItemRoot> MessageModel_ItemRootPtr;

//! \class MessageModel_ItemRoot
//! Collects message reports that should be visualized in tree view. Reports are cached and if reports are not needed,
//! cache should be cleared using RemoveAllReports
//! Parent is NULL, children are MessageModel_ItemReport items.
class MessageModel_ItemRoot : public MessageModel_ItemBase
{
private:
  //! \struct to extend report by description
  struct MessageModel_ReportInformation
  {
    //! Constructor
    MessageModel_ReportInformation (Handle(Message_Report) theReport, const TCollection_AsciiString& theDescription)
      : myReport (theReport), myDescription (theDescription) {}

    Handle(Message_Report) myReport; //! report
    TCollection_AsciiString myDescription; //! report description
  };
public:

  //! Creates an item wrapped by a shared pointer
  static MessageModel_ItemRootPtr CreateItem (TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
    { return MessageModel_ItemRootPtr (new MessageModel_ItemRoot (theParent, theRow, theColumn)); }

  //! Destructor
  virtual ~MessageModel_ItemRoot() Standard_OVERRIDE {};

  //! Appends new report
  //! \param theReport a report instance
  //! \param theReportDescription an additional report information
  void AddReport (const Handle(Message_Report)& theReport, const TCollection_AsciiString& theReportDescription)
  { myReports.Append (MessageModel_ReportInformation (theReport, theReportDescription)); }

  //! Set report, se it into the given row index
  //! \param theRowId a report child row
  //! \param theReport a report instance
  //! \param theReportDescription an additional report information
  Standard_EXPORT void SetReport (const int theRowId, const Handle(Message_Report)& theReport,
    const TCollection_AsciiString& theReportDescription = "");

  //! Clears internal container of added reports
  void RemoveAllReports() { myReports.Clear(); }

  //! Returns report by the number
  //! \param theRowId an index of the report in the internal container.
  Standard_EXPORT const Handle(Message_Report)& GetReport (const int theRowId, TCollection_AsciiString& theReportDescription);

  //! Set the view reversed. If reversed, the last report alert is upper item in the tree view
  //! \param theReversed boolean flag
  void SetReversed (const Standard_Boolean& theReversed) { myIsReversed = theReversed; }

  //! Return whether the view is reversed or not
  //! \return boolean value
  Standard_Boolean IsReversed() const { return myIsReversed; };

  //! Set the view reversed. If reversed, the last report alert is upper item in the tree view
  //! \param theReversed boolean flag
  void SetUniteAlerts (const Standard_Boolean& theUniteAlerts) { myIsUniteAlerts = theUniteAlerts; }

  //! Return whether the view is reversed or not
  //! \return boolean value
  Standard_Boolean IsUniteAlerts() const { return myIsUniteAlerts; };

  //! Set the view reversed. If reversed, the last report alert is upper item in the tree view
  //! \param theReversed boolean flag
  void SetName (const TCollection_AsciiString& theName) { myName = theName; }

protected:

  //! Return data value for the role.
  //! \param theRole a value role
  //! \return the value
  virtual QVariant initValue (const int theRole) const Standard_OVERRIDE;

  //! \return number of children.
  virtual int initRowCount() const Standard_OVERRIDE { return myReports.Size(); }

  //! Creates a child item in the given position.
  //! \param theRow the child row position
  //! \param theColumn the child column position
  //! \return the created item
  virtual TreeModel_ItemBasePtr createChild (int theRow, int theColumn) Standard_OVERRIDE;

private:

  //! Constructor
  //! param theParent a parent item
  MessageModel_ItemRoot (TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  : MessageModel_ItemBase (theParent, theRow, theColumn), myIsReversed (Standard_False) {}

private:

  NCollection_List<MessageModel_ReportInformation> myReports; //!< reports sent by algorithms
  Standard_Boolean myIsReversed; //!< state if the model is reversed
  Standard_Boolean myIsUniteAlerts; //!< state whether equal named alerts should be united in a folder
  TCollection_AsciiString myName; //!< DisplayRole data, if defined
};

#endif
