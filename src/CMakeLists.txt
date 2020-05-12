set(CMAKE_MFC_FLAG 2)

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

