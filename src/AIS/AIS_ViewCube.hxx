// Created on: 2017-07-25
// Created by: Anastasia BOBYLEVA
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

#ifndef _AIS_ViewCube_HeaderFile
#define _AIS_ViewCube_HeaderFile

#include <AIS_AnimationCamera.hxx>
#include <AIS_InteractiveObject.hxx>
#include <BRepPrim_Direction.hxx>
#include <gp.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <Graphic3d_ArrayOfTriangles.hxx>
#include <Graphic3d_Camera.hxx>
#include <Graphic3d_Group.hxx>
#include <Graphic3d_Vec2.hxx>
#include <NCollection_DataMap.hxx>
#include <Poly_Triangulation.hxx>
#include <Prs3d_ToolDisk.hxx>
#include <SelectMgr_EntityOwner.hxx>
#include <Standard_DefineHandle.hxx>
#include <TColStd_MapTransientHasher.hxx>
#include <V3d_View.hxx>

class AIS_ViewCubeFlat;

//! Interactive object for displaying the view manipulation cube.
//! It allows to manage view camera with more convenient way than manual manipulation of camera with mouse or touches.
//! View cube consists of several parts that are responsible for different camera manipulations:
//! @li Cube sides represent main views: top, bottom, left, right, front and back.
//! @li Edges represent rotation of one of main views on 45 degrees.
//! @li Vertices represent rotation of one of man views in two directions.
//! @li Arrows represent rotation of view around X, Y, and Z axes on 45 degrees.
//!
//! Sides, edges, vertices and axes form 3D part of object that rotates with view camera, but doesn't respond on panning and scaling.
//! This is made with trigedron transform persistence on 3D part of View Cube. Note that th eobject does not support
//! changing of transform persistance option outside the object, as well as local transformation and childs attaching and deletion.
//!
//! Arrows form 2D part of objects. This part is fully 2D and but doesn't respond on camera manipulations. This part has 2D transform persistance applied,
//! and for this purpose it is separated from 3D part into child interactive object.
//! Interation of View Cube is represented with StartTransform(), Transform() and HasTransformation() methods.
//!
//! AIS_ViewCube can be simply added to the target view with method AddTo(). It uses View Affinity option to display object in the application.
//! @code
//! Handle(AIS_ViewCube) aViewCube = new AIS_ViewCube();
//! aViewCube->AddTo (aContext, aView);
//! @endcode
//! or it can be just be displayed without iew affinity option (in this case it will be displayed in all views):
//! @code
//! Handle(AIS_ViewCube) aViewCube = new AIS_ViewCube();
//! Handle(AIS_InteractiveContext) aContext;
//! aContext->Display (aViewCube, Standard_False);
//! @endcode
//!
//! View Cube parts are sensitive to detection, or dynamic highlighting (but not selection), and every its owner corresponds to camera transformation.
//! So, once one of the owners of View Cube is detected, application is to be call StartTransform (anOwner) and Transform (anOwner) for starting
//! animation of transformation.
//! @code
//! aViewCube->StartTransform (aDetectedOwner);
//! while (aViewCube->HasTransformation())
//! {
//!   aViewCube->Transform();
//!   // Updating of application window
//!   ...
//! }
//! @endcode
//! or
//! @code aViewCube->Transform (aDetectedOwner); @endcode
//! that includes transformation loop.
//! This loop allows external actions like application updating. For this purpose AIS_ViewCube has virtual interface onAfterTransform(),
//! that is to be redefined on application level.
//! Note that after modification end no highlighting is restored by default.
//! This means that after transformation of box if mouse pointer still points to box, it will not be highlighted. 
//! Highlighting restoring of box is to be made on application level with AIS_InteracitveContext::MoveTo() method.
//!
//! @b Positioning:
//! View Cube is attached to one defined point in the view plane. This point of attachment is placed in the center of view
//! and can be changed with SetPosition() methods.
//!
//! @b Functionality:
//! View cube structure is customizable. It can be done with SetDrawVertices() and SetDrawEdges(). They make edges and vertices fully invisible
//! and insensitive.
//! 
//! @b Appearance management:
//! The class has API for:
//! @li Changing color of 3D part with SetBoxColor()
//! @li Changing color of 2D part with SetArrowColor()
//! @li Changing color of the whole object with SetColor()
//! @li Changing transparency with SetBoxTransparency(), SetArrowTransparency(), SetTransparency
//! @li Changing font for sides and axes text with SetText()
//! @li Changing arrow parameters with SetArrowAngle(), SetArrowLength()
//! By default arrows are semi-transparent.
class AIS_ViewCube : public AIS_InteractiveObject
{
public:

  DEFINE_STANDARD_RTTIEXT(AIS_ViewCube, AIS_InteractiveObject)

  Standard_EXPORT AIS_ViewCube();

  ~AIS_ViewCube() {}

  //! Add View Cube to defined context and displays it in the input view only.
  //! @param theContext [in] interactive context.
  //! @param theView [in] 3D view.
  Standard_EXPORT void AddTo (const Handle(AIS_InteractiveContext)& theContext, const Handle(V3d_View)& theView);

  //! Make view cube visible only in the input view
  //! @param theView [in] V3d View object
  //! @warning This method should be called after View Cube is displayed in context
  //!          or it will have no effect
  Standard_EXPORT void SetViewAffinity (const Handle(V3d_View)& theView);

  //! Hide View Cube in view
  Standard_EXPORT void Hide();

  //! Show View Cube in view
  Standard_EXPORT void Show();

