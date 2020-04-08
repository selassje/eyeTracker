set(CMAKE_MFC_FLAG 2)
add_definitions(-D_AFXDLL -DUNICODE -D_UNICODE)
                          

file(GLOB_RECURSE SRC
    "*.cpp"
    "*.h"
    "*.hpp"
    "*.odl"
)

file(GLOB_RECURSE RES
    "res/*"
    "*.rc"
)

if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.15/conan.cmake"
                 "${CMAKE_CURRENT_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_CURRENT_BINARY_DIR}/conan.cmake)

conan_add_remote(NAME selassje URL https://api.bintray.com/conan/selassje/conan)

conan_cmake_run(REQUIRES opencv/3.4.5@conan/stable NTGraph/0.1@conan/stable
                BASIC_SETUP CMAKE_TARGETS
                BUILD missing)

include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
link_directories(${CONAN_OPENCV_ROOT_RELEASE}/staticlib)
link_directories(${CONAN_OPENCV_ROOT_DEBUG}/staticlib)
include_directories(${CONAN_INCLUDE_DIRS_OPENCV})     

add_executable(EyeTracker WIN32 ${SRC})
target_sources(${PROJECT_NAME} PRIVATE ${RES})
set_source_files_properties(src/EyeTracker.rc PROPERTIES LANGUAGE RC)
source_group("Resource Files" FILES ${RES})

conan_target_link_libraries(EyeTracker)

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIGUPPER )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER} ${CMAKE_BINARY_DIR}/${OUTPUTCONFIG})
    file(COPY src/res/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIGUPPER}} FILES_MATCHING PATTERN "*.xml")
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

set_target_properties(EyeTracker PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})

