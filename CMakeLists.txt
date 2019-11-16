cmake_minimum_required(VERSION 3.0)

project( auto360cam )

# find_package( Threads )
find_package( OpenCV REQUIRED )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17" )

set( VLFEAT_INCLUDE_DIRS "external/vlfeat" )

include_directories( ${OpenCV_INCLUDE_DIRS} ${VLFEAT_INCLUDE_DIRS} ) 

add_executable( auto360cam "src/main.cpp"
"src/glimpses.cpp"
"src/videoinfo.cpp"
"src/saliency.cpp"
"external/saliency/SalMapStentiford.cpp"
"external/saliency/SalMapMargolin.cpp" 
"external/saliency/SalMapItti.cpp" )
# "src/Arguments.cpp"
# "src/AutocropStentiford.cpp"
# "src/AutocropFang.cpp"
# "src/AutocropSuh.cpp"
# "src/CompositionModel.cpp")

add_library( vlfeats external/vlfeat/vl/host.c
external/vlfeat/vl/random.c
external/vlfeat/vl/generic.c
external/vlfeat/vl/slic.c )

# pthread
target_link_libraries( auto360cam ${OpenCV_LIBS} vlfeats)
