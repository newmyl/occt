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

#ifndef TreeModel_VisibilityState_H
#define TreeModel_VisibilityState_H

#include <inspector/TreeModel.hxx>
#include <inspector/TreeModel_ModelBase.hxx>
#include <inspector/TreeModel_ColumnType.hxx>

#include <Standard.hxx>
#include <Standard_Macro.hxx>

#include <QObject>
#include <QString>

//! \class TreeModel_VisibilityState
//! \brief Interface that provides connection between model and visualization control to:
//! - know whether the model item is visible
//! - change visibility of the model item
class TREEMODEL_EXPORT TreeModel_VisibilityState : public QObject
{
  Q_OBJECT
public:
  //! Constructor
  TreeModel_VisibilityState (TreeModel_ModelBase* theModel) : myModel (theModel) {}

  //! Destructor
  ~TreeModel_VisibilityState() {}

  //! Returns true if visibility of the item can be changed
  //! \param theIndex tree model index
  //! \return boolean value
  virtual bool CanBeVisible (const QModelIndex& theIndex) const = 0;

  //! Sets visibility state
  //! \param theIndex tree model index
  //! \param theState visibility state
  //! \return true if state is changed
  virtual bool SetVisible (const QModelIndex& theIndex, const bool theState) = 0;

  //! Returns visibility state value
  //! \param theIndex tree model index
  //! \return boolean value
  virtual bool IsVisible (const QModelIndex& theIndex) const = 0;

protected slots:
  //! Processes the mouse clicked on the index.
  //! It changes the item visibility if model allows to change it.
  //! \theIndex tree model index
  void OnClicked (const QModelIndex& theIndex);

private:
  TreeModel_ModelBase* myModel; //! tree view model
};

#endif
