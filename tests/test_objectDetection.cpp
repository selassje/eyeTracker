#include "ObjectDetection.hpp"
#include "opencv2/imgcodecs.hpp"
#include <filesystem>
#include <iostream>


int main() {
    CObjectDetection::Init();
    
    constexpr int expectedFacesDetected = 28;
    const std::string path = "bioid";
    std::error_code error;
    auto detectedFaces = 0;
    auto dir_it = std::filesystem::directory_iterator(path, error);

    if (error) {
        std::cout << error.message() << std::endl;
        return error.value();
    } 
    else {     
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

    if (detectedFaces != expectedFacesDetected) {
        std::cout << "Detected faces: " << detectedFaces << std::endl;
        std::cout << "Test failed" << std::endl;
    } else {
        std::cout << "Test passed" << std::endl;
    }
    return 0;
}