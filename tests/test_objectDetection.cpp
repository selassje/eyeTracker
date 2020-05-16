#include "ObjectDetection.hpp"
#include "opencv2/imgcodecs.hpp"
#include <filesystem>
#include <iostream>
#include "gtest/gtest.h"

TEST(objectDetection, testDetectFaces)
{
    CObjectDetection::Init();

    constexpr int expectedFacesDetected = 28;
    const std::string path = "bioid";
    std::error_code error;
    auto detectedFaces = 0;
    auto dir_it = std::filesystem::directory_iterator(path, error);

    if (error) {
        std::cout << error.message() << std::endl;
    } else {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            CStringA filePath(CString { entry.path().c_str() });
            const auto img = cv::imread(static_cast<const char*>(filePath));
            if (!img.empty()) {
                auto face = CObjectDetection::DetectFace(img);
                if (face) {
                    ++detectedFaces;
                }
            }
        }
    }
    EXPECT_EQ(detectedFaces, 28);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}