  //! Set view for View Cube.
  //! It can be used as alternative of AddTo() method.
  Standard_EXPORT void SetView (const Handle(V3d_View)& theView);

  Standard_EXPORT const Handle(V3d_View)& View() const;

  //! Set default parameters for visual attributes
  //! @sa Attributes()
  Standard_EXPORT virtual void UnsetAttributes() Standard_OVERRIDE;

  //! Set default parameters for dynamic highlighting attributes, reset highlight attributes
  Standard_EXPORT virtual void UnsetHilightAttributes() Standard_OVERRIDE;

  //! Reset all size and style parameters to default.
  //! @warning It doesn't reset position of View Cube
  Standard_EXPORT void Reset();

protected:

  //! Internal constructor for child objects
  Standard_EXPORT AIS_ViewCube (const Handle(PrsMgr_PresentableObject)& theParent);

  //! Set default visual attributes
  Standard_EXPORT void setDefaultAttributes();

  //! Set default dynamic highlight properties
  Standard_EXPORT void setDefaultHighlightAttributes();

public: //! @name Geometry management API

  //! @return position of center of View Cube in terms of 2d trandform  persistence.
  //! @sa Aspect_TypeOfTriedronPosition, Graphic3d_Vec2i
  Standard_EXPORT void Position (Aspect_TypeOfTriedronPosition& thePosition,
                                 Graphic3d_Vec2i& theOffset);

  //! @return position of center of View Cube in screen cooordinates
  //! (origin of system if topleft corner). 
  Standard_EXPORT Graphic3d_Vec2i Position() const;

  //! Set position of center of View Cube in view plane depending on size of view.
  //! @warning this method works only if object is already attahed to the view.
  Standard_EXPORT Standard_Boolean SetPosition (const Graphic3d_Vec2i& thePosition);

  //! Set position of center of View Cube in view plane.
  //! Note that this method can be used before AddTo() method call (and therefore attaching of View Cube to the V3d_View),
  //! and therefore no Redisplay() will be called.
  //! Position is complex and consists of part of view where object will be attached
  //! and offset in pixels from the corner of view.
  //! @warning vertical/horizontal offsets are applied only if object is placed not in center. 
  //! @param thePosition [in] relative position of the view.
  //! @param theOffset [in] offset from the corner.
  //! Usage case:
  //! @code aViewCube->SetPosition (Aspect_TOTP_LEFT_LOWER, 200, 200); @endcode
  //! @sa Aspect_TypeOfTriedronPosition
  Standard_EXPORT void SetPosition (const Aspect_TypeOfTriedronPosition thePosition,
                                    const Standard_Integer theXOffset,
                                    const Standard_Integer theYOffset);

  //! Set position of center of View Cube in view plane depending on size of input view.
  //! Note that this method can be used before AddTo() method call (and therefore attaching of View Cube to the V3d_View),
  //! and therefore no Redisplay() will be called.
  //! @param thePosition [in] Position in view plane from top left corner.
  //! @param theView [in] input view.
  Standard_EXPORT Standard_Boolean SetPosition (const Graphic3d_Vec2i& thePosition,
                                                const Handle(V3d_View)& theView);

  //! @return size (width and height) of View cube sides.
  Standard_EXPORT Standard_Real Size() const;

  //! Sets size (width and height) of View cube sides.
  //! @param theToAdaptAnother [in] if is TRUE another parameters are adapted with the size.
  Standard_EXPORT void SetSize (const Standard_Real theValue,
                                const Standard_Boolean theToAdaptAnother = Standard_False);

  //! Set new value of padding between box sides.
  Standard_EXPORT void SetBoxPadding (const Standard_Real theValue);

  //! @return padding between box sides.
  Standard_EXPORT Standard_Real BoxPadding() const;

  //! Set new value of padding between axis and 3D part (box).
  Standard_EXPORT void SetAxisPadding (const Standard_Real theValue);

  //! @return padding between axes and 3D part (box).
  Standard_EXPORT Standard_Real AxisPadding() const;

  //! Set corner radius of View Cube sides.
  //! @param theValue [in] value in [0, 0.5] that show ration between box size and corner radius
  //! i.e. theValue = CornerRadius / Size();
  //! @warning throw Program Error if value is out of bounds.
  Standard_EXPORT void SetCornerRadius (const Standard_Real theValue);

  //! @return radius of side corners.
  Standard_EXPORT Standard_Real CornerRadius() const;

  //! @return TRUE if vertices (vertex) of View Cube is drawn.
  Standard_EXPORT Standard_Boolean ToDrawVertices() const;

  //! Enable/disable drawing of vertices (corners) of View Cube.
  Standard_EXPORT void SetDrawVertices (const Standard_Boolean theValue);

  //! @return TRUE if edges of View Cube is drawn.
  Standard_EXPORT Standard_Boolean ToDrawEdges() const;

  //! Enable/disable drawing of edges of View Cube.
  Standard_EXPORT void SetDrawEdges (const Standard_Boolean theValue);

  //! Set new value of arrow angle.
  //! @param theValue [in] the input angle in radians
  //! @code Attributes()->ArrowAspect()->SetAngle() @endcode
  //! can be used instead.
  Standard_EXPORT void SetArrowAngle (const Standard_Real theValue);

  //! @return angle on the pointer of arrow in radians.
  //! @code Attributes()->ArrowAspect()->Angle() @endcode
  //! can be used instead.
  Standard_EXPORT Standard_Real ArrowAngle() const;

