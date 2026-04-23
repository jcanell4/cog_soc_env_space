cmake_minimum_required(VERSION 3.14)
project(cog_soc_env_space VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(ENABLE_RAYLIB_VIEWER "Build raylib-based snapshot viewer" OFF)

# nlohmann/json (FetchContent)
# CMP0168 OLD: use legacy FetchContent sub-build; avoids pkgRedirects / redirect issues on some CMake 4.x CI runners.
if(POLICY CMP0168)
    cmake_policy(SET CMP0168 OLD)
endif()
include(FetchContent)
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)
FetchContent_Declare(json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# Platform-specific settings
if(WIN32)
    add_definitions(-DCOG_SOC_ENV_SPACE_PLATFORM_WINDOWS)
elseif(APPLE)
    add_definitions(-DCOG_SOC_ENV_SPACE_PLATFORM_MACOS)
elseif(UNIX)
    add_definitions(-DCOG_SOC_ENV_SPACE_PLATFORM_LINUX)
endif()

set(COG_SOC_ENV_SPACE_LIB_SOURCES
    src/Builders.cpp
    src/JsonEcosystem.cpp
    src/JsonFrameSource.cpp
    src/Heterotroph.cpp
    src/Decomposer.cpp
    src/ConsumerLivingBeing.cpp
    src/LivingBeing.cpp
    src/Autotroph.cpp
    src/Cohort.cpp
    src/Niche.cpp
    src/Environment.cpp
    src/EnvironmentConfig.cpp
    src/SimulationSnapshotReader.cpp
    src/SimulationConfig.cpp
    src/Utilities.cpp
)

add_executable(cog_soc_env_space
    src/main.cpp
    ${COG_SOC_ENV_SPACE_LIB_SOURCES}
)

target_include_directories(cog_soc_env_space PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cog_soc_env_space PRIVATE nlohmann_json::nlohmann_json)

# Local test runner: not part of default/all target, so CI `cmake --build build` only builds main app.
add_executable(cog_soc_env_space_tests EXCLUDE_FROM_ALL
    src/test_app.cpp
    ${COG_SOC_ENV_SPACE_LIB_SOURCES}
)
target_include_directories(cog_soc_env_space_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(cog_soc_env_space_tests PRIVATE nlohmann_json::nlohmann_json)

if(ENABLE_RAYLIB_VIEWER)
    find_package(raylib QUIET)
    if(NOT raylib_FOUND)
        # Allow building on minimal Linux environments lacking optional X11 dev headers.
        if(UNIX AND NOT APPLE)
            set(GLFW_BUILD_XRANDR OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_XINERAMA OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_XCURSOR OFF CACHE BOOL "" FORCE)
            set(GLFW_BUILD_XINPUT OFF CACHE BOOL "" FORCE)
        endif()
        FetchContent_Declare(raylib
            GIT_REPOSITORY https://github.com/raysan5/raylib.git
            GIT_TAG 5.5
        )
        FetchContent_MakeAvailable(raylib)
    endif()

    if(TARGET raylib)
        set(COG_SOC_ENV_SPACE_RAYLIB_TARGET raylib)
    elseif(TARGET raylib::raylib)
        set(COG_SOC_ENV_SPACE_RAYLIB_TARGET raylib::raylib)
    else()
        message(FATAL_ERROR "Raylib target not found after dependency setup")
    endif()

    add_executable(cog_soc_env_space_viewer
        src/viewer_main.cpp
        src/JsonFrameSource.cpp
        src/SimulationSnapshotReader.cpp
    )
    target_include_directories(cog_soc_env_space_viewer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_compile_definitions(cog_soc_env_space_viewer PRIVATE COG_SOC_ENV_SPACE_HAS_RAYLIB=1)
    target_link_libraries(cog_soc_env_space_viewer PRIVATE
        nlohmann_json::nlohmann_json
        ${COG_SOC_ENV_SPACE_RAYLIB_TARGET}
    )
endif()

# Optional: install target
install(TARGETS cog_soc_env_space RUNTIME DESTINATION bin)
if(ENABLE_RAYLIB_VIEWER)
    install(TARGETS cog_soc_env_space_viewer RUNTIME DESTINATION bin)
endif()
install(FILES README.md LICENSE DESTINATION .)
install(DIRECTORY config DESTINATION .)
if(UNIX AND NOT APPLE)
    install(
        FILES
            scripts/linux/run_sim.sh
            scripts/linux/run_viewer.sh
        DESTINATION bin
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
    )
endif()
