# This is a project template file defining an OCCT Toolkit.
# This project should be included with predefined OCC_TOOLKIT_NAME variable.

TEMPLATE = lib
!exists(../../src/$$OCC_TOOLKIT_NAME/PACKAGES) {
  TEMPLATE = app
  CONFIG += console
}

include(OccCppConfig.pri)
exists(custom.pri) { include(custom.pri) }

aSrcRoot = $$OccGitRoot/src
aHxxRoot = $$OccGitRoot/inc
INCLUDEPATH += $$aHxxRoot

# CSF variables
CSF_FREETYPE  = freetype
CSF_TclLibs   = tcl8.6
CSF_TclTkLibs = tk8.6
HAVE_FREEIMAGE { CSF_FreeImagePlus = FreeImage } else:win32 { CSF_FreeImagePlus = windowscodecs Ole32 }
HAVE_FFMPEG    { CSF_FFmpeg = avcodec avformat swscale avutil }
HAVE_TBB       { CSF_TBB = tbb tbbmalloc }
HAVE_ZLIB      { CSF_ZLIB = zlib }
HAVE_LIBLZMA   { CSF_LIBLZMA = liblzma }
win32 {
  CSF_kernel32   = kernel32
  CSF_advapi32   = advapi32
  CSF_gdi32      = gdi32
  CSF_user32     = user32 comdlg32
  CSF_shell32    = Shell32
  CSF_opengl32   = opengl32
  CSF_wsock32    = wsock32
  CSF_netapi32   = netapi32
  CSF_OpenGlLibs = opengl32
  HAVE_GLES2 { CSF_OpenGlLibs = libEGL libGLESv2 }
  CSF_psapi      = Psapi
  CSF_d3d9       = d3d9
  CSF_TclLibs    = tcl86
  CSF_TclTkLibs  = tk86
  CSF_TBB =
} else {
  CSF_dl = dl
  CSF_ThreadLibs = pthread rt
  CSF_OpenGlLibs = GL
  CSF_TclTkLibs  = X11 tk8.6
  CSF_XwLibs     = X11 Xext Xmu Xi
  CSF_MotifLibs  = X11
  HAVE_GLES2 { CSF_OpenGlLibs = EGL GLESv2 }
}

# Define the list of standard OCCT file extensions
aHxxRegex = ^.*\.(hxx|h|lxx|gxx)$
aPxxRegex = ^.*\.(pxx)$
aCxxRegex = ^.*\.(cxx|c)$
mac { aCxxRegex = ^.*\.(cxx|c|m|mm)$ }

# Auxiliary function for probing file extension
defineTest (occCheckExtension) {
  aProbe = $$find(1, "$$2")
  count(aProbe, 1) { return(true) } else { return(false) }
}

# Auxiliary function for probing compilable files
defineTest (occIsCxxFile) { occCheckExtension ($$1, $$aCxxRegex) { return(true) } else { return(false) } }

# Auxiliary function for probing header files
defineTest (occIsHxxFile) { occCheckExtension ($$1, $$aHxxRegex) { return(true) } else { return(false) } }

aTkFiles     = $$cat($$aSrcRoot/$$OCC_TOOLKIT_NAME/FILES,     lines)
aTkFiles += CMakeLists.txt
aPackages    = $$cat($$aSrcRoot/$$OCC_TOOLKIT_NAME/PACKAGES,  lines)
anExternLibs = $$cat($$aSrcRoot/$$OCC_TOOLKIT_NAME/EXTERNLIB, lines)

for (aTkFileIter, aTkFiles) { OTHER_FILES += $$aSrcRoot/$$OCC_TOOLKIT_NAME/$$aTkFileIter }
for (anExternLib, anExternLibs) {
  hasCsf = $$find(anExternLib, CSF_)
  count(hasCsf, 1) {
    aList = $$split($$anExternLib, " ")
    for (aLibIter, aList) { LIBS += -l$$aLibIter }
  } else {
    LIBS += -l$$anExternLib
  }
}

# Iterate over Packages and add compilable files into this project
isEmpty (aPackages) { aPackages = $$OCC_TOOLKIT_NAME }
for (aPackage, aPackages) {
  #anOldHxxList  = $$files($$aHxxRoot/$${aPackage}*.hxx)
  #anOldHxxList += $$files($$aHxxRoot/$${aPackage}*.lxx)
  #anOldHxxList += $$files($$aHxxRoot/$${aPackage}*.gxx)
  #anOldHxxList += $$files($$aHxxRoot/$${aPackage}*.h)

  aPackageFolder = $$aSrcRoot/$$OCC_TOOLKIT_NAME/$$aPackage
  aPackageFiles = $$cat($$aSrcRoot/$$aPackage/FILES, lines)
  for (aFileIter, aPackageFiles) {
    occIsCxxFile($$aFileIter) {
      SOURCES += $$aSrcRoot/$$aPackage/$$aFileIter
    } else {
      contains(anOldHxxList, $$aHxxRoot/$$aFileIter) {
        anOldHxxList -= $$aHxxRoot/$$aFileIter
      } else {
        #OCCT_PACKAGE = $$aPackage
        #OCCT_HEADER  = $$aFileIter
        #occhxxgen.input  = $$_PRO_FILE_PWD_/../OcctHeaderLink.hxx.in
        #occhxxgen.output = $$aHxxRoot/$$aFileIter
        #QMAKE_SUBSTITUTES += occhxxgen
        #eval(occhxxgen_$${aFileIter}.input  = $$_PRO_FILE_PWD_/../OcctHeaderLink.hxx.in)
        #eval(occhxxgen_$${aFileIter}.output = $$aHxxRoot/$$aFileIter)
        #eval(QMAKE_SUBSTITUTES += occhxxgen_$${aFileIter})
      }
      occIsHxxFile($$aFileIter) {
        #HEADERS += $$aSrcRoot/$$aPackage/$$aFileIter
      } else {
        #OTHER_FILES += $$aSrcRoot/$$aPackage/$$aFileIter
      }
    }
  }

  !isEmpty(anOldHxxList) {
    #warning(The following header files will be removed on CLEAN: $$anOldHxxList)
    #QMAKE_CLEAN += $$anOldHxxList
  }
}
