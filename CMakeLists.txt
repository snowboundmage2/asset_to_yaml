cmake_minimum_required(VERSION 3.10)

# Set the project name
project(AssetToYamlCPP)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add the executable
add_executable(${PROJECT_NAME}
    main.cpp
    AT_Animation.h
    AT_Binary.h
    AT_DemoButton.h
    AT_Dialogue.h
    AT_GruntyQuestion.h
    AT_LevelSetup.h
    AT_MidiSeq.h
    AT_Model.h
    AT_QuizQuestion.h
    AT_Sprite.h
    AT_Texture.h
    Asset.h
    AssetEntry.h
    AssetFactory.h
    AssetFolder.h
    AssetMeta.h
    AssetType.h
    BKString.h
    DecompressionUtils.h
    ContInput.h
    ImgFmt.h
    SpriteChunk.h
    SpriteFrame.h
    StringToVecu8.h
    Vecu8ToString.h
)

# Include directories
include_directories(
    #skipping png support for now
    #${PROJECT_SOURCE_DIR}/tools/pngpp
    ${PROJECT_SOURCE_DIR}/tools/yamlcpp
    ${PROJECT_SOURCE_DIR}/tools/zlib
)

add_subdirectory(tools/yamlcpp)
add_subdirectory(tools/zlib)


target_link_libraries(${PROJECT_NAME} yaml-cpp zlib)

# Add any additional libraries if needed
# target_link_libraries(MyProject <library_name>)