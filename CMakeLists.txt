cmake_minimum_required(VERSION 3.0)

project( auto360cam )

find_package( OpenCV REQUIRED )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

set( VLFEAT_INCLUDE_DIRS "external/vlfeat" )

include_directories( ${OpenCV_INCLUDE_DIRS} ${VLFEAT_INCLUDE_DIRS} ) 

add_executable( auto360cam "src/main.cpp"
"src/argparse.cpp"
"src/autocrop.cpp"
"src/c3d.cpp"
"src/glimpses.cpp"
"src/logger.cpp"
"src/renderer.cpp"
"src/saliency.cpp"
"src/scorespace.cpp"
"src/trajectory.cpp"
"src/videoinfo.cpp"
"external/crop/AutocropFang.cpp"
"external/crop/AutocropStentiford.cpp"
"external/crop/AutocropSuh.cpp"
"external/crop/CompositionModel.cpp"
"external/saliency/SalMapItti.cpp"
"external/saliency/SalMapMargolin.cpp" 
"external/saliency/SalMapStentiford.cpp" )

add_library( vlfeats external/vlfeat/vl/host.c
external/vlfeat/vl/random.c
external/vlfeat/vl/generic.c
external/vlfeat/vl/slic.c )

target_link_libraries( auto360cam ${OpenCV_LIBS} vlfeats ) # pthread stdc++fs - for older gcc