  //! Set new value of arrow length.
  //! @param theValue [in] new value of arrow length.
  //! @code Attributes()->ArrowAspect()->SetLength() @endcode
  //! can be used instead.
  Standard_EXPORT void SetArrowLength (const Standard_Real theValue);

  //! @return length of arrows.
  //! @code Attributes()->ArrowAspect()->Length() @endcode
  //! can be used instead.
  Standard_EXPORT Standard_Real ArrowLength() const;

  //! Set new value of additional arrow padding between 3D box of View Cube and circle of 2D arrows.
  //! @param theValue [in] the input value of arrow padding.
  Standard_EXPORT void SetArrowPadding (const Standard_Real theValue);

  //! @return additional arrow padding between 3D box of View Cube and circle of 2D arrows.
  //! By default it is 0.
  Standard_EXPORT Standard_Real ArrowPadding() const;

  //! Set duration of animation.
  //! @param theValue [in] input value of duration in seconds.
  Standard_EXPORT void SetDuration (const Standard_Real theValue);

  //! @return duration of animation.
  Standard_EXPORT Standard_Real Duration() const;

public: //! @name Style management API

  //! Set color of text labels on box sides. By default it is black.
  //! @code Attributes()->TextAspect()->SetColor() @endcode can be used instead
  //! @param theColor [in] new color of text
  Standard_EXPORT void SetTextColor (const Quantity_Color& theColor);

  //! @return text color of labels of box sides. By default it is black.
  Standard_EXPORT const Quantity_Color& TextColor() const;

  //! Set font name that is used for displaying of sides and axes text.
  //! @param theFont [in] input name of font.
  //! @code Attributes()->TextAspect()->SetFont() @endcode
  //! can be used instead.
  Standard_EXPORT void SetFont (const TCollection_AsciiString& theFont);

  //! @return font name that is used for displaying of sides and axes text.
  //! @code Attributes()->TextAspect()->Aspect()->SetFont() @endcode
  //! can be used instead.
  Standard_EXPORT const TCollection_AsciiString& Font() const;

  //! Change font height
  //! @param theValue [in] new height of font.
  //! @code Attributes()->TextAspect()->SetHeight() @endcode
  //! can be used instead.
  Standard_EXPORT void SetFontHeight (const Standard_Real theValue);

  //! @return height of font
  Standard_EXPORT Standard_Real FontHeight() const;

  //! Set new value of color for the 2D part of object.
  //! @param theColor [in] input color value.
  Standard_EXPORT void SetArrowColor (const Quantity_Color& theColor);

  //! @return value of color for the 2D part of object.
  Standard_EXPORT const Quantity_Color& ArrowColor() const;

  //! Set new value of transparency for 2D part of object.
  //! @param theValue [in] input transparency value.
  Standard_EXPORT void SetArrowTransparency (const Standard_Real theValue);

  //! @return value of transparency for 2D part of object.
  Standard_EXPORT Standard_Real ArrowTransparency() const;

  //! Set color of sides back material.
  //! @code Attributes()->ShadingAspect()->Aspect()->ChangeBackMaterial().SetColor() @endcode
  //! can be used instead
  //! @param theColor [in] the color
  Standard_EXPORT void SetInnerColor (const Quantity_Color& theColor);

  //! @return color of sides back material.
  Standard_EXPORT const Quantity_Color& InnerColor() const;

  //! Set new value of front color for the 3D part of object.
  //! @param theColor [in] input color value.
  Standard_EXPORT void SetBoxColor (const Quantity_Color& theColor);

  //! @return value of front color for the 3D part of object.
  Standard_EXPORT const Quantity_Color& BoxColor() const;

  //! Set new value of transparency for 3D part of object.
  //! @param theValue [in] input transparency value.
  Standard_EXPORT void SetBoxTransparency (const Standard_Real theValue);

  //! @return transparency for 3D part of object.
  Standard_EXPORT Standard_Real BoxTransparency() const;

  //! Set new value of color for the whole object.
  //! @param theColor [in] input color value.
  Standard_EXPORT virtual void SetColor (const Quantity_Color& theColor) Standard_OVERRIDE;

  //! Set new value of transparency for the whole object.
  //! @param theValue [in] input transparency value.
  Standard_EXPORT virtual void SetTransparency (const Standard_Real theValue) Standard_OVERRIDE;

public: //! @name Transformation methods

  //! @return TRUE if View Cube has unfinished transformation of view camera.
  Standard_EXPORT Standard_Boolean HasTransformation() const;

  //! Start camera transformation corresponding to the input detected owner.
  //! @param theOwner [in] detected owner.
  Standard_EXPORT virtual void StartTransform (const Handle(SelectMgr_EntityOwner)& theOwner);

  //! Perform one step of current camera transformation.
  //! theToUpdate [in] enable/disable update of view.
  //! @return TRUE if animation is not stopped.
  Standard_EXPORT virtual Standard_Boolean Transform (const Standard_Boolean theToUpdate = Standard_False);

  //! Perform camera transformation loop corresponding to the input detected owner.
  //! @param theOwner [in] detected owner.
  Standard_EXPORT virtual void Transform (const Handle(SelectMgr_EntityOwner)& theOwner);

  //! Set transformation of View Cube and view Redraw() on HilightSelected() method (on mouse click).
  //! By defualt it is disabled.
  //! @param theValue [in] enable/disable flag
  Standard_EXPORT void SetAutoTransform (const Standard_Boolean theValue);

  //! @return TRUE if View Cube is automatically transformated and redraws view on HilightSelected() method.
  Standard_EXPORT Standard_Boolean IsAutoTransform() const;

protected:

