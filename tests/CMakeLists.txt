include(GoogleTest)

file(COPY ../src/res/ DESTINATION ${CMAKE_CURRENT_BINARY_DIR} FILES_MATCHING PATTERN "*.xml")
file(CREATE_LINK  ${CMAKE_CURRENT_BINARY_DIR}/../../tests/bioid/ ${CMAKE_CURRENT_BINARY_DIR}/bioid SYMBOLIC)

get_conan_deps(gtest/1.8.1@bincrafters/stable)

include_directories(../src)

add_executable(TestObjectDetection test_objectDetection.cpp ../src/ObjectDetection.cpp ../src/ObjectDetection.hpp)

conan_target_link_libraries(TestObjectDetection)
gtest_add_tests(TARGET TestObjectDetection                 
                TEST_SUFFIX .noArgs
                TEST_LIST   noArgsTests)

