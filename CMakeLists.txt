cmake_minimum_required(VERSION 3.10)

# Set the project name
project(AssetToYamlCPP)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(${PROJECT_NAME}
    main.cpp
    AnimationAsset.h
    Asset.h
    AssetEntry.h
    AssetFolder.h
    AssetMeta.h
    AssetType.h
    BinaryAsset.h
    BKString.h
    ContInput.h
    DemoButton.h
    DialogueAsset.h
    GruntyQuestion.h
    ImgFmt.h
    LevelSetup.h
    MidiSeq.h
    ModelAsset.h
    QuizQuestion.h
    Sprite.h
    SpriteChunk.h
    SpriteFrame.h
    StringToVecu8.h
    Texture.h
    TextureAsset.h
    Vecu8ToString.h
)

# Include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/tools/pngpp
    ${PROJECT_SOURCE_DIR}/tools/yamlcpp
)

add_subdirectory(tools/yamlcpp)

target_link_libraries(${PROJECT_NAME} yaml-cpp)

# Add any additional libraries if needed
# target_link_libraries(MyProject <library_name>)