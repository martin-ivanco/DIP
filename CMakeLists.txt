cmake_minimum_required(VERSION 3.0)

project( auto360cam )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

find_package( OpenCV REQUIRED )
find_package( OpenMP )
if ( OpenMP_CXX_FOUND )
    set( OpenMP_LIBS OpenMP::OpenMP_CXX )
else ()
    set( OpenMP_LIBS "")
endif ()
set( BLA_VENDOR OpenBLAS )
find_package( BLAS )
if ( NOT BLAS_FOUND ) # dirty fix for OpenBLAS installation via Homebrew on macOS
    set( BLAS_INCLUDE_DIRS "/usr/local/opt/openblas/include" )
    set( BLAS_LIBS "/usr/local/opt/openblas/lib/libopenblas.dylib" )
endif ()
set( VLFEAT_INCLUDE_DIRS "external/vlfeat" )
set( DLIB_INCLUDE_DIRS "external/openface/lib/3rdParty/dlib" )
set( LANDMARK_INCLUDE_DIRS "external/openface/lib/local/LandmarkDetector/include" )

include_directories( ${OpenCV_INCLUDE_DIRS}
${BLAS_INCLUDE_DIRS}
${VLFEAT_INCLUDE_DIRS}
${DLIB_INCLUDE_DIRS}
${LANDMARK_INCLUDE_DIRS} ) 

add_executable( auto360cam "src/main.cpp"
"src/aid.cpp"
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
"external/saliency/MBS.cpp"
"external/saliency/SalMapItti.cpp"
"external/saliency/SalMapMargolin.cpp" 
"external/saliency/SalMapStentiford.cpp" )

add_library( vlfeats external/vlfeat/vl/host.c
external/vlfeat/vl/random.c
external/vlfeat/vl/generic.c
external/vlfeat/vl/slic.c )
add_subdirectory( external/openface/lib/3rdParty/dlib/dlib )
add_subdirectory( external/openface/lib/local/LandmarkDetector )

target_link_libraries( auto360cam ${OpenCV_LIBS} ${OpenMP_LIBS} ${BLAS_LIBS} vlfeats dlib LandmarkDetector ) # pthread stdc++fs - for older gcc
