// Copyright (c) 2013 OPEN CASCADE SAS
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

#ifndef _BRepMesh_IncrementalMesh_HeaderFile
#define _BRepMesh_IncrementalMesh_HeaderFile

#include <BRepMesh_DiscretRoot.hxx>
#include <IMeshTools_Parameters.hxx>

#include <BRepMesh_FastDiscret.hxx>

//! Builds the mesh of a shape with respect of their 
//! correctly triangulated parts 
class BRepMesh_IncrementalMesh : public BRepMesh_DiscretRoot
{
public: //! @name mesher API

  //! Default constructor
  Standard_EXPORT BRepMesh_IncrementalMesh();

  //! Destructor
  Standard_EXPORT virtual ~BRepMesh_IncrementalMesh();

  //! Constructor.
  //! Automatically calls method Perform.
  //! @param theShape shape to be meshed.
  //! @param theLinDeflection linear deflection.
  //! @param isRelative if TRUE deflection used for discretization of 
  //! each edge will be <theLinDeflection> * <size of edge>. Deflection 
  //! used for the faces will be the maximum deflection of their edges.
  //! @param theAngDeflection angular deflection.
  //! @param isInParallel if TRUE shape will be meshed in parallel.
  Standard_DEPRECATED("Deprecated constructor. Use constructor with IMeshTools_Parameters")
  Standard_EXPORT BRepMesh_IncrementalMesh(const TopoDS_Shape&    theShape,
                                           const Standard_Real    theLinDeflection,
                                           const Standard_Boolean isRelative = Standard_False,
                                           const Standard_Real    theAngDeflection = 0.5,
                                           const Standard_Boolean isInParallel = Standard_False);

  //! Constructor.
  //! Automatically calls method Perform.
  //! @param theShape shape to be meshed.
  //! @param theParameters - parameters of meshing
  Standard_EXPORT BRepMesh_IncrementalMesh(const TopoDS_Shape&          theShape,
                                           const IMeshTools_Parameters& theParameters);

  Standard_DEPRECATED("Deprecated constructor. Use constructor with IMeshTools_Parameters")
  Standard_EXPORT BRepMesh_IncrementalMesh(const TopoDS_Shape&                     theShape,
                                           const BRepMesh_FastDiscret::Parameters& theParameters);

//! Performs meshing ot the shape.
  Standard_EXPORT virtual void Perform() Standard_OVERRIDE;
  
public: //! @name accessing to parameters.

  //! Returns meshing parameters
  inline const IMeshTools_Parameters& GetParameters() const
  {
    return myParameters;
  }

  //! Returns modifiable meshing parameters
  inline IMeshTools_Parameters& GetParameters()
  {
    return myParameters;
  }
  
  //! Returns meshing parameters
  Standard_DEPRECATED("Deprecated method Parameters()")
  inline const BRepMesh_FastDiscret::Parameters& Parameters() const
  {
    const BRepMesh_FastDiscret::Parameters* aParams =
      reinterpret_cast<const BRepMesh_FastDiscret::Parameters*> (&myParameters);
    return *aParams;
  }

  //! Returns modifiable meshing parameters
  Standard_DEPRECATED("Deprecated method ChangeParameters()")
  inline BRepMesh_FastDiscret::Parameters& ChangeParameters()
  {
    BRepMesh_FastDiscret::Parameters* aParams =
      reinterpret_cast<BRepMesh_FastDiscret::Parameters*> (&myParameters);
    return *aParams;
  }

  //! Returns modified flag.
  inline Standard_Boolean IsModified() const
  {
    return myModified;
  }
  
  //! Returns accumulated status flags faced during meshing.
  inline Standard_Integer GetStatusFlags() const
  {
    return myStatus;
  }
  
private:

  //! Converts deprecated data to the new ones
  inline void initParameters(const BRepMesh_FastDiscret::Parameters& theParameters)
  {
    myParameters = reinterpret_cast<const IMeshTools_Parameters&>(theParameters);
    myParameters.AngleInterior      = -1.0;
    myParameters.DeflectionInterior = -1.0;
    initParameters();
  }

  //! Initializes specific parameters
  inline void initParameters()
  {
    if (myParameters.DeflectionInterior < Precision::Confusion())
    {
      myParameters.DeflectionInterior = myParameters.DeflectionBorder;
    }

    if (myParameters.MinSize < Precision::Confusion())
    {
      myParameters.MinSize =
        Max(IMeshTools_Parameters::RelMinSize() * Min(myParameters.DeflectionBorder,
                                                      myParameters.DeflectionInterior),
            Precision::Confusion());
    }

    if (myParameters.AngleInterior < Precision::Angular())
    {
      myParameters.AngleInterior = 2.0 * myParameters.AngleBorder;
    }
  }

public: //! @name plugin API

  //! Plugin interface for the Mesh Factories.
  //! Initializes meshing algorithm with the given parameters.
  //! @param theShape shape to be meshed.
  //! @param theLinDeflection linear deflection.
  //! @param theAngDeflection angular deflection.
  //! @param[out] theAlgo pointer to initialized algorithm.
  Standard_EXPORT static Standard_Integer Discret(const TopoDS_Shape&    theShape,
                                                  const Standard_Real    theLinDeflection,
                                                  const Standard_Real    theAngDeflection,
                                                  BRepMesh_DiscretRoot* &theAlgo);
  
  //! Returns multi-threading usage flag set by default in 
  //! Discret() static method (thus applied only to Mesh Factories).
  Standard_EXPORT static Standard_Boolean IsParallelDefault();
  
  //! Setup multi-threading usage flag set by default in 
  //! Discret() static method (thus applied only to Mesh Factories).
  Standard_EXPORT static void SetParallelDefault(const Standard_Boolean isInParallel);

  DEFINE_STANDARD_RTTI_INLINE(BRepMesh_IncrementalMesh, BRepMesh_DiscretRoot)

protected:

  IMeshTools_Parameters myParameters;
  Standard_Boolean      myModified;
  Standard_Integer      myStatus;
};

#endif
