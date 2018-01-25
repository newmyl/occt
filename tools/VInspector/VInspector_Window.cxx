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

#include <inspector/VInspector_Window.hxx>

#include <AIS_Shape.hxx>

#include <inspector/MessageModel_Actions.hxx>
#include <inspector/MessageModel_ItemReport.hxx>
#include <inspector/MessageModel_TreeModel.hxx>
#include <inspector/TreeModel_ColumnType.hxx>
#include <inspector/TreeModel_ContextMenu.hxx>
#include <inspector/TreeModel_MessageDialog.hxx>
#include <inspector/TreeModel_Tools.hxx>
#include <inspector/TreeModel_ContextMenu.hxx>

#include <inspector/VInspector_ItemPresentableObject.hxx>
#include <inspector/VInspector_ToolBar.hxx>
#include <inspector/VInspector_Tools.hxx>
#include <inspector/VInspector_ViewModel.hxx>
#include <inspector/VInspector_CallBack.hxx>
#include <inspector/VInspector_Communicator.hxx>
#include <inspector/VInspector_ItemEntityOwner.hxx>
#include <inspector/VInspector_ItemPresentableObject.hxx>
#include <inspector/VInspector_ToolBar.hxx>
#include <inspector/VInspector_Tools.hxx>
#include <inspector/VInspector_ViewModel.hxx>

#include <inspector/View_Widget.hxx>
#include <inspector/View_Window.hxx>

#include <QApplication>
#include <QDockWidget>
#include <QHeaderView>
#include <QGridLayout>
#include <QItemSelectionModel>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QToolButton>
#include <QTreeView>
#include <QWidget>

const int VINSPECTOR_DEFAULT_WIDTH  = 1250;
const int VINSPECTOR_DEFAULT_HEIGHT = 2000;//800;

const int VINSPECTOR_DEFAULT_POSITION_X = 200;
const int VINSPECTOR_DEFAULT_POSITION_Y = 60;

const int VINSPECTOR_DEFAULT_VIEW_WIDTH = 400;
const int VINSPECTOR_DEFAULT_VIEW_HEIGHT = 1000;

const int VINSPECTOR_DEFAULT_HISTORY_VIEW_WIDTH = 400;
const int VINSPECTOR_DEFAULT_HISTORY_VIEW_HEIGHT = 200;

const int VINSPECTOR_DEFAULT_VIEW_POSITION_X = 200 + 900 + 100; // TINSPECTOR_DEFAULT_POSITION_X + TINSPECTOR_DEFAULT_WIDTH + 100
const int VINSPECTOR_DEFAULT_VIEW_POSITION_Y = 60; // TINSPECTOR_DEFAULT_POSITION_Y + 50

//! \class Vinspector_TreeView
//! Extended tree view control with possibility to set predefined size.
class Vinspector_TreeView : public QTreeView
{
public:
  //! Constructor
  Vinspector_TreeView (QWidget* theParent) : QTreeView (theParent), myDefaultWidth (-1), myDefaultHeight (-1) {}

  //! Destructor
  virtual ~Vinspector_TreeView() {}

  //! Sets default size of control, that is used by the first control show
  //! \param theDefaultWidth the width value
  //! \param theDefaultHeight the height value
  void SetPredefinedSize (int theDefaultWidth, int theDefaultHeight);

  //! Returns predefined size if both values are positive, otherwise parent size hint
  virtual QSize sizeHint() const Standard_OVERRIDE;

private:

  int myDefaultWidth; //!< default width, -1 if it should not be used
  int myDefaultHeight; //!< default height, -1 if it should not be used
};

// =======================================================================
// function : SetPredefinedSize
// purpose :
// =======================================================================
void Vinspector_TreeView::SetPredefinedSize (int theDefaultWidth, int theDefaultHeight)
{
  myDefaultWidth = theDefaultWidth;
  myDefaultHeight = theDefaultHeight;
}

