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

#ifndef MessageModel_ItemAlert_H
#define MessageModel_ItemAlert_H

#include <inspector/MessageModel_ItemBase.hxx>
#include <Message_Alert.hxx>
#include <Message_ListOfAlert.hxx>
#include <Message_Report.hxx>
#include <Standard.hxx>
#include <TopoDS_Shape.hxx>

#include <QMap>
#include <QVariant>

#include <NCollection_Vector.hxx>

class QAbstractTableModel;

class MessageModel_ItemAlert;
typedef QExplicitlySharedDataPointer<MessageModel_ItemAlert> MessageModel_ItemAlertPtr;

//! \class MessageModel_ItemAlert
//! This item is connected to Message_Alert.
//! Parent is either MessageModel_ItemRoot or MessageModel_ItemAlert, children are MessageModel_ItemAlert or no children
class MessageModel_ItemAlert : public MessageModel_ItemBase
{
public:

  //! Creates an item wrapped by a shared pointer
  //! \param theRow the item row positition in the parent item
  //! \param theColumn the item column positition in the parent item
  //! \return the pointer to the created item
  static MessageModel_ItemAlertPtr CreateItem (TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  { return MessageModel_ItemAlertPtr (new MessageModel_ItemAlert (theParent, theRow, theColumn)); }

  //! Destructor
  virtual ~MessageModel_ItemAlert() Standard_OVERRIDE {};

  //! Returns the current shape
  const Handle(Message_Alert)& GetAlert() const { return myAlert; }

  //! Returns united alerts or empty list
  const Message_ListOfAlert& GetUnitedAlerts() const { return myUnitedAlerts; }

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

  //! Sets some shape to present the item
  //! \param theShape shape instance
  void SetCustomShape (const TopoDS_Shape& theShape) { myCustomShape = theShape; }

  //! Returns custom shape to present the item
  //! \return instance of the shape
  const TopoDS_Shape& GetCustomShape() const { return myCustomShape; }

  //! Returns summ of children alert elapsed times. The method is recusive.
  //! \param theAlert a message alert
  //! \return double value
  Standard_EXPORT static double AmountElapsedTime (const Handle(Message_Alert)& theAlert);

  //! Returns alerts united by Message Key
  //! \param theAlerts source message alert
  //! \param theUnitedAlerts arranged source message alerts
  Standard_EXPORT static void GetUnitedAlerts (const Message_ListOfAlert& theAlerts,
                                               NCollection_Vector<Message_ListOfAlert> & theUnitedAlerts);

protected:

  //! Initialize the current item. It is empty because Reset() is also empty.
  virtual void initItem() const Standard_OVERRIDE;

  //! Creates a child item in the given position.
  //! \param theRow the child row position
  //! \param theColumn the child column position
  //! \return the created item
  virtual TreeModel_ItemBasePtr createChild (int theRow, int theColumn) Standard_OVERRIDE;

  //! Returns current alert, initialized item if it has not been initialized yet
  //! \return alert value
  const Handle(Message_Alert)& getAlert() const;

  //! Returns current united alert, initialized item if it has not been initialized yet
  //! \return container of united alert value
  const Message_ListOfAlert& getUnitedAlerts() const;

  //! Returns summ of children alert elapsed times. The method is recusive.
  //! \param theAlert a message alert
  //! \return double value
  double getAmountElapsedTime() const;

private:

  //! Constructor
  MessageModel_ItemAlert (TreeModel_ItemBasePtr theParent, const int theRow, const int theColumn)
  : MessageModel_ItemBase (theParent, theRow, theColumn) {}

private:
  Handle(Message_Alert) myAlert;
  Message_ListOfAlert myUnitedAlerts;
  TopoDS_Shape myCustomShape;
};

#endif
