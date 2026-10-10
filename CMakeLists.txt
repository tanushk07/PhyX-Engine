cmake_minimum_required(VERSION 3.14...3.28)
project(phyX LANGUAGES C CXX)

# Is phyX being built on its own, or pulled in by another project (a renderer)?
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(PHYX_IS_TOP_LEVEL ON)
else()
    set(PHYX_IS_TOP_LEVEL OFF)
endif()
option(PHYX_BUILD_DEMO  "Build the raylib demo"   ${PHYX_IS_TOP_LEVEL})
option(PHYX_BUILD_TESTS "Build the console tests" ${PHYX_IS_TOP_LEVEL})

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# ---- phyx_math: Vec3, Quat, Mat3 (headers only)
add_library(phyx_math INTERFACE)
add_library(phyx::math ALIAS phyx_math)
target_include_directories(phyx_math INTERFACE src/Math)
target_compile_features(phyx_math INTERFACE cxx_std_20)

# ---- phyx_core: the physics engine (no raylib)
add_library(phyx_core STATIC
        src/Engine.cpp
        src/Engine.h
        src/core/RigidBody.cpp
        src/core/RigidBody.h
        src/core/IntersectionData.h
        src/core/Inertia.h
        src/colliders/AABB.cpp
        src/colliders/AABB.h
        src/colliders/BoundingSphere.cpp
        src/colliders/BoundingSphere.h
        src/colliders/OBB.cpp
        src/colliders/OBB.h
        src/colliders/PlaneCollider.cpp
        src/colliders/PlaneCollider.h
        src/utils/BodyDesc.h
        src/utils/Shape.h
        src/utils/Contact.h
)
add_library(phyx::core ALIAS phyx_core)
target_include_directories(phyx_core PUBLIC src src/core src/colliders src/utils)
target_link_libraries(phyx_core PUBLIC phyx_math)

# ---- phyX: the raylib demo (only when phyX is built on its own)
if(PHYX_BUILD_DEMO)
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

    include(FetchContent)
    FetchContent_Declare(
            raylib
            URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
    )
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    set(CUSTOMIZE_BUILD ON CACHE BOOL "" FORCE)
    set(USE_AUDIO OFF CACHE BOOL "" FORCE)
    set(SUPPORT_MODULE_RAUDIO OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(raylib)

    if(MINGW AND CMAKE_C_COMPILER_VERSION VERSION_LESS 8)
        target_include_directories(raylib PRIVATE
                "${CMAKE_CURRENT_SOURCE_DIR}/cmake/old-mingw-compat"
        )
    endif()

    add_executable(phyX
            src/main.cpp
            src/utils/Visualizer.cpp
            src/utils/Visualizer.h
    )
    target_link_libraries(phyX PRIVATE phyx_core raylib)
endif()

# ---- phyX_tests: console-only math checks (no raylib, no window)
if(PHYX_BUILD_TESTS)
    enable_testing()
    add_executable(phyX_tests src/tests/MathTests.cpp)
    target_link_libraries(phyX_tests PRIVATE phyx_math)
    add_test(NAME math COMMAND phyX_tests)
endif()