// =======================================================================
// function : sizeHint
// purpose :
// =======================================================================
QSize Vinspector_TreeView::sizeHint() const
{
  if (myDefaultWidth > 0 && myDefaultHeight > 0)
    return QSize (myDefaultWidth, myDefaultHeight);
  return QTreeView::sizeHint();
}

// =======================================================================
// function : Constructor
// purpose :
// =======================================================================
VInspector_Window::VInspector_Window()
: myParent (0), myExportToShapeViewDialog (0), myViewWindow (0)
{
  myMainWindow = new QMainWindow (0);

  QWidget* aCentralWidget = new QWidget (myMainWindow);
  QGridLayout* aParentLay = new QGridLayout (aCentralWidget);
  aParentLay->setContentsMargins (0, 0, 0, 0);
  aParentLay->setSpacing(0);

  // restore state of tool bar: on the bottom of the window
  myToolBar = new VInspector_ToolBar(aCentralWidget);
  connect (myToolBar, SIGNAL (actionClicked (int)), this, SLOT (onToolBarActionClicked (int)));
  aParentLay->addWidget (myToolBar->GetControl(), 0, 0);

  // tree view
  myTreeView = new QTreeView (aCentralWidget);
  myTreeView->setSelectionBehavior (QAbstractItemView::SelectRows);
  myTreeView->setSelectionMode (QAbstractItemView::ExtendedSelection);
  myTreeView->header()->setStretchLastSection (true);
  myTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
  VInspector_ViewModel* aTreeModel = new VInspector_ViewModel (myTreeView);
  myTreeView->setModel (aTreeModel);
  connect (myTreeView, SIGNAL(customContextMenuRequested(const QPoint&)),
           this, SLOT (onTreeViewContextMenuRequested(const QPoint&)));
  new TreeModel_ContextMenu (myTreeView);

  QItemSelectionModel* aSelModel = new QItemSelectionModel (myTreeView->model(), myTreeView);
  myTreeView->setSelectionModel (aSelModel);
  connect (aSelModel, SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)),
           this, SLOT (onSelectionChanged (const QItemSelection&, const QItemSelection&)));

  aParentLay->addWidget(myTreeView, 1, 0);
  aParentLay->setRowStretch (1, 1);
  myMainWindow->setCentralWidget (aCentralWidget);

  myHistoryView = new Vinspector_TreeView (myMainWindow);
  ((Vinspector_TreeView*)myHistoryView)->SetPredefinedSize (VINSPECTOR_DEFAULT_HISTORY_VIEW_WIDTH,
                                                            VINSPECTOR_DEFAULT_HISTORY_VIEW_HEIGHT);
  myHistoryView->setContextMenuPolicy (Qt::CustomContextMenu);
  myHistoryView->header()->setStretchLastSection (true);
  connect (myHistoryView, SIGNAL (customContextMenuRequested (const QPoint&)),
          this, SLOT (onHistoryViewContextMenuRequested (const QPoint&)));
  new TreeModel_ContextMenu (myHistoryView);
  MessageModel_TreeModel* aModel = new MessageModel_TreeModel (myHistoryView);

  TreeModel_HeaderSection anItem = aModel->GetHeaderItem (0);
  anItem.SetName ("History");
  aModel->SetHeaderItem (0, anItem);
  for (int aColumnId = 3; aColumnId <= 5; aColumnId++) // hide time columns
  {
    TreeModel_HeaderSection anItem = aModel->GetHeaderItem (aColumnId);
    anItem.SetIsHidden (true);
    aModel->SetHeaderItem (aColumnId, anItem);
  }
  aModel->SetReversed (Standard_True);
  aModel->SetUniteAlerts (Standard_True);
  aModel->SetRootItemName ("AIS_InteractiveContext : Report");
  myHistoryView->setModel (aModel);
  TreeModel_Tools::UseVisibilityColumn (myHistoryView, false);
  // hide Visibility column
  anItem = aModel->GetHeaderItem ((int)TreeModel_ColumnType_Visibility);
  anItem.SetIsHidden (true);
  aModel->SetHeaderItem ((int)TreeModel_ColumnType_Visibility, anItem);

  QItemSelectionModel* aSelectionModel = new QItemSelectionModel (aModel);
  myHistoryView->setSelectionMode (QAbstractItemView::ExtendedSelection);
  myHistoryView->setSelectionModel (aSelectionModel);
  connect (aSelectionModel, SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)),
    this, SLOT (onHistoryViewSelectionChanged (const QItemSelection&, const QItemSelection&)));

  myHistoryActions = new MessageModel_Actions (myHistoryView, aModel, aSelectionModel);

  QAction* anAction = myHistoryActions->GetAction (MessageModel_ActionType_Activate);
  disconnect (anAction, SIGNAL (triggered (bool)), myHistoryActions, SLOT (OnActivateReport()));
  connect (anAction, SIGNAL (triggered (bool)), this, SLOT (OnActivateReport()));

  anAction = myHistoryActions->GetAction (MessageModel_ActionType_Deactivate);
  disconnect (anAction, SIGNAL (triggered (bool)), myHistoryActions, SLOT (OnDeactivateReport()));
  connect (anAction, SIGNAL (triggered (bool)), this, SLOT (OnDeactivateReport()));

  QModelIndex aParentIndex = myHistoryView->model()->index (0, 0);
  myHistoryView->setExpanded (aParentIndex, true);

  QDockWidget* aHistoryDockWidget = new QDockWidget (tr ("HistoryView"), myMainWindow);
  aHistoryDockWidget->setObjectName (aHistoryDockWidget->windowTitle());
  aHistoryDockWidget->setTitleBarWidget (new QWidget(myMainWindow));
  aHistoryDockWidget->setWidget (myHistoryView);
  myMainWindow->addDockWidget (Qt::BottomDockWidgetArea, aHistoryDockWidget);

  myMainWindow->resize (450, 800);
  myMainWindow->move (60, 20);

  myMainWindow->resize (VINSPECTOR_DEFAULT_WIDTH, VINSPECTOR_DEFAULT_HEIGHT);
  myMainWindow->move (VINSPECTOR_DEFAULT_POSITION_X, VINSPECTOR_DEFAULT_POSITION_Y);
}

