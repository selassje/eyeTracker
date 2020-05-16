cmake_minimum_required(VERSION 3.14)

include(CMakePrintHelpers)

project(EyeTracker CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
add_definitions(-D_AFXDLL)
add_definitions(-DUNICODE -D_UNICODE)

if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.15/conan.cmake"
                 "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)

set(CONAN_COMMON_DEPS opencv/3.4.5@conan/stable)  
                  
macro(get_conan_deps ADDITIONAL_DEPS)
    conan_cmake_run(REQUIRES ${CONAN_COMMON_DEPS} ${ADDITIONAL_DEPS}
                    BASIC_SETUP CMAKE_TARGETS
                    BUILD missing)

    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
    link_directories(${CONAN_OPENCV_ROOT_RELEASE}/staticlib)
    link_directories(${CONAN_OPENCV_ROOT_DEBUG}/staticlib)
endmacro()

add_subdirectory(src)   
add_subdirectory(tests)
