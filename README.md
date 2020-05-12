# Algorithms for Automatic Spherical Video Cropping
*Author:* Martin Ivanco
*E-mail:* xivanc03@stud.fit.vutbr.cz

Diploma thesis project at Brno University of Technology. The goal was to research and implement algorithms for automatic spherical video cropping. The application implements 3 such algorithms with several variations. The input should be a spherical video in the standard equirectangular projection. The output is a 2D video with stereographic projection that is a certain view from the input video. The algorithms try to choose that view in a way that the output would contain the most important content from the input.

## How to use
Most of the required libraries that are not standard are included in the `external` folder, however, there are several requirements for successful building and running the code.

### Requirements
* Compiler that supports C++17
* [OpenCV](https://opencv.org/) (at least version 3.0)
* [OpenBLAS](https://www.openblas.net/) -> please set the path to the library in the CMakeLists.txt file
* [C3D version 1.0](https://github.com/facebookarchive/C3D) (The binary for extracting C3D features is required for the AutoCam method. It needs to be built from the linked source on your system and the binary shall be placed inside the `external/c3d/` folder. For other methods, this is not required.)
* For using the AutoCam method, C3D features for training a linear regression classifier need to be present in the `external/c3d/features/` folder - see `external/c3d/features/README.md` for details on how to obtain them
* [OpenMP](https://www.openmp.org/) (optional)

### Building and running
1. Run `./run.sh build` to build the application
2. Run `./run.sh run -h` to display information about available arguments
3. Place a spherical video in the `data/input/` folder
4. Run the desired cropping algorithm, for example `./run.sh run -m aid-cont -s`
5. The output will be placed in the `data/output/` folder

## Folder structure
* `data/`
  * `c3d` -> extracted c3d features will be placed here
  * `glimpses` -> generated spatio-temporal glimpses will be placed here
  * `input` -> input spherical videos shall be placed here
  * `output` -> output videos and logs will be placed here
* `external/`
  * `c3d` -> C3D related requirements - trained model, protofiles, features
  * `crop` -> automatic cropping algorithms implemented by Ambrož
  * `openface` -> OpenFace framework required for face detection
  * `saliency` -> saliency mapping algorithms implemented by Ambrož and Zhang et al.
  * `vlfeat` -> vlfeat library required by algorithms by Ambrož
* `src` -> original source code written for this project
