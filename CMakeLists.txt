cmake_minimum_required(VERSION 3.14)
project(env_soc_cog_space VERSION 0.1.0 LANGUAGES CXX)

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
    add_definitions(-DENV_SOC_COG_SPACE_PLATFORM_WINDOWS)
elseif(APPLE)
    add_definitions(-DENV_SOC_COG_SPACE_PLATFORM_MACOS)
elseif(UNIX)
    add_definitions(-DENV_SOC_COG_SPACE_PLATFORM_LINUX)
endif()

add_executable(env_soc_cog_space
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

target_include_directories(env_soc_cog_space PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(env_soc_cog_space PRIVATE nlohmann_json::nlohmann_json)

# Optional: install target
install(TARGETS env_soc_cog_space RUNTIME DESTINATION bin)
