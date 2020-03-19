cmake_minimum_required(VERSION 3.14)

include(CMakePrintHelpers)

project(EyeTracker CXX)

set_source_files_properties(src/EyeTracker.rc PROPERTIES LANGUAGE RC)
set(CMAKE_MFC_FLAG 2)
add_definitions(-D_AFXDLL -DUNICODE -D_UNICODE)
                          

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.15/conan.cmake"
                 "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)
conan_cmake_run(CONANFILE conanfile.txt
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_release.cmake)
link_directories(${CONAN_OPENCV_ROOT_RELEASE}/staticlib)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_debug.cmake)
link_directories(${CONAN_OPENCV_ROOT_DEBUG}/staticlib)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
include_directories(${CONAN_INCLUDE_DIRS_OPENCV})     

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
                          src/EyeTracker.rc
                          src/res/EyeTracker.rc2
                          src/res/EyeTracker2.ico)

conan_target_link_libraries(EyeTracker)