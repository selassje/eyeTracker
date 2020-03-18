cmake_minimum_required(VERSION 3.14)

include(CMakePrintHelpers)

project(EyeTracker CXX)

set(OPENCV_INCLUDE_DIRS "F:/Instalki/OpenCV/opencv-3.4.1/build"
                        "F:/Instalki/OpenCV/opencv-3.4.1/modules/core/include"
                        "F:/Instalki/OpenCV/opencv-3.4.1/modules/videoio/include"
                        "F:/Instalki/OpenCV/opencv-3.4.1/modules/objdetect/include"
                        "F:/Instalki/OpenCV/opencv-3.4.1/modules/highgui/include"
                        "F:/Instalki/OpenCV/opencv-3.4.1/modules/imgcodecs/include"
                        "F:/Instalki/OpenCV/opencv-3.4.1/modules/imgproc/include")


set(LINK_DIRS "F:/Instalki/OpenCV/opencv-3.4.1/build/lib/Release")

set(CMAKE_MFC_FLAG 2)
add_definitions(-D_AFXDLL -DUNICODE -D_UNICODE)

include_directories(BEFORE SYSTEM ${OPENCV_INCLUDE_DIRS} src/videoinput)
link_directories(${LINK_DIRS})

cmake_print_variables(INCLUDE_DIRS)

set_source_files_properties(src/EyeTracker.rc PROPERTIES LANGUAGE RC)

add_executable(EyeTracker WIN32
                          src/EyeTracker.cpp
                          src/EyeTrackerDlg.cpp
                          src/CustomTabCtrl.cpp
                          src/ComparerDlg.cpp
                          src/CameraDlg.cpp
                          src/ImageDlg.cpp
                          src/SettingDlg.cpp
                          src/LogDlg.cpp
                          src/ObjectDetection.cpp
                          src/stdafx.cpp
                          src/graph.cpp
                          src/videoinput/videoinput.cpp
                          src/EyeTracker.rc
                          src/res/EyeTracker.rc2
                          src/res/EyeTracker2.ico

)
target_link_libraries(EyeTracker "opencv_core341.lib" 
                                 "opencv_imgcodecs341.lib" 
                                 "opencv_highgui341.lib" 
                                 "opencv_imgproc341.lib" 
                                 "opencv_videoio341.lib"
                                 "opencv_objdetect341.lib"
                                 )