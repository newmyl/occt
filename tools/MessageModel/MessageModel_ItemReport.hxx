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

#ifndef MessageModel_ItemReport_H
#define MessageModel_ItemReport_H

#include <inspector/MessageModel_ItemBase.hxx>
#include <Standard.hxx>
#include <TCollection_AsciiString.hxx>
#include <Message_Report.hxx>

#include <QMap>
#include <QVariant>

class MessageModel_ItemReport;
typedef QExplicitlySharedDataPointer<MessageModel_ItemReport> MessageModel_ItemReportPtr;

//! \class MessageModel_ItemReport
//! This item is connected to Message_Alert.
//! Parent is MessageModel_ItemRoot, children are MessageModel_ItemAlert or no children
class MessageModel_ItemReport : public MessageModel_ItemBase
{
public:

  //! Creates an item wrapped by a shared pointer
  //! \param theRow the item row positition in the parent item
  //! \param theColumn the item column positition in the parent item
  //! \return the pointer to the created item
  static MessageModel_ItemReportPtr CreateItem (TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  { return MessageModel_ItemReportPtr (new MessageModel_ItemReport (theParent, theRow, theColumn)); }

  //! Destructor
  virtual ~MessageModel_ItemReport() Standard_OVERRIDE {};

  //! Returns the current shape
  const Handle(Message_Report)& GetReport() const { return myReport; }

  const TCollection_AsciiString& GetDescription() const { return myDescription; }

  //! Inits the item, fills internal containers
  Standard_EXPORT virtual void Init() Standard_OVERRIDE;

  //! Resets cached values
  Standard_EXPORT virtual void Reset() Standard_OVERRIDE;

  //! Return data value for the role.
  //! \param theRole a value role
  //! \return the value
  Standard_EXPORT virtual QVariant initValue (const int theRole) const Standard_OVERRIDE;

  //! \return number of children.
  Standard_EXPORT virtual int initRowCount() const Standard_OVERRIDE;

  //! Returns report of the tree model item. Iterates up by parents intil the report item is found.
  //! \return an item or NULL
  Standard_EXPORT static MessageModel_ItemReportPtr FindReportItem (const TreeModel_ItemBasePtr& theItem);

  //! Returns summ of children alert elapsed times. The method is recusive.
  //! \param theAlert a message alert
  //! \return double value
  Standard_EXPORT static double AmountElapsedTime (const Handle(Message_Report)& theReport);

protected:

  //! Initialize the current item. It is empty because Reset() is also empty.
  virtual void initItem() const Standard_OVERRIDE;

  //! Creates a child item in the given position.
  //! \param theRow the child row position
  //! \param theColumn the child column position
  //! \return the created item
  virtual TreeModel_ItemBasePtr createChild (int theRow, int theColumn) Standard_OVERRIDE;

  //! Returns number of child shapes. Init item if it is not initialized
  //! \return integer value
  int getRowCount() const;

  //! Returns current shape, initialized item if it has not been initialized yet
  //! \return shape value
  const Handle(Message_Report)& getReport() const;

private:

  //! Constructor
  MessageModel_ItemReport (TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  : MessageModel_ItemBase (theParent, theRow, theColumn) {}

private:

  Handle(Message_Report) myReport; //!< current report
  TCollection_AsciiString myDescription; //!< description
};

#endif