  //! Perform internal single step of transformation.
  Standard_EXPORT Standard_Boolean transform();

protected: //! @name protected vitrual API

  //! Method that is called after one step of transformation.
  Standard_EXPORT virtual void onAfterTransform()
  {}

  //! Method that is called after transformation finish.
  Standard_EXPORT virtual void onFinishTransformation()
  {}

public: //! @name Presentation computation

  //! Compute 3D part of View Cube.
  //! @param thePrsMgr [in] presentation manager.
  //! @param thePrs [in] input presentation that is to be filled with flat presentation primitives.
  //! @param theMode [in] display mode.
  //! @warning this object accept only 0 display mode.
  Standard_EXPORT virtual void Compute (const Handle(PrsMgr_PresentationManager3d)& thePrsMgr,
                                        const Handle(Prs3d_Presentation)& thePrs,
                                        const Standard_Integer theMode = 0) Standard_OVERRIDE;

  //! Disables auto highlighting to use HilightSelected() and HilightOwnerWithColor() overriden methods.
  Standard_EXPORT virtual Standard_Boolean IsAutoHilight() const Standard_OVERRIDE
  {
    return Standard_False;
  }

  //! Method which clear all selected owners belonging to this selectable object.
  //! @warning this object does not support selection.
  virtual void ClearSelected() Standard_OVERRIDE
  {}

  //! Method which hilghights inputowner belonging to this selectable object.
  //! @param thePM [in] presentation manager
  //! @param theStyle [in] style for dymnamic highlighting.
  //! @param theOwner [in] input entity owner.
  Standard_EXPORT virtual void HilightOwnerWithColor (const Handle(PrsMgr_PresentationManager3d)& thePM,
                                                      const Handle(Prs3d_Drawer)& theStyle,
                                                      const Handle(SelectMgr_EntityOwner)& theOwner) Standard_OVERRIDE;

  //! Method which draws selected owners.
  Standard_EXPORT virtual void HilightSelected (const Handle(PrsMgr_PresentationManager3d)& thePM,
                                                const SelectMgr_SequenceOfOwner& theSeq) Standard_OVERRIDE;


  //! Redefine computing of sensitive entites for View Cube.
  //! @param theSelection [in] input selection object that is to be filled wtih sensitive entities.
  //! @param theMode [in] selection mode.
  //! @warning object accepts only 0 selection mode.
  Standard_EXPORT void ComputeSelection (const Handle(SelectMgr_Selection)& theSelection,
                                         const Standard_Integer theMode);

public: //! @name Managing of child object oresentation and selection.

  //! Compute presentation with flat parts of View Cube.
  //! @param thePrsMgr [in] presentation manager.
  //! @param thePrs [in] input presentation that is to be filled with flat presentation primitives.
  Standard_EXPORT void ComputeFlat (const Handle(PrsMgr_PresentationManager3d)& thePrsMgr, const Handle(Prs3d_Presentation)& thePrs);

  //! Compute sensitive entities for flat parts of View Cube.
  //! @param theSelection [in] input selection object that is to be filled wtih sensitive entities.
  Standard_EXPORT void ComputeSelectionFlat (const Handle(SelectMgr_Selection)& theSelection);

public:

  //! Redefines transform persistence management to setup transformation for sub-presentation of axes.
  //! @warning this interactive object does not support custom transformation persistence when.
  Standard_EXPORT virtual void SetTransformPersistence (const  Handle(Graphic3d_TransformPers)& theTrsfPers) Standard_OVERRIDE;

  //! Makes theObject child of current object in scene hierarchy.
  Standard_EXPORT virtual void AddChild (const Handle(PrsMgr_PresentableObject)& theObject) Standard_OVERRIDE;

  //! Removes theObject from children of current object in scene hierarchy.
  Standard_EXPORT virtual void RemoveChild (const Handle(PrsMgr_PresentableObject)& theObject) Standard_OVERRIDE;

protected:

  Standard_EXPORT void setTransformPersistence (const Handle(Graphic3d_TransformPers)& theTrsfPers);

  Standard_EXPORT void addChild (const Handle(PrsMgr_PresentableObject)& theObject);

  Standard_EXPORT void removeChild (const Handle(PrsMgr_PresentableObject)& theObject);

  //! Redefines local transformation management method to inform user of inproper use.
  //! @warning this interactive object does not support setting custom local transformation.
  Standard_EXPORT virtual void setLocalTransformation (const Handle(Geom_Transformation)& theTrsf) Standard_OVERRIDE;

  //! Hide methods
  using AIS_InteractiveObject::SetLocalTransformation;
  using SelectMgr_SelectableObject::SetZLayer;

protected: //! @name Auxilliary classes to fill presentation with proper primitives

  //! Base object that represent transformation of view camera.
  class CameraState : public Standard_Transient
  {
  public:
    DEFINE_STANDARD_RTTI_INLINE (CameraState, Standard_Transient)
    CameraState() {}
    virtual void FillCamera (const Handle(V3d_View)& theView, const Handle(Graphic3d_Camera)& theCamera) = 0;
  };

  //! Object that defines new orientation of camera.
  class CameraStateReplace : public CameraState
  {
  public:

    DEFINE_STANDARD_RTTI_INLINE (CameraStateReplace, CameraState)

    //! @param theDir [in] Direction from eye to center of camera.
    //! @param theUp [in] Up direction of camera.
    CameraStateReplace (const gp_Dir& theDir, const gp_Dir& theUp)
      : Up (theUp),
        Direction (theDir)
    {}