// =======================================================================
// function : SetParent
// purpose :
// =======================================================================
void VInspector_Window::SetParent (void* theParent)
{
  myParent = (QWidget*)theParent;
  if (!myParent)
    return;

  QLayout* aLayout = myParent->layout();
  if (aLayout)
    aLayout->addWidget (GetMainWindow());
}

// =======================================================================
// function : FillActionsMenu
// purpose :
// =======================================================================
void VInspector_Window::FillActionsMenu (void* theMenu)
{
  QMenu* aMenu = (QMenu*)theMenu;
  QList<QDockWidget*> aDockwidgets = myMainWindow->findChildren<QDockWidget*>();
  for (QList<QDockWidget*>::iterator it = aDockwidgets.begin(); it != aDockwidgets.end(); ++it)
  {
    QDockWidget* aDockWidget = *it;
    if (aDockWidget->parentWidget() == myMainWindow)
      aMenu->addAction (aDockWidget->toggleViewAction());
  }
}

// =======================================================================
// function : GetPreferences
// purpose :
// =======================================================================
void VInspector_Window::GetPreferences (TInspectorAPI_PreferencesDataMap& theItem)
{
  theItem.Clear();
  theItem.Bind ("geometry",  TreeModel_Tools::ToString (myMainWindow->saveState()).toStdString().c_str());

  QMap<QString, QString> anItems;
  TreeModel_Tools::SaveState (myTreeView, anItems);
  TCollection_AsciiString aValue1, aValue2;
  for (QMap<QString, QString>::const_iterator anItemsIt = anItems.begin(); anItemsIt != anItems.end(); anItemsIt++)
  {
    aValue1 = anItemsIt.key().toStdString().c_str();
    aValue2 = anItemsIt.value().toStdString().c_str();
    theItem.Bind (anItemsIt.key().toStdString().c_str(), anItemsIt.value().toStdString().c_str());
  }

  anItems.clear();
  TreeModel_Tools::SaveState (myHistoryView, anItems, "history_view_");
  for (QMap<QString, QString>::const_iterator anItemsIt = anItems.begin(); anItemsIt != anItems.end(); anItemsIt++)
    theItem.Bind (anItemsIt.key().toStdString().c_str(), anItemsIt.value().toStdString().c_str());
}

