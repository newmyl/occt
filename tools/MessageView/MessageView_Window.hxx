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

#ifndef MessageView_Window_H
#define MessageView_Window_H

#include <Message_Report.hxx>
#include <Standard.hxx>
#include <TCollection_AsciiString.hxx>

#include <inspector/MessageModel_Actions.hxx>

#include <inspector/TInspectorAPI_PluginParameters.hxx>

#ifdef _MSC_VER
#pragma warning(disable : 4127) // conditional expression is constant
#endif
#include <QItemSelection>
#include <QList>
#include <QModelIndexList>
#include <QObject>
#include <QPoint>
#include <QString>
#include <QTreeView>

class View_Window;

class ViewControl_PropertyView;
class MessageModel_ReportCallBack;

class QDockWidget;
class QMainWindow;
class QWidget;

//! \class MessageView_Window
//! Window that unites all MessageView controls.
class MessageView_Window : public QObject
{
  Q_OBJECT
public:

  //! Constructor
  Standard_EXPORT MessageView_Window (QWidget* theParent);

  //! Destructor
  virtual ~MessageView_Window() {}

  //! Provides the container with a parent where this container should be inserted.
  //! If Qt implementation, it should be QWidget with QLayout set inside
  //! \param theParent a parent class
  Standard_EXPORT void SetParent (void* theParent);

  //! Sets parameters container, it should be used when the plugin is initialized or in update content
  //! \param theParameters a parameters container
  void SetParameters (const Handle(TInspectorAPI_PluginParameters)& theParameters)
  { myParameters = theParameters; myTreeViewActions->SetParameters (theParameters); }

  //! Sets message callback to update the current content of the view
  //! \param theCallBack
  void SetCallBack (const Handle(Message_ReportCallBack)& theCallBack) { myCallBack = theCallBack; }

  //! Provide container for actions available in inspector on general level
  //! \param theMenu if Qt implementation, it is QMenu object
  Standard_EXPORT virtual void FillActionsMenu (void* theMenu);

  //! Returns plugin preferences: dock widgets state, tree view columns.
  //! \param theItem container of preference elements
  Standard_EXPORT void GetPreferences (TInspectorAPI_PreferencesDataMap& theItem);

  //! Applies plugin preferences
  //! \param theItem container of preference elements
  Standard_EXPORT void SetPreferences (const TInspectorAPI_PreferencesDataMap& theItem);

  //! Applyes parameters to Init controls, opens files if there are in parameters, updates OCAF tree view model
  Standard_EXPORT void UpdateContent();

  //! Returns main control
  QMainWindow* GetMainWindow() const { return myMainWindow; }

  //! Returns current tree view
  QTreeView* GetTreeView() const { return myTreeView; }

protected:
  //! Appends shape into tree view model
  //! \param theShape a shape instance
  //! \param theReportDescription an additional report information
  void addReport (const Handle(Message_Report)& theReport,
                  const TCollection_AsciiString& theReportDescription = "");

private:

  //! Fills controls of the plugin by parameters:
  //! - Fine AIS_InteractiveObject and fills View if it if it differs from the current context
  //! \param theParameters a parameters container
  void Init (NCollection_List<Handle(Standard_Transient)>& theParameters);

  //! Read Shape from the file name, add Shape into tree view
  //! \param theFileName BREP file name
  void openFile (const TCollection_AsciiString& theFileName);

  //! Updates tree model
  void updateTreeModel();

protected slots:
  //! Updates property view selection in table if the item is hidden
  //! \param theIndex tree view model index
  void onTreeViewVisibilityClicked(const QModelIndex& theIndex);

  //! Udpates all controls by changed selection in OCAF tree view
  //! \param theSelected list of selected tree view items
  //! \param theDeselected list of deselected tree view items
  void onTreeViewSelectionChanged (const QItemSelection& theSelected, const QItemSelection& theDeselected);

  //! Shows context menu for tree view selected item. It contains expand/collapse actions.
  //! \param thePosition a clicked point
  void onTreeViewContextMenuRequested (const QPoint& thePosition);

  //! Display content of selected tree view item if isToggled is true
  //! \param isToggled true if the property dock widget is shown
  void onPropertyPanelShown (bool isToggled);

  //! Update presentation of the selected tree view item using information about selection in property view
  //! \param theSelected container of selected table cells
  //! \param theDeselected container of selected table cells
  void onPropertyViewSelectionChanged();

  //! Updates visibility states by erase all in context
  void onEraseAllPerformed();

  //! Export report into document
  void onExportReport();

  //! Import report into document
  void onImportReport();

  //! Reads if possible report of a selected item and updates this report in tree view
  void onReloadReport();

protected:
  //! Updates property panel content by item selected in tree view.
  void updatePropertyPanelBySelection();

private:
  QMainWindow* myMainWindow; //!< main control, parent for all MessageView controls
  QDockWidget* myViewDockWidget; //!< view dock widget to hide/show

  QDockWidget* myPropertyPanelWidget; //!< property pane dockable widget
  ViewControl_PropertyView* myPropertyView; //!< property control to display model item values if exist

  View_Window* myViewWindow; //!< OCC 3d view to visualize presentations
  QTreeView* myTreeView; //!< tree view visualized shapes
  MessageModel_Actions* myTreeViewActions; //!< processing history view actions

  Handle(TInspectorAPI_PluginParameters) myParameters; //!< plugins parameters container
  Handle(Message_ReportCallBack) myCallBack; //! < message call back to update content of the view
};

#endif
