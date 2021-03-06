cmake_minimum_required(VERSION 2.6)
project(PipelineRTFS-NS-Diff-GIS)

find_package(OpenCV REQUIRED)

# Make sure the compiler can find the includes of the runtime system 
include_directories(${Runtime_SOURCE_DIR})
include_directories(${Runtime_SOURCE_DIR}/ResourceManager)
include_directories(${Runtime_SOURCE_DIR}/util)
include_directories(${RegionTemplate_SOURCE_DIR})
include_directories(${RegionTemplate_SOURCE_DIR}/Utils)
include_directories(${RegionTemplate_SOURCE_DIR}/comparativeanalysis)

# nscale dirs
find_path(NSCALE_SRC_DIR REQUIRED)
find_path(NSCALE_BUILD_DIR REQUIRED)
set(nscalesrcdir ${NSCALE_SRC_DIR})
set(nscalebuilddir ${NSCALE_BUILD_DIR})


#includes
include_directories(${nscalesrcdir}/segment)
include_directories(${nscalesrcdir}/segment/cuda)
include_directories(${nscalesrcdir}/common)
include_directories(${nscalesrcdir}/features)
include_directories(${nscalesrcdir}/normalization)

# Make sure the compiler can find the library in runtime system directory
link_directories(${Runtime_SOURCE_DIR})
# nscale lib dir
#link_directories(/Users/george/Documents/emory/nscale-sc13/nscale/lib)
link_directories(${nscalebuilddir}/lib)
# Create library with components implementation
set(libname componentnsdiffgis)

# Source files used by the Runtime System
set(libcomponentssrcs NormalizationComp.cpp Segmentation.cpp FeatureExtraction.cpp
        TaskNormalization.cpp TaskSegmentation.cpp TaskFeatures.cpp DiffMaskComp.cpp)

#Hadoopgis Dependencies
    set(libcomparativeanalysis ../../comparativeanalysis/TaskDiffMask.cpp
            ../../comparativeanalysis/pixelcompare/PixelCompare.cpp ../../comparativeanalysis/hadoopgis/Hadoopgis.cpp
            ../../comparativeanalysis/hadoopgis/predicates/DiceCoefficient.cpp
            ../../comparativeanalysis/hadoopgis/predicates/JaccardIndex.cpp
            ../../comparativeanalysis/hadoopgis/predicates/MaskIntersection.cpp
            ../../comparativeanalysis/hadoopgis/predicates/DiceNotCoolCoefficient.cpp)

    #Replace Macro inside program
    add_definitions(-DHADOOPGIS_SCRIPTS_DIRECTORY=${HADOOPGIS_SCRIPTS_OUTPUT_DIR})
    add_definitions(-DWRITE_ENABLED_TEMPORARY_PATH=${WRITE_ENABLED_TEMP_PATH})
    add_definitions(-DHADOOPGIS_BUILD_DIRECTORY=${HADOOPGIS_BUILD_DIR})


    #Copy the hadoopgis scripts to the executable build folder everytime the program is compiled.
    add_custom_target(copyScripts-Diff-GIS COMMAND ${CMAKE_COMMAND} -E copy_directory ${RegionTemplate_SOURCE_DIR}/comparativeanalysis/hadoopgis/scripts/ ${HADOOPGIS_SCRIPTS_OUTPUT_DIR})


#Copy the cache-conf.xml file located in the example folder to the build folder everytime the program is compiled.
add_custom_target(copyCacheConf-Diff-GIS COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/cache-conf/rtconf.xml ${CMAKE_CURRENT_BINARY_DIR})

# Add rule to generate the Runtime System, as a shared library (.so)
add_library("${libname}" SHARED ${libcomponentssrcs} ${libcomparativeanalysis})


add_dependencies("${libname}" copyScripts-Diff-GIS) #Hadoopgis dependency

#set_target_properties("${libname}" PROPERTIES LINK_FLAGS "${MPI_LINK_FLAGS}") 
target_link_libraries("${libname}" runtimesystem)
target_link_libraries(${libname} regiontemplates)

# nscale libs
target_link_libraries("${libname}" segment normalization featuresAll ${OpenCV_LIBS})

# Create executable to be built from the files listed
add_executable(PipelineRTFS-NS-Diff-GIS PipelineManager.cpp)
add_dependencies(PipelineRTFS-NS-Diff-GIS copyCacheConf-Diff-GIS)


# Link the executable to the runtime system library
target_link_libraries(PipelineRTFS-NS-Diff-GIS runtimesystem "${libname}")
target_link_libraries(PipelineRTFS-NS-Diff-GIS regiontemplates)