    //! Apply new state to the camera.
    //! @param theCamera [in] input camera object.
    virtual void FillCamera (const Handle(V3d_View)& theView, const Handle(Graphic3d_Camera)& theCamera) Standard_OVERRIDE;

  protected:

    gp_Dir Up; //!< Up direction of camera
    gp_Dir Direction; //!< Direction from eye to center of camera.
  };

  //! Object that defines rotation of camera on defined axis angles
  class CameraStateRotate : public CameraState
  {
  public:

    DEFINE_STANDARD_RTTI_INLINE (CameraStateRotate, CameraState)

    //! @param theAngleX [in] Angle for rotation of X Axis.
    //! @param theAngleY [in] Angle for rotation of Y Axis.
    //! @param theAngleZ [in] Angle for rotation of Z Axis.
    CameraStateRotate (const Standard_Real theAngleX, const Standard_Real theAngleY, const Standard_Real theAngleZ)
      : AngleX (theAngleX),
        AngleY (theAngleY),
        AngleZ (theAngleZ)
    {}

    //! Apply rotate object state to the camera.
    //! @param theCamera [in] input camera object.
    virtual void FillCamera (const Handle(V3d_View)& theView, const Handle(Graphic3d_Camera)& theCamera) Standard_OVERRIDE;

  protected:

    Standard_Real AngleX; //!< Angle for rotation of X Axis
    Standard_Real AngleY; //!< Angle for rotation of Y Axis
    Standard_Real AngleZ; //!< Angle for rotation of Z Axis
  };

  //! Triangular part of view cube
  class Part : public Standard_Transient
  {
  public:

    DEFINE_STANDARD_RTTI_INLINE (Part, Standard_Transient)

    Part() {}

    Part (const Handle(AIS_ViewCube)& theParent)
    : myParent (theParent)
    {}

    ~Part()
    {
      Reset();
    }

    //! Reset part fields.
    void Reset();

    void ComputeSelection (const Handle(SelectMgr_EntityOwner)& theOwner,
                           const Handle(SelectMgr_Selection)& theSelection);

    //! Set transform persistance for highlighting presentation.
    void SetTransformPersistence (const Handle(Graphic3d_TransformPers)& theTrsfPers);

    //! @return presentation for highlighting.
    const Handle(Prs3d_Presentation)& HighlightPresentation() const
    {
      return myHighlightPresentation;
    }

    //! @return triangulations of the part.
    const NCollection_Sequence<Handle(Poly_Triangulation)>& Triangulations() const
    {
      return myTriangulations;
    }

    //! Append triangulation to part.
    void AddTriangulation (const Handle(Poly_Triangulation)& theTri)
    {
      myTriangulations.Append (theTri);
    }

    //! @return direction corresponding to the input direction identifier.
    //! @param theDir [in] direction identifier.
    //! @sa BRepPrim_Direction
    gp_XYZ Direction (const BRepPrim_Direction theDir);

    //! @return absolute location of left bottom corner of parent cube.
    gp_XYZ Location();

  protected:

    //! Set of triangulations of sensitive entities.
    NCollection_Sequence<Handle(Poly_Triangulation)> myTriangulations;
    Handle(Prs3d_Presentation) myHighlightPresentation;
    Handle(AIS_ViewCube) myParent;
  };
  
  //! Tool to fill primitive array for rectangle with rounded corners.
  class ToolRectangle
  {
  public:


    //! @param thePosition [in] a plane for rectangle.
    //! @param theBottomLeft [in] bottom left corner.
    //! @param theTopLeft [in] top left corner.
    //! @param theBottomRight [in] bottom right corner.
    //! @param theTopRight [in] top right corner.
    //! @param theRadius [in] radius of rounded corners.
    ToolRectangle (const gp_Ax2& thePosition, const gp_Pnt& theBottomLeft, const gp_Pnt& theTopLeft,
                   const gp_Pnt& theBottomRight, const gp_Pnt& theTopRight, const Standard_Real theRadius)
    : myPosition (thePosition),
      myBottomLeft (theBottomLeft),
      myTopLeft (theTopLeft),
      myBottomRight (theBottomRight),
      myTopRight (theTopRight),
      myRadius (theRadius)
    {}

    //! Fill primitive array and triangulation for sentitve entity.
    void FillArray (Handle(Graphic3d_ArrayOfTriangles)& theArray,
                    Handle(Poly_Triangulation)& theTriangulation);

  protected:

    //! Compute number of vertices  for triangular array.
    Standard_Integer NumberOfTriangleVerices() const;

    //! Fill primitive array with triangles for rounded corner of rectnagle.
    //! @param theCircle [in] defines position and radius of corner.
    //! @param theStartParameter [in] start circular parameter of corner arc.
    //! @param theendParameter [in] end circular parameter of corner arc.
    //! @param theArray [in] primitive array.
    void FillCorner (const gp_Circ& theCircle,
                     const Standard_Real theStartParameter,
                     const Standard_Real theEndParameter,
                     const Handle(Graphic3d_ArrayOfTriangles)& theArray);

  protected:

    gp_Ax2 myPosition;      //!< Position of plane where rectangle lies.
    gp_Pnt myBottomLeft;    //!< Bottom left corner. 
    gp_Pnt myTopLeft;       //!< Top left corner.
    gp_Pnt myBottomRight;   //!< Bottom right corner.
    gp_Pnt myTopRight;      //!< Top right corner.
    Standard_Real myRadius; //!< Radius of corners.
  };

  //! Part to display side of cube with rounded corners.
  class Side : public Part
  {
  public:
    DEFINE_STANDARD_RTTI_INLINE (Side, Part)

