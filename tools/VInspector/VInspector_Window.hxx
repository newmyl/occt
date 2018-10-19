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

#ifndef VInspector_Window_H
#define VInspector_Window_H

#include <AIS_InteractiveContext.hxx>
#include <NCollection_List.hxx>
#include <SelectBasics_EntityOwner.hxx>
#include <Standard.hxx>

#include <inspector/TInspectorAPI_PluginParameters.hxx>
#include <inspector/VInspector_CallBack.hxx>
#include <inspector/VInspector_DisplayActionType.hxx>

#include <Standard_WarningsDisable.hxx>
#include <QObject>
#include <QItemSelection>
#include <QMainWindow>
#include <Standard_WarningsRestore.hxx>

class OpenGl_Element;

class VInspector_PrsOpenGlElement;

class ViewControl_MessageDialog;
class ViewControl_PropertyView;

class VInspector_ToolBar;
class View_Window;

class QAbstractItemModel;
class QAction;
class QDockWidget;
class QTreeView;
class QWidget;

//! \class VInspector_Window
//! Window that unites all VInspector controls.
class VInspector_Window : public QObject
{
  Q_OBJECT

public:

  //! Constructor
  Standard_EXPORT VInspector_Window();

  //! Destructor
  virtual ~VInspector_Window() Standard_OVERRIDE {}

  //! Provides the container with a parent where this container should be inserted.
  //! If Qt implementation, it should be QWidget with QLayout set inside
  //! \param theParent a parent class
  Standard_EXPORT void SetParent (void* theParent);

  //! Sets parameters container, it should be used when the plugin is initialized or in update content
  //! \param theParameters a parameters container
  void SetParameters (const Handle(TInspectorAPI_PluginParameters)& theParameters) { myParameters = theParameters; }

  //! Provide container for actions available in inspector on general level
  //! \param theMenu if Qt implementation, it is QMenu object
  Standard_EXPORT void FillActionsMenu (void* theMenu);

  //! Returns plugin preferences: dock widgets state, tree view columns.
  //! \param theItem container of preference elements
  Standard_EXPORT void GetPreferences (TInspectorAPI_PreferencesDataMap& theItem);

  //! Applies plugin preferences
  //! \param theItem container of preference elements
  Standard_EXPORT void SetPreferences (const TInspectorAPI_PreferencesDataMap& theItem);

  //! Applyes parameters to Init controls, opens files if there are in parameters, updates OCAF tree view model
  Standard_EXPORT void UpdateContent();

  //! Returns main control
  QWidget* GetMainWindow() const { return myMainWindow; }

  //! Returns presentations of selected items in tree model
  //! \param theModel selection model
  //! \return container of presentations
  NCollection_List<Handle(AIS_InteractiveObject)> GetSelectedPresentations (QItemSelectionModel* theModel);

  //! Returns selected shapes
  //! \param theModel selection model
  //! \return container of shapes
  NCollection_List<TopoDS_Shape> GetSelectedShapes (QItemSelectionModel* theModel);

  //! Returns selected elements
  //! \param theModel selection model
  //! \return container of OpenGl elements
  NCollection_List<OpenGl_Element*> GetSelectedElements (QItemSelectionModel* theModel);

private:

  //! Fills controls of the plugin by parameters:
  //! - Fine AIS_InteractiveObject and fills View if it if it differs from the current context
  //! \param theParameters a parameters container
  bool Init (const NCollection_List<Handle(Standard_Transient)>& theParameters);

  //! Read BREP file, creates AIS presentation for the shape and visualize it in the current context
  //! \param theFileName a name of BREP file
  bool OpenFile (const TCollection_AsciiString& theFileName);

private slots:

  //! Shows context menu for tree view selected item. It contains clear view or BREP operations items
  //! \param thePosition a clicked point
  void onTreeViewContextMenuRequested(const QPoint& thePosition);

  //! Performs the functionality of the clicked action
  //! \param theActionId an action identifier in tool bar
  void onToolBarActionClicked (const int theActionId);

  //! Display content of selected tree view item if isToggled is true
  //! \param isToggled true if the property dock widget is shown
  void onPropertyPanelShown (bool isToggled);

  //! Update presentation of the selected tree view item using information about selection in property view
  //! \param theSelected container of selected table cells
  //! \param theDeselected container of selected table cells
  void onPropertyViewSelectionChanged();

  //! Synchronization selection between history and tree view. Selection by history view
  //! \param theSelected a selected items
  //! \param theDeselected a deselected items
  void onHistoryViewSelectionChanged (const QItemSelection& theSelected,
                                      const QItemSelection& theDeselected);

  //! Processes selection in tree view: make presentation or owner selected in the context if corresponding
  //! check box is checked
  //! \param theSelected a selected items
  //! \param theDeselected a deselected items
  void onTreeViewSelectionChanged (const QItemSelection& theSelected, const QItemSelection& theDeselected);

  //! Exports the first selected shape into ShapeViewer plugin.
  void onExportToShapeView();

  //! Apply activated display action
  void onDisplayActionTypeClicked();

private:

  //! Inits the window content by the given context
  //! \param theContext a context
  void SetContext (const Handle(AIS_InteractiveContext)& theContext);

  //! Updates tree model
  void UpdateTreeModel();

  //! Updates property panel content by item selected in tree view.
  void updatePropertyPanelBySelection();

  //! Set selected in tree view presentations displayed or erased in the current context. Note that erased presentations
  //! still belongs to the current context until Remove is called.
  //! \param theType display action type
  void displaySelectedPresentations (const VInspector_DisplayActionType theType);

  //! Creates an istance of 3D view to initialize context.
  //! \return a context of created view.
  Handle(AIS_InteractiveContext) createView();

  //!< Updates presentation of preview for parameter shapes. Creates a compound of the shapes
  //!< \param theShape container of shapes
  void updatePreviewPresentation (const NCollection_List<TopoDS_Shape>& theShapes);

  //!< Updates presentation of preview for OpenGl elements.
  //!< \param theElements container of elements
  void updatePreviewPresentation (const NCollection_List<OpenGl_Element*>& theElements);

private:

  QWidget* myParent; //!< widget, comes when Init window, the window control lays in the layout, updates window title

  QMainWindow* myMainWindow; //!< main control
  VInspector_ToolBar* myToolBar; //!< tool bar actions

  QDockWidget* myPropertyPanelWidget; //!< property pane dockable widget
  ViewControl_PropertyView* myPropertyView; //!< property control to display model item values if exist

  QTreeView* myTreeView; //!< tree view of AIS content
  QTreeView* myHistoryView; //!< history of AIS context calls
  Handle(VInspector_CallBack) myCallBack; //!< AIS context call back, if set

  ViewControl_MessageDialog* myExportToShapeViewDialog; //!< dialog about exporting TopoDS_Shape to ShapeView plugin
  View_Window* myViewWindow; //!< temporary view window, it is created if Open is called but context is still NULL

  Handle(TInspectorAPI_PluginParameters) myParameters; //!< plugins parameters container
  Handle(AIS_InteractiveObject) myPreviewPresentation; //!< presentation of preview for a selected object
  Handle(VInspector_PrsOpenGlElement) myOpenGlPreviewPresentation; //!< presentation of preview for OpenGl elements
};

#endif
