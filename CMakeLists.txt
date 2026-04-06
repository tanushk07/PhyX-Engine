cmake_minimum_required(VERSION 3.14...3.28)
project(phyX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

include(FetchContent)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

set(SOURCES
    src/main.cpp
    src/Engine.cpp
    src/core/RigidBody.cpp
    src/colliders/AABB.cpp
    src/colliders/BoundingSphere.cpp
    src/colliders/PlaneCollider.cpp
    src/utils/Visualizer.cpp
)

add_executable(phyX ${SOURCES})
target_include_directories(phyX PRIVATE src/core src/colliders src/utils)
target_link_libraries(phyX PRIVATE raylib)
