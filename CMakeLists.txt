cmake_minimum_required(VERSION 3.14)
project(cog_soc_env_space VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# nlohmann/json
include(FetchContent)
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

add_executable(cog_soc_env_space
    src/main.cpp
    src/Builders.cpp
    src/Heterotroph.cpp
    src/Decomposer.cpp
    src/LivingBeing.cpp
    src/Autotroph.cpp
    src/Cohort.cpp
    src/Niche.cpp
    src/Environment.cpp
    src/EnvironmentConfig.cpp
    src/SimulationConfig.cpp
    src/Utilities.cpp
)

target_include_directories(cog_soc_env_space PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cog_soc_env_space PRIVATE nlohmann_json::nlohmann_json)

# Optional: install target
install(TARGETS cog_soc_env_space RUNTIME DESTINATION bin)
