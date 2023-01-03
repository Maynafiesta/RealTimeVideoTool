# SmartIR Video Tool for Encoding, Decoding and Testing Purposes

## Compile Guide

There are 2 quick build shell script file for debug and release version. This scripts creates a build folder and compiles project inside.
At that compile directory, you can run the executable smirVideoTool by just `./smirVideoTool` after compilation.

To activate Realsense support, you need to build it by yourself. Realsense argument support will be added to these quick build scripts.
To do this, create a build folder and type `cmake .. -D REALSENSE_SDK=ON` to activate Realsense support.

## Run Guide

Related guide is ready when you just use `-h` or `--help` arguments with executable. `./smirVideoTool -h`

## Notes

Now, all the libraries are sharedly linked. So you need to install FFMPEG, OpenCV and optionaly Realsense SDK properly. 