    Side() : Part() {}

    //! @param theParent [in] AIS_ViewCube parent object.
    //! @param theDirection [in] Defines cube face.
    //! @param theText [in] Text drawn in the middle of face.
    Side (const Handle(AIS_ViewCube)& theParent,
          const BRepPrim_Direction theDirection,
          const TCollection_ExtendedString theText)
      : Part (theParent),
        myDirection (theDirection),
        myText (theText)
    {}

    ~Side() {}

    //! Fill graphic group with primitives, fills internal triangulation and highlight presentation.
    //! @param thePrsMgr [in] presentation manager.
    //! @param theGroup [in] the graphic group.
    //! @param theTextAspect [in] text style.
    void Display (const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                  const Handle(Graphic3d_Group)& theGroup,
                  const Handle(Prs3d_TextAspect)& theTextAspect);

  protected:

    BRepPrim_Direction myDirection; //!< Defines cube face.
    TCollection_ExtendedString myText; //!< Text drawn in the middle of face.
  };

  //! Part to display vertex of cube as circle with defined radius.
  class Vertex : public Part
  {
  public:
    DEFINE_STANDARD_RTTI_INLINE (Vertex, Part)

    Vertex() : Part() {}

    //! @param theParent [in] AIS_ViewCube parent object
    //! @param theDirection1 [in] Defines cube face tangent to vertex.
    //! @param theDirection2 [in] Defines cube face tangent to vertex.
    //! @param theDirection3 [in] Defines cube face tangent to vertex.
    Vertex (const Handle(AIS_ViewCube)& theParent,
            const BRepPrim_Direction theDirection1,
            const BRepPrim_Direction theDirection2,
            const BRepPrim_Direction theDirection3)
    : Part (theParent),
      myDirection1 (theDirection1),
      myDirection2 (theDirection2),
      myDirection3 (theDirection3)
    {}

    ~Vertex() {}

    //! Fill graphic group with primitives, fills internal triangulation and highlight presentation.
    //! @param thePrsMgr [in] presentation manager
    //! @param theGroup [in] the graphic group
    //! @param theRadius [in] radius of rounded vertex
    //! @param theSlicesNb [in] number of facettes for Prs3d_ToolDisk
    void Display (const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                  const Handle(Graphic3d_Group)& theGroup,
                  const Standard_Real theRadius,
                  const Standard_Integer theSlicesNb = 20);

  protected:

    BRepPrim_Direction myDirection1; //!< Defines cube face tangent to vertex.
    BRepPrim_Direction myDirection2; //!< Defines cube face tangent to vertex.
    BRepPrim_Direction myDirection3; //!< Defines cube face tangent to vertex.
  };

  //! Part that defines rectangular edge with rounded corners.
  class Edge : public Part
  {
  public:
    DEFINE_STANDARD_RTTI_INLINE (Edge, Part)

    Edge(): Part() {}

    //! Constructor
    //! @param theParent [in] AIS_ViewCube parent object
    //! @param theDirection1 [in] Defines cube face tangent to edge.
    //! @param theDirection2 [in] Defines cube face tangent to edge.
    Edge (const Handle(AIS_ViewCube)& theParent,
          const BRepPrim_Direction theDirection1,
          const BRepPrim_Direction theDirection2)
      : Part (theParent),
        myDirection1 (theDirection1),
        myDirection2 (theDirection2)
    {}

    //! Fill graphic group with primitives, fills internal triangulation and highlight presentation.
    //! @param thePrsMgr [in] presentation manager
    //! @param theGroup [in] the graphic group
    //! @param theThickness [in] thickness of edge
    void Display (const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                  const Handle(Graphic3d_Group)& theGroup,
                  const Standard_Real theThickness);

  protected:

    BRepPrim_Direction myDirection1; //!< Defines cube face tangent to edge.
    BRepPrim_Direction myDirection2; //!< Defines cube face tangent to edge.
  };

  //! Tool to fill primitive array for flat triangular arrow.
  //!      ^
  //!     /|   <-theDir
  //!    / |
  //!   /--*  <- thePosition
  class ToolArrow
  {
  public:

    //! @param theAxis [in] Axis from the base of arrow to the pointer.
    //! @param theLength [in] Length from the pointer ot base of arrow.
    //! @param theAngle [in] Value of pointer angle in radians.
    ToolArrow (const gp_Ax1& theAxis, const Standard_Real theLength, const Standard_Real theAngle)
      : myAxis (theAxis),
        myLength (theLength),
        myAngle (theAngle)
    {}

    //! Fill primitive array and triangulation for sentitve entity.
    void FillArray (Handle(Graphic3d_ArrayOfTriangles)& theArray,
                    Handle(Poly_Triangulation)& theTriangulation);

  protected:

    gp_Ax1 myAxis; //!< Axis from the base of arrow to the pointer.
    Standard_Real myLength; //!< Length from the pointer ot base of arrow.
    Standard_Real myAngle; //!< Value of pointer angle in radians.
  };

  //! Part to display triangular flat arrow.
  class FlatArrow : public Part
  {
  public:

    DEFINE_STANDARD_RTTI_INLINE (FlatArrow, Part)
  public:

    FlatArrow() : Part() {}

    FlatArrow (const Handle(AIS_ViewCube)& theParent)
    : Part (theParent)
    {}

   ~FlatArrow() {}

