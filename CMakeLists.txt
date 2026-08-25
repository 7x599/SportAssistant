cmake_minimum_required(VERSION 3.24)

project(SportAssistant VERSION 1.0.0 LANGUAGES CXX)
if(MSVC)
    add_compile_options(/utf-8)
endif()

option(SPORTASSISTANT_BUILD_GUI "Build the Qt/OpenCV desktop application" ON)
option(SPORTASSISTANT_BUILD_TESTS "Build deterministic core tests" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(sport_core STATIC
    src/exercise/Geometry.cpp
    src/exercise/PoseSmoother.cpp
    src/exercise/SquatAnalyzer.cpp
    src/exercise/PushUpAnalyzer.cpp
    src/training/TrainingSession.cpp
    src/training/TrainingRecord.cpp
)
target_include_directories(sport_core PUBLIC include)
target_compile_features(sport_core PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(sport_core PRIVATE /W4 /permissive- /utf-8)
    target_compile_definitions(sport_core PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(sport_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(SPORTASSISTANT_BUILD_GUI)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets)
    find_package(OpenCV 4 REQUIRED COMPONENTS core imgproc videoio dnn)
    qt_standard_project_setup()

    add_library(sport_vision STATIC
        src/vision/PoseDetector.cpp
        src/vision/DemoPoseSource.cpp
        src/vision/VideoPipeline.cpp
    )
    target_include_directories(sport_vision PUBLIC include ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(sport_vision PUBLIC sport_core ${OpenCV_LIBS})

    qt_add_executable(SportAssistant
        src/main.cpp
        src/ui/MainWindow.cpp
        include/ui/MainWindow.h
    )
    qt_add_resources(SportAssistant "styles"
        PREFIX "/styles"
        FILES resources/dark.qss
    )
    target_link_libraries(SportAssistant PRIVATE Qt6::Widgets sport_vision sport_core)
    if(MSVC)
        target_compile_options(SportAssistant PRIVATE /W4 /permissive- /utf-8)
        target_compile_definitions(SportAssistant PRIVATE NOMINMAX _CRT_SECURE_NO_WARNINGS)
    endif()

    install(TARGETS SportAssistant
        BUNDLE DESTINATION .
        RUNTIME DESTINATION .
    )
    install(DIRECTORY models DESTINATION . OPTIONAL)
endif()

if(SPORTASSISTANT_BUILD_TESTS)
    enable_testing()
    add_executable(sport_core_tests tests/core_tests.cpp)
    target_link_libraries(sport_core_tests PRIVATE sport_core)
    add_test(NAME sport_core_tests COMMAND sport_core_tests)
endif()