// =======================================================================
// function : SetPreferences
// purpose :
// =======================================================================
void VInspector_Window::SetPreferences (const TInspectorAPI_PreferencesDataMap& theItem)
{
  if (theItem.IsEmpty())
  {
    TreeModel_Tools::SetDefaultHeaderSections (myTreeView);
    TreeModel_Tools::SetDefaultHeaderSections (myHistoryView);
    return;
  }

  for (TInspectorAPI_IteratorOfPreferencesDataMap anItemIt (theItem); anItemIt.More(); anItemIt.Next())
  {
    if (anItemIt.Key().IsEqual ("geometry"))
      myMainWindow->restoreState (TreeModel_Tools::ToByteArray (anItemIt.Value().ToCString()));
    else if (TreeModel_Tools::RestoreState (myTreeView, anItemIt.Key().ToCString(), anItemIt.Value().ToCString()))
      continue;
    else if (TreeModel_Tools::RestoreState (myHistoryView, anItemIt.Key().ToCString(), anItemIt.Value().ToCString(),
                                            "history_view_"))
      continue;
  }
}

// =======================================================================
// function : UpdateContent
// purpose :
// =======================================================================
void VInspector_Window::UpdateContent()
{
  TCollection_AsciiString aName = "TKVInspector";

  if (myParameters->FindParameters (aName))
    Init(myParameters->Parameters (aName));
  if (myParameters->FindFileNames(aName))
  {
    for (NCollection_List<TCollection_AsciiString>::Iterator aFileNamesIt(myParameters->FileNames(aName));
         aFileNamesIt.More(); aFileNamesIt.Next())
      OpenFile (aFileNamesIt.Value());

    NCollection_List<TCollection_AsciiString> aNames;
    myParameters->SetFileNames(aName, aNames);
  }

  // make AIS_InteractiveObject selected selected if exist in select parameters
  NCollection_List<Handle(Standard_Transient)> anObjects;
  VInspector_ViewModel* aViewModel = dynamic_cast<VInspector_ViewModel*>(myTreeView->model());
  if (aViewModel && myParameters->GetSelectedObjects(aName, anObjects))
  {
    QItemSelectionModel* aSelectionModel = myTreeView->selectionModel();
    aSelectionModel->clear();
    for (NCollection_List<Handle(Standard_Transient)>::Iterator aParamsIt (anObjects);
         aParamsIt.More(); aParamsIt.Next())
    {
      Handle(Standard_Transient) anObject = aParamsIt.Value();
      Handle(AIS_InteractiveObject) aPresentation = Handle(AIS_InteractiveObject)::DownCast (anObject);
      if (aPresentation.IsNull())
        continue;

      QModelIndex aPresentationIndex = aViewModel->FindIndex (aPresentation);
      if (!aPresentationIndex.isValid())
        continue;
       aSelectionModel->select (aPresentationIndex, QItemSelectionModel::Select);
       myTreeView->scrollTo (aPresentationIndex);
    }
  }
}