    //! @param thePrsMgr [in] presentation manager
    //! @param theGroup [in] the graphic group
    //! @param theAxis [in] bottom center of arrow, direction from bottom line to the pointer of arrow
    //! @param theLength [in] length of the arrow from the pointer to the base of arrow
    //! @param theAngle [in] value of pointer angle in radians
    void Display (const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                  const Handle(Graphic3d_Group)& theGroup,
                  const gp_Ax1& theAxis,
                  const Standard_Real theLength,
                  const Standard_Real theAngle);
  };

  //! Tool to fill triangle primitive array corresponding to disk segment.
  class ToolDiskSegment : public Prs3d_ToolDisk
  {
  public:

    //! @param theInnerRadius [in] small radius of disk segment.
    //! @param theOuterRadius [in] big radius of disk segment.
    //! @param theStartAngle [in] Start angle in counter clockwise order.
    //! @param theEndAngle [in] End angle in counter clockwise order.
    //! @param theNbSlides [in] Number of facettes.
    Standard_EXPORT ToolDiskSegment (const Standard_Real    theInnerRadius,
                                     const Standard_Real    theOuterRadius,
                                     const Standard_Real    theStartAngle,
                                     const Standard_Real    theEndAngle,
                                     const Standard_Integer theNbFacettes);

  protected:

    Standard_EXPORT virtual gp_Pnt Vertex (const Standard_Real theU, const Standard_Real theV) Standard_OVERRIDE;

    Standard_Real myStartAngle; //!< Start angle in counter clockwise order.
    Standard_Real myEndAngle; //!< End angle in counter clockwise order.
  };

  //! Displays rounded arrow consisted of disk segment and triangular arrow
  class RoundedArrow : public Part
  {
  public:

    DEFINE_STANDARD_RTTI_INLINE (RoundedArrow, Part)

    //! Empty constructor
    RoundedArrow() : Part() {}

    RoundedArrow (const Handle(AIS_ViewCube)& theParent)
    : Part (theParent)
    {}

    //! Destructor
    ~RoundedArrow() {}

    //! @param thePrsMgr [in] presentation manager
    //! @param theGroup [in] the graphic group
    //! @param theCenter [in] location of circle that lies on the center axis of rounded arrow 
    //! @param theArrowAngle [in] arrow angle
    //! @param theArrowLength [in] arrow length
    //! @param theInnerRadius [in] inner radius
    //! @param theStartAngle [in] start angle
    //! @param theEndAngle [in] end angle
    //! @param theIsClockwise [in] the direction of angle, be default
    //! angle is counted in couner clockwise order, from start to end angle
    //! @param theSlicesNb [in] number of slices for triangulation of arrow tail segment
    void Display (const Handle(PrsMgr_PresentationManager)& thePrsMgr,
                  const Handle(Graphic3d_Group)& theGroup,
                  const gp_Pnt& theCenter,
                  const Standard_Real theArrowLength,
                  const Standard_Real theArrowAngle,
                  const Standard_Real theInnerRadius,
                  const Standard_Real theStartAngle,
                  const Standard_Real theEndAngle,
                  const Standard_Boolean theIsClockwise = Standard_False,
                  const Standard_Integer theSlicesNb = 20);
  };

  //! The class displays axis presentation with text.
  class Axis
  {
  public:

    Axis (const gp_Ax1& theAxis = gp_Ax1(),
          const Quantity_Color& theColor = Quantity_Color(),
          const Standard_ExtCharacter& theSymbol = L' ',
          const Standard_Real theLength = 10.);

    //! Compute presentation of axis.
    //! @param theGroup [in] graphic group.
    //! @param theAspect [in] shading style.
    //! @param theTextAspect [in] text style.
    void Compute (const Handle(Graphic3d_Group)& theGroup,
                  const Handle(Prs3d_ShadingAspect)& theAspect,
                  const Handle(Prs3d_TextAspect)& theTextAspect);

    //! @return color of axis.
    Quantity_Color Color() const { return myColor; }

    //! Set position of axis.
    void SetPosition (const gp_Ax1& thePosition) { myPosition = thePosition; }

    //! @return position of axis.
    const gp_Ax1& Position() const { return myPosition; }

    //! @return lenght of axis.
    Standard_Real Length() const { return myLength; }

    //! Set length of axis including arrows.
    void SetLength (const Standard_Real theValue)
    {
      myLength = theValue;
    }

    //! @return text symbol of the axis.
    const Standard_ExtCharacter Symbol() const { mySymbol; }

    //! Set text symbol of axis: 'X', 'Y' or 'Z' are used.
    void SetSymbol (const Standard_ExtCharacter& theValue) { mySymbol = theValue; }

  protected:

    gp_Ax1 myPosition; //!< Position of the axis including local transformation.
    Quantity_Color myColor; //!< Color of axis.
    Standard_Real myLength; //!< Length of translation axis.
    Standard_ExtCharacter mySymbol; //!< Symbol attached to the axis.
  };

protected:

  Standard_EXPORT Standard_Integer addPart (const Handle(Part)& thePart,
                                            const gp_Dir& theDir, const gp_Dir& theUp,
                                            const Standard_Integer thePriority = 7);

  Standard_EXPORT Standard_Integer addPart (const Handle(Part)& thePart,
                                            const Standard_Real theAngleX, const Standard_Real theAngleY, const Standard_Real theAngleZ,
                                            const Standard_Integer thePriority = 7);

  Standard_EXPORT gp_XYZ direction (const Standard_Integer theDir);

  Standard_EXPORT Standard_Real sign (const Standard_Real theValue);

protected: //! @name Map definitions

  typedef NCollection_IndexedDataMap<Handle(SelectMgr_EntityOwner), Handle(CameraState),TColStd_MapTransientHasher>
  MapOfOwnerCameraState;

