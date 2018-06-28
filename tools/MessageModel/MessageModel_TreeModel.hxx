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

#ifndef MessageModel_TreeModel_H
#define MessageModel_TreeModel_H

#include <inspector/MessageModel_ItemBase.hxx>
#include <Standard.hxx>
#include <TCollection_AsciiString.hxx>
#include <Message_Report.hxx>
#include <inspector/TreeModel_ModelBase.hxx>

#include <QMap>
#include <QObject>

class MessageModel_TreeModel;

//! \class MessageModel_TreeModel
//! View model to visualize MessageReport/s content
class MessageModel_TreeModel : public TreeModel_ModelBase
{
public:

  //! Constructor
  Standard_EXPORT MessageModel_TreeModel (QObject* theParent);

  //! Destructor
  virtual ~MessageModel_TreeModel() Standard_OVERRIDE {};

  //! Add shape, append it to the model root item
  //! \param theReport a report instance
  //! \param theReportDescription an additional report information
  Standard_EXPORT void AddReport (const Handle(Message_Report)& theReport,
    const TCollection_AsciiString& theReportDescription = "");

  //! Set report, se it into the given row index
  //! \param theRowId a report child row
  //! \param theReport a report instance
  //! \param theReportDescription an additional report information
  Standard_EXPORT void SetReport (const int theRowId, const Handle(Message_Report)& theReport,
    const TCollection_AsciiString& theReportDescription = "");

  //! Set the view reversed. If reversed, the last report alert is upper item in the tree view
  //! \param theReversed boolean flag
  Standard_EXPORT void SetReversed (const Standard_Boolean& theReversed);
  
  //! Return whether the view is reversed or not
  //! \return boolean value
  Standard_Boolean IsReversed() const { return myIsReversed; };

  //! Set union of alerts having equal message in additional folder item.
  //! \param theUniteAlerts boolean flag
  Standard_EXPORT void SetUniteAlerts (const Standard_Boolean& theUniteAlerts);
  
  //! Return whether the alerts should be united
  //! \return boolean value
  Standard_Boolean IsUniteAlerts() const { return myIsUniteAlerts; };

  //! Sets the text value of the Root item, only "Name" column accepts the parameter value
  //! \theName visulized text of root item
  Standard_EXPORT void SetRootItemName (const TCollection_AsciiString& theName);

  //! Returns root item by column
  //! \param theColumn an index of the column
  //! \return root item instance
  virtual TreeModel_ItemBasePtr RootItem (const int theColumn) const Standard_OVERRIDE
  { return myRootItems.contains (theColumn) ? myRootItems[theColumn] : TreeModel_ItemBasePtr(); }

  //! Updates tree model
  Standard_EXPORT void UpdateTreeModel();

protected:
  //! Creates root item
  //! \param theColumnId index of a column
  virtual void createRootItem (const int theColumnId) Standard_OVERRIDE;

private:
  QMap<int, TreeModel_ItemBasePtr> myRootItems; //!< container of root items, for each column own root item
  Standard_Boolean myIsReversed; //!< state if the model is reversed
  Standard_Boolean myIsUniteAlerts; //!< state if alerts should be united
};

#endif
