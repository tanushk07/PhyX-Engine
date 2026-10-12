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
add_library(phyx_math INTERFACE
        src/math/Vector.h
        src/math/Quaternion.h
        src/math/Mat3.h
)
add_library(phyx::math ALIAS phyx_math)
target_include_directories(phyx_math INTERFACE src/math)
target_compile_features(phyx_math INTERFACE cxx_std_20)

# ---- phyx_core: the physics engine (no raylib)
add_library(phyx_core STATIC
        src/physics/Engine.cpp
        src/physics/Engine.h
        src/physics/bodies/RigidBody.cpp
        src/physics/bodies/RigidBody.h
        src/physics/bodies/Inertia.h
        src/physics/bodies/Shape.h
        src/physics/bodies/BodyDesc.h
        src/physics/collision/AABB.cpp
        src/physics/collision/AABB.h
        src/physics/collision/BoundingSphere.cpp
        src/physics/collision/BoundingSphere.h
        src/physics/collision/OBB.cpp
        src/physics/collision/OBB.h
        src/physics/collision/PlaneCollider.cpp
        src/physics/collision/PlaneCollider.h
        src/physics/collision/IntersectionData.h
        src/physics/collision/Contact.h
)
add_library(phyx::core ALIAS phyx_core)
target_include_directories(phyx_core PUBLIC src/physics src/physics/bodies src/physics/collision)
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

    # raylib 5.5 bug: with CUSTOMIZE_BUILD it copies BOTH branches of an #ifdef in config.h,
    # so it gets MAX_MESH_VERTEX_BUFFERS=9 and =7. The 7 wins, but GPU skinning is on and writes
    # 9 buffer ids per mesh -> heap corruption ("Debug Error!" when the app closes).
    get_target_property(RAYLIB_DEFS raylib COMPILE_DEFINITIONS)
    list(REMOVE_ITEM RAYLIB_DEFS "MAX_MESH_VERTEX_BUFFERS=7")
    set_target_properties(raylib PROPERTIES COMPILE_DEFINITIONS "${RAYLIB_DEFS}")

    if(MINGW AND CMAKE_C_COMPILER_VERSION VERSION_LESS 8)
        target_include_directories(raylib PRIVATE
                "${CMAKE_CURRENT_SOURCE_DIR}/cmake/old-mingw-compat"
        )
    endif()

    add_executable(phyX
            src/demo/main.cpp
            src/demo/app/DemoState.h
            src/demo/app/Controls.cpp
            src/demo/app/Controls.h
            src/demo/app/Interaction.cpp
            src/demo/app/Interaction.h
            src/demo/chapters/Chapters.cpp
            src/demo/chapters/Chapters.h
            src/demo/render/Visualizer.cpp
            src/demo/render/Visualizer.h
            src/demo/render/SceneDrawing.cpp
            src/demo/render/SceneDrawing.h
            src/demo/render/LitShader.h
            src/demo/render/RaylibConvert.h
            src/demo/hud/Theme.cpp
            src/demo/hud/Theme.h
            src/demo/hud/Hud.cpp
            src/demo/hud/Hud.h
    )
    target_include_directories(phyX PRIVATE src/demo/app src/demo/chapters src/demo/render src/demo/hud)
    target_link_libraries(phyX PRIVATE phyx_core raylib)

    # fonts are loaded from <exe folder>/assets/fonts
    add_custom_command(TARGET phyX POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:phyX>/assets)
endif()

# ---- phyX_tests: console-only math checks (no raylib, no window)
if(PHYX_BUILD_TESTS)
    enable_testing()
    add_executable(phyX_tests src/tests/MathTests.cpp)
    target_link_libraries(phyX_tests PRIVATE phyx_math)
    add_test(NAME math COMMAND phyX_tests)
endif()