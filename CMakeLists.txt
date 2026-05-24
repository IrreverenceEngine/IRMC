cmake_minimum_required(VERSION 3.6)
project(irmc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

file(GLOB_RECURSE IRMC_SRC CONFIGURE_DEPENDS src/*.cpp)

add_subdirectory(vendor/IRX)

set(SDL_EXAMPLES OFF)
add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)

set(RECASTNAVIGATION_DEMO OFF)
set(RECASTNAVIGATION_TESTS OFF)
set(RECASTNAVIGATION_EXAMPLES OFF)
add_subdirectory(vendor/recastnavigation)

add_executable(irmc ${IRMC_SRC})

target_link_libraries(irmc PRIVATE
    glm::glm SDL3-static RecastNavigation::Recast RecastNavigation::Detour irx
)

target_include_directories(irmc PUBLIC
    src/
    src/thirdparty
    irx
    vendor/SDL/include
    vendor/recastnavigation/Recast/Include
    vendor/recastnavigation/Detour/Include
)
