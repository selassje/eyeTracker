file(COPY ../src/res/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR} FILES_MATCHING PATTERN "*.xml")
file(CREATE_LINK  ${CMAKE_CURRENT_BINARY_DIR}/../../tests/bioid/ ${CMAKE_CURRENT_BINARY_DIR}/bioid SYMBOLIC)

include_directories(../src)

add_executable(TestObjectDetection test_objectDetection.cpp ../src/ObjectDetection.cpp ../src/ObjectDetection.hpp)
conan_target_link_libraries(TestObjectDetection)
add_test(TestObjectDetection TestObjectDetection)

set_tests_properties(TestObjectDetection
                     PROPERTIES
                     PASS_REGULAR_EXPRESSION "Test passed")
set_tests_properties(TestObjectDetection
                     PROPERTIES
                     FAIL_REGULAR_EXPRESSION "(Exception|Test failed)")
set_tests_properties(TestObjectDetection
                     PROPERTIES
                     TIMEOUT 120)