// =======================================================================
// function : Init
// purpose :
// =======================================================================
void VInspector_Window::Init (const NCollection_List<Handle(Standard_Transient)>& theParameters)
{
  Handle(AIS_InteractiveContext) aContext;
  Handle(VInspector_CallBack) aCallBack;

  for (NCollection_List<Handle(Standard_Transient)>::Iterator aParamsIt (theParameters); aParamsIt.More(); aParamsIt.Next())
  {
    Handle(Standard_Transient) anObject = aParamsIt.Value();
    if (aContext.IsNull())
      aContext = Handle(AIS_InteractiveContext)::DownCast (anObject);

    if (aCallBack.IsNull())
      aCallBack = Handle(VInspector_CallBack)::DownCast (anObject);
  }
  if (aContext.IsNull())
    return;
  VInspector_ViewModel* aViewModel = dynamic_cast<VInspector_ViewModel*> (myTreeView->model());
  if (aViewModel && aViewModel->GetContext() == aContext)
    UpdateTreeModel();
  else {
    // dynamic update of the view
    Handle(Message_Report) aMessageReport = aContext->GetReport();
    if (!aMessageReport.IsNull())
      aMessageReport->SetCallBack (myParameters->GetCallBack());
    SetContext (aContext);
  }

  if (!aCallBack.IsNull() && aCallBack != myCallBack)
  {
    myCallBack = aCallBack;
  }
}

// =======================================================================
// function : SetContext
// purpose :
// =======================================================================
void VInspector_Window::SetContext (const Handle(AIS_InteractiveContext)& theContext)
{
  VInspector_ViewModel* aViewModel = dynamic_cast<VInspector_ViewModel*> (myTreeView->model());
  aViewModel->SetContext (theContext);
  myTreeView->setExpanded (aViewModel->index (0, 0), true);

  if (!myCallBack.IsNull())
    myCallBack->SetContext (theContext);

  MessageModel_TreeModel* aHistoryModel = dynamic_cast<MessageModel_TreeModel*> (myHistoryView->model());
  aHistoryModel->AddReport (theContext->GetReport());
}

// =======================================================================
// function : OpenFile
// purpose :
// =======================================================================
void VInspector_Window::OpenFile(const TCollection_AsciiString& theFileName)
{
  VInspector_ViewModel* aViewModel = dynamic_cast<VInspector_ViewModel*> (myTreeView->model());
  if (!aViewModel)
  {
    Handle(AIS_InteractiveContext) aContext = createView();
    SetContext (aContext);
    aViewModel = dynamic_cast<VInspector_ViewModel*> (myTreeView->model());
  }
  if (!aViewModel)
    return;

  Handle(AIS_InteractiveContext) aContext = aViewModel->GetContext();
  if (aContext.IsNull())
    return;

  TopoDS_Shape aShape = VInspector_Tools::ReadShape (theFileName);
  if (aShape.IsNull())
    return;

  Handle(AIS_Shape) aPresentation = new AIS_Shape (aShape);
  aContext->Display (aPresentation, false);
  aContext->Load (aPresentation, -1/*selection mode*/, true);
  aContext->UpdateCurrentViewer();

  UpdateTreeModel();
  myTreeView->setExpanded (aViewModel->index (0, 0), true);
}

// =======================================================================
// function : onTreeViewContextMenuRequested
// purpose :
// =======================================================================
void VInspector_Window::onTreeViewContextMenuRequested(const QPoint& thePosition)
{
  QMenu* aMenu = new QMenu (GetMainWindow());
  aMenu->addAction (TreeModel_Tools::CreateAction (tr ("Export to ShapeView"), SLOT (onExportToShapeView()), GetMainWindow(), this));
  aMenu->addAction (TreeModel_Tools::CreateAction (tr ("Show"), SLOT (onShow()), GetMainWindow(), this));
  aMenu->addAction (TreeModel_Tools::CreateAction (tr ("Hide"), SLOT (onHide()), GetMainWindow(), this));
  QPoint aPoint = myTreeView->mapToGlobal (thePosition);
  aMenu->exec(aPoint);
}

