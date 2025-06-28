cmake_minimum_required(VERSION 3.6)
project(irmc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

file(GLOB_RECURSE IRMC_SRC CONFIGURE_DEPENDS src/*.cpp)

add_subdirectory(vendor/glm)
add_subdirectory(vendor/raylib)

add_executable(irmc ${IRMC_SRC})

target_link_libraries(irmc PRIVATE
	glm raylib
)

target_include_directories(irmc PUBLIC
    src/
    src/thirdparty
    vendor/glm/glm
    vendor/raylib/src
)
