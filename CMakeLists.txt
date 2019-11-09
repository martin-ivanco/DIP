cmake_minimum_required(VERSION 3.0)

project( auto360cam )

# find_package( Boost REQUIRED COMPONENTS filesystem ) 
# find_package( Threads )
find_package( OpenCV REQUIRED )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17" )

# change your local PATH of VLFeat here!
# set( VLFEAT_INCLUDE_DIRS "vlfeat" )

# ${VLFEAT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS}
include_directories( ${OpenCV_INCLUDE_DIRS} ) 

add_executable( auto360cam "src/main.cpp"
"src/glimpses.cpp"
"src/videoinfo.cpp" )
# "src/Arguments.cpp"
# "src/AutocropStentiford.cpp"
# "src/AutocropFang.cpp"
# "src/AutocropSuh.cpp"
# "src/SalMapStentiford.cpp"
# "src/SalMapMargolin.cpp" 
# "src/SalMapItti.cpp"
# "src/CompositionModel.cpp")

# change your local PATH of VLFeat here!
# add_library( vlfeats vlfeat/vl/host.c vlfeat/vl/random.c vlfeat/vl/generic.c vlfeat/vl/slic.c )

# ${Boost_LIBRARIES} vlfeats pthread
target_link_libraries( auto360cam ${OpenCV_LIBS} )