// =======================================================================
// function : onToolBarActionClicked
// purpose :
// =======================================================================
void VInspector_Window::onToolBarActionClicked (const int theActionId)
{
  VInspector_ViewModel* aViewModel = dynamic_cast<VInspector_ViewModel*> (myTreeView->model());
  if (!aViewModel)
    return;

  switch (theActionId)
  {
    case VInspector_ToolActionType_UpdateId:
    {
      UpdateTreeModel();
      break;
    }
    case VInspector_ToolActionType_SelectPresentationsId:
    {
      bool isChecked = myToolBar->GetToolButton((VInspector_ToolActionType)theActionId)->isChecked();
      NCollection_List<Handle(AIS_InteractiveObject)> aPresentationsForViewer;
      if (isChecked)
        aPresentationsForViewer = VInspector_ItemPresentableObject::GetSelectedPresentations(myTreeView->selectionModel());
      Handle(AIS_InteractiveContext) aContext = aViewModel->GetContext();
      VInspector_Tools::AddOrRemovePresentations(aContext, aPresentationsForViewer);
      UpdateTreeModel();
      break;
    }
    case VInspector_ToolActionType_SelectOwnersId:
    {
      NCollection_List<Handle(SelectBasics_EntityOwner)> anOwnersForViewer;
      if (myToolBar->GetToolButton((VInspector_ToolActionType)theActionId)->isChecked())
        VInspector_ViewModel::GetSelectedOwners(myTreeView->selectionModel(), anOwnersForViewer);
      VInspector_Tools::AddOrRemoveSelectedShapes(aViewModel->GetContext(), anOwnersForViewer);
      UpdateTreeModel();
      break;
    }
    default:
      break;
  }
}

// =======================================================================
// function : onSelectionChanged
// purpose :
// =======================================================================
void VInspector_Window::onSelectionChanged (const QItemSelection&,
                                            const QItemSelection&)
{
  QApplication::setOverrideCursor (Qt::WaitCursor);

  if (myToolBar->GetToolButton(VInspector_ToolActionType_SelectPresentationsId)->isChecked())
    onToolBarActionClicked(VInspector_ToolActionType_SelectPresentationsId);
  else if (myToolBar->GetToolButton(VInspector_ToolActionType_SelectOwnersId)->isChecked())
    onToolBarActionClicked(VInspector_ToolActionType_SelectOwnersId);

  QApplication::restoreOverrideCursor();
}

// =======================================================================
// function : onHistoryViewContextMenuRequested
// purpose :
// =======================================================================
void VInspector_Window::onHistoryViewContextMenuRequested (const QPoint& thePosition)
{
  QMenu* aMenu = new QMenu (GetMainWindow());

  QModelIndexList aSelectedIndices = myHistoryView->selectionModel()->selectedIndexes();
  myHistoryActions->AddMenuActions (aSelectedIndices, aMenu);

  QPoint aPoint = myHistoryView->mapToGlobal (thePosition);
  aMenu->exec (aPoint);
}

// =======================================================================
// function : onHistoryViewSelectionChanged
// purpose :
// =======================================================================
void VInspector_Window::onHistoryViewSelectionChanged (const QItemSelection& theSelected,
                                                       const QItemSelection&)
{
  //VInspector_ViewModelHistory* aHistoryModel = dynamic_cast<VInspector_ViewModelHistory*> (myHistoryView->model());
  //if (!aHistoryModel)
  //  return;

  //if (theSelected.size() == 0)
  //  return;

  //QModelIndexList aSelectedIndices = theSelected.indexes();
  //QStringList aPointers = aHistoryModel->GetSelectedPointers(aSelectedIndices.first());

  //VInspector_ViewModel* aTreeModel = dynamic_cast<VInspector_ViewModel*> (myTreeView->model());
  //if (!aTreeModel)
  //  return;

  //QModelIndexList anIndices = aTreeModel->FindPointers (aPointers);
  //QItemSelectionModel* aSelectionModel = myTreeView->selectionModel();
  //aSelectionModel->clear();
  //for (int anIndicesId = 0, aSize = anIndices.size(); anIndicesId < aSize; anIndicesId++)
  //{
  //  QModelIndex anIndex = anIndices[anIndicesId];
  //  myTreeView->setExpanded (aTreeModel->parent (anIndex), true);
  //  aSelectionModel->select (anIndex, QItemSelectionModel::Select);
  //}
}