  typedef NCollection_IndexedDataMap<Handle(SelectMgr_EntityOwner), Handle(Part), TColStd_MapTransientHasher>
  MapOfOwnerPart;

private:

  
  Aspect_TypeOfTriedronPosition myPosition; //!< Presisted position of cube.
  Graphic3d_Vec2i myOffset; //!< Offset to the position. It is needed to set needed position.
  Standard_Real mySize; //!< Size of box side, length of one axis.
  Standard_Real myBoxPadding; //!< Padding between box elements.
  Standard_Real myAxisPadding; //!< Padding between box and axis.
  Standard_Real myCornerRadius; //!< Value of corner radius in value of ratio to Size, is in [0; 0.5].
  Standard_Real myArrowPadding; //!< Additional padding for arrows and box, by default it is equal to 0.
  Standard_Boolean myToDisplayEdges; //!< Flag that shows if edges of box are to be displayed.
  Standard_Boolean myToDisplayVertices; //!< Flag that shows if vertices (corners) of box are to be displayed.
  
  Handle(V3d_View) myView; //!< Reference to view.
  Handle(Prs3d_ShadingAspect) myArrowAspect; //!< Style for 2D arrows presentations.
  MapOfOwnerPart myParts; //!< Map of graphical parts: box sides, edges, vertices, arrows.

private: //! @name Animation options

  Handle(Graphic3d_Camera) myStartState; //!< Start state of view camera.
  Handle(Graphic3d_Camera) myEndState; //!< End state of view camera.
  Handle(AIS_AnimationCamera) myAnimation; //!< Camera animation object.
  Standard_Real myDuration; //!< Duration of animation. By default it is half a second.
  MapOfOwnerCameraState myStates; //! Map of linked owners and camera states: positions or angles to rotate.
  Standard_Boolean myIsAutoTransform; //!< Means that transformation is performed in HilightSelected() method.

private:

  Handle(AIS_ViewCubeFlat) myFlatPart; //!< Reference to the child interactive object for 2D part.
};

//! @brief Auxilliary class for defining flat part of View Cube.
//! It is created to display arrows with 2D transform persistance
//! and amange sensitive entites with the same persistance.
//! This object does not manage any View Cube options, camera manipulations.
class AIS_ViewCubeFlat : public AIS_ViewCube
{
public:

  DEFINE_STANDARD_RTTIEXT(AIS_ViewCubeFlat, AIS_ViewCube)

  //! Constructs dependent interactive object for View Cube flat part
  AIS_ViewCubeFlat (const Handle(AIS_ViewCube)& theParent);

  //! Destructor
  ~AIS_ViewCubeFlat() {}

protected:

  //! @return parent object: AIS_ViewCube instance
  Handle(AIS_ViewCube) parent() const;

public:

  //! Fill flat presentation.
  //! @param thePrsMgr [in] presentation manager
  //! @param thePrs [in] reference to the presentation that is to be filled with graphic groups. It is mutable here.
  //! @param theMode [in] display mode.
  //! @warning Only 0 display mode is acceptable.
  Standard_EXPORT virtual void Compute (const Handle(PrsMgr_PresentationManager3d)& thePrsMgr,
                                        const Handle(Prs3d_Presentation)& thePrs,
                                        const Standard_Integer theMode = 0) Standard_OVERRIDE;

  //! Fill sensitive primitives.
  //! @param theSelection [in] selection obejct to be filled
  //! @param theMode [in] input selection mode
  //! @warning Only zero seelction mode is acceptable.
  Standard_EXPORT void ComputeSelection (const Handle(SelectMgr_Selection)& theSelection,
                                         const Standard_Integer theMode) Standard_OVERRIDE;

  //! Manage custom dynamic highlighting. It is calls as IsAutoHilight() method is redefined for thids object
  //! and always return Standard_False.
  //! @param thePM [in] presentation manager
  //! @param theStyle [in] graphical attributes for dynamic highlighting. Here DynamicHilightAttributes() of parent
  //! interactive object is used.
  //! @param theOwner [in] owner of detected sensitive entity.
  Standard_EXPORT virtual void HilightOwnerWithColor (const Handle(PrsMgr_PresentationManager3d)& thePM,
                                                      const Handle(Prs3d_Drawer)& theStyle,
                                                      const Handle(SelectMgr_EntityOwner)& theOwner) Standard_OVERRIDE;

  //! Method which draws selected owners.
  Standard_EXPORT virtual void HilightSelected (const Handle(PrsMgr_PresentationManager3d)& thePM,
                                                const SelectMgr_SequenceOfOwner& theSeq) Standard_OVERRIDE;

};

//! Redefined entity owner that is highglighted when owner is detected,
//! even if Interactive Context highlighted on last detection procedure.
//! This owner is needed for View Cube to highlight arrows once more after tranformation.
class AIS_ViewCubeOwner : public SelectMgr_EntityOwner
{
public:

  DEFINE_STANDARD_RTTI_INLINE (AIS_ViewCubeOwner, SelectMgr_EntityOwner)

  AIS_ViewCubeOwner (const Handle(SelectMgr_SelectableObject)& theObject, const Standard_Integer thePriority = 0)
  : SelectMgr_EntityOwner (theObject, thePriority)
  {}

  //! @return TRUE. This owner will always call method
  //! Hilight for its Selectable Object when the owner is detected.
  Standard_EXPORT virtual Standard_Boolean IsForcedHilight() const Standard_OVERRIDE
  {
    return Standard_True;
  }
};


#endif
