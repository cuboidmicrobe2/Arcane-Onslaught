cmake_minimum_required(VERSION 3.21)
project(ArcaneOnslaught LANGUAGES CXX)

include(FetchContent)

find_package(raylib CONFIG REQUIRED)
find_package(unofficial-lua CONFIG REQUIRED)

FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG v3.15.0
)
FetchContent_MakeAvailable(entt)

add_executable(ArcaneOnslaught
    src/main.cpp
    src/engine/EngineAPI.cpp
)

target_include_directories(ArcaneOnslaught PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_compile_features(ArcaneOnslaught PRIVATE cxx_std_17)
target_link_libraries(ArcaneOnslaught PRIVATE
    raylib
    unofficial::lua::lua
    EnTT::EnTT
)

if(WIN32)
    target_link_libraries(ArcaneOnslaught PRIVATE winmm gdi32)
endif()

add_custom_command(TARGET ArcaneOnslaught POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/scripts
            $<TARGET_FILE_DIR:ArcaneOnslaught>/scripts
    COMMENT "Copying Lua scripts to output directory"
)

add_custom_command(TARGET ArcaneOnslaught POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:ArcaneOnslaught>
            $<TARGET_FILE_DIR:ArcaneOnslaught>
    COMMAND_EXPAND_LISTS
)
