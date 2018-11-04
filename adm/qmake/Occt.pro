# This is a Solution project for building entire OCCT framework
TEMPLATE = subdirs

# Iterate over Modules and generate sub-projects
aSolModuleList = $$cat(../MODULES, lines)
for (aSolModuleIter, aSolModuleList) {
  aSolModule = $$first($$list($$aSolModuleIter))
  eval(occtkgen_$${aSolModule}.input  = $$_PRO_FILE_PWD_/OccModule.pro.in)
  eval(occtkgen_$${aSolModule}.output = $$_PRO_FILE_PWD_/$${aSolModule}/$${aSolModule}.pro)
  eval(occtkgen_$${aSolModule}.config = verbatim)
  eval(QMAKE_SUBSTITUTES += occtkgen_$${aSolModule})
  SUBDIRS += $${aSolModule}
}

# These dependencies are manually defined
ModelingData.depends = FoundationClasses
ModelingAlgorithms.depends = FoundationClasses ModelingData
Visualization.depends = FoundationClasses ModelingData ModelingAlgorithms
ApplicationFramework.depends = FoundationClasses ModelingData ModelingAlgorithms Visualization
DataExchange.depends = FoundationClasses ModelingData ModelingAlgorithms ApplicationFramework
Draw.depends = FoundationClasses ModelingData ModelingAlgorithms ApplicationFramework DataExchange Visualization