// =======================================================================
// function : onExportToShapeView
// purpose :
// =======================================================================
void VInspector_Window::onExportToShapeView()
{
  NCollection_List<Handle(AIS_InteractiveObject)> aSelectedPresentations =
            VInspector_ItemPresentableObject::GetSelectedPresentations(myTreeView->selectionModel());
  if (aSelectedPresentations.Extent() <= 0)
    return;

  TCollection_AsciiString aPluginName ("TKShapeView");
  NCollection_List<Handle(Standard_Transient)> aParameters;
  if (myParameters->FindParameters (aPluginName))
    aParameters = myParameters->Parameters (aPluginName);

  NCollection_List<TCollection_AsciiString> anItemNames;
  if (myParameters->FindSelectedNames (aPluginName))
    anItemNames = myParameters->GetSelectedNames (aPluginName);

  QStringList anExportedPointers;
  for (NCollection_List<Handle(AIS_InteractiveObject)>::Iterator anIOIt (aSelectedPresentations); anIOIt.More(); anIOIt.Next())
  {
    Handle(AIS_Shape) aShapePresentation = Handle(AIS_Shape)::DownCast (anIOIt.Value());
    if (aShapePresentation.IsNull())
      continue;

    const TopoDS_Shape& aShape = aShapePresentation->Shape();
    if (aShape.IsNull())
      continue;
    aParameters.Append (aShape.TShape());
    anItemNames.Append (TInspectorAPI_PluginParameters::ParametersToString(aShape));
    anExportedPointers.append (VInspector_Tools::GetPointerInfo (aShape.TShape(), true).ToCString());
  }
  if (anExportedPointers.empty())
    return;

  TCollection_AsciiString aPluginShortName = aPluginName.SubString (3, aPluginName.Length());
  QString aMessage = QString ("TShape %1 is sent to %2.")
    .arg (anExportedPointers.join(", "))
    .arg (aPluginShortName.ToCString());
  QString aQuestion = QString ("Would you like to activate %1 immediately?\n")
    .arg (aPluginShortName.ToCString()).toStdString().c_str();
  if (!myExportToShapeViewDialog)
    myExportToShapeViewDialog = new TreeModel_MessageDialog (myParent, aMessage, aQuestion);
  else
    myExportToShapeViewDialog->SetInformation (aMessage);
  myExportToShapeViewDialog->Start();

  myParameters->SetSelectedNames (aPluginName, anItemNames);
  myParameters->SetParameters (aPluginName, aParameters, myExportToShapeViewDialog->IsAccepted());
}

// =======================================================================
// function : onShow
// purpose :
// =======================================================================
void VInspector_Window::onShow()
{
  displaySelectedPresentations (true);
}

// =======================================================================
// function : onHide
// purpose :
// =======================================================================
void VInspector_Window::onHide()
{
  displaySelectedPresentations (false);
}

// =======================================================================
// function : OnDeactivateReport
// purpose :
// =======================================================================
void VInspector_Window::OnDeactivateReport()
{
  VInspector_ViewModel* aViewModel = dynamic_cast<VInspector_ViewModel*> (myTreeView->model());
  if (!aViewModel)
    return;

  Handle(AIS_InteractiveContext) aContext = aViewModel->GetContext();
  if (aContext.IsNull())
    return;

  aContext->SetReportActive (Standard_False);

  QModelIndex aReportIndex = findHistoryReportItem();
  if (aReportIndex.isValid())
    ((MessageModel_TreeModel*)aViewModel)->EmitDataChanged (aReportIndex, aReportIndex);
}

// =======================================================================
// function : OnActivateReport
// purpose :
// =======================================================================
void VInspector_Window::OnActivateReport()
{
  VInspector_ViewModel* aViewModel = dynamic_cast<VInspector_ViewModel*> (myTreeView->model());
  if (!aViewModel)
    return;

  Handle(AIS_InteractiveContext) aContext = aViewModel->GetContext();
  if (aContext.IsNull())
    return;

  aContext->SetReportActive (Standard_True);

  QModelIndex aReportIndex = findHistoryReportItem();
  if (aReportIndex.isValid())
    ((MessageModel_TreeModel*)aViewModel)->EmitDataChanged (aReportIndex, aReportIndex);
}

// =======================================================================
// function : UpdateTreeModel
// purpose :
// =======================================================================
void VInspector_Window::UpdateTreeModel()
{
  VInspector_ViewModel* aViewModel = dynamic_cast<VInspector_ViewModel*> (myTreeView->model());
  if (aViewModel)
    aViewModel->UpdateTreeModel();

  MessageModel_TreeModel* aHistoryModel = dynamic_cast<MessageModel_TreeModel*> (myHistoryView->model());
  if (aHistoryModel)
    aHistoryModel->UpdateTreeModel();
}

// =======================================================================
// function : displaySelectedPresentations
// purpose :
// =======================================================================
void VInspector_Window::displaySelectedPresentations(const bool theToDisplay)
{
  VInspector_ViewModel* aViewModel = dynamic_cast<VInspector_ViewModel*> (myTreeView->model());
  if (!aViewModel)
    return;

  Handle(AIS_InteractiveContext) aContext = aViewModel->GetContext();
  if (aContext.IsNull())
    return;

  NCollection_List<Handle(AIS_InteractiveObject)> aSelectedPresentations =
    VInspector_ItemPresentableObject::GetSelectedPresentations(myTreeView->selectionModel());
  if (aSelectedPresentations.Extent() <= 0)
    return;

  for (NCollection_List<Handle(AIS_InteractiveObject)>::Iterator anIOIt(aSelectedPresentations); anIOIt.More(); anIOIt.Next())
  {
    Handle(AIS_InteractiveObject) aPresentation = Handle(AIS_Shape)::DownCast(anIOIt.Value());
    if (aPresentation.IsNull())
      continue;
    if (theToDisplay) {
      aContext->Display(aPresentation, false);
      aContext->Load(aPresentation, -1, true);
    }
    else
      aContext->Erase(aPresentation, false);
  }
  aContext->UpdateCurrentViewer();

  // the order of objects returned by AIS_InteractiveContext is changed because the processed object is moved from
  // Erased to Displayed container or back
  QItemSelectionModel* aSelectionModel = myTreeView->selectionModel();
  aSelectionModel->clear();

  UpdateTreeModel();
}

// =======================================================================
// function : createView
// purpose :
// =======================================================================
Handle(AIS_InteractiveContext) VInspector_Window::createView()
{
  myViewWindow = new View_Window (0);
  myViewWindow->GetView()->SetPredefinedSize (VINSPECTOR_DEFAULT_VIEW_WIDTH, VINSPECTOR_DEFAULT_VIEW_HEIGHT);
  myViewWindow->move (VINSPECTOR_DEFAULT_VIEW_POSITION_X, VINSPECTOR_DEFAULT_VIEW_POSITION_Y);
  myViewWindow->show();

  return myViewWindow->GetView()->GetViewer()->GetContext();
}

// =======================================================================
// function : findHistoryReportItem
// purpose :
// =======================================================================
QModelIndex VInspector_Window::findHistoryReportItem()
{
  MessageModel_TreeModel* aModel = dynamic_cast<MessageModel_TreeModel*> (myHistoryView->model());
  if (!aModel)
    return QModelIndex();

  QModelIndex aParentIndex = aModel->index (0, 0); // root item
  QModelIndex aReportIndex = aModel->index (0, 0, aParentIndex); // report item

  TreeModel_ItemBasePtr anItemBase = TreeModel_ModelBase::GetItemByIndex (aReportIndex);
  MessageModel_ItemReportPtr aReportItem = itemDynamicCast<MessageModel_ItemReport> (anItemBase);

  if (!aReportItem)
    return QModelIndex();

  return aReportIndex;
}
