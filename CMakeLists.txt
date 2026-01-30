cmake_minimum_required(VERSION 3.20)
project(ArtemisMoonOrbiterSim VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

include(FetchContent)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# glad
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
    SOURCE_SUBDIR cmake
)
FetchContent_MakeAvailable(glad)
glad_add_library(glad_gl_core_33 STATIC API gl:core=3.3)

# glm
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# imgui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.6
)
FetchContent_MakeAvailable(imgui)

# Create imgui library
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw)

# stb_image
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)

# Source files
set(SOURCES
    src/main.cpp
    src/core/Application.cpp
    src/core/Time.cpp
    src/physics/Integrator.cpp
    src/physics/Orbit.cpp
    src/physics/Spacecraft.cpp
    src/render/Renderer.cpp
    src/render/Camera.cpp
    src/render/Shader.cpp
    src/render/Mesh.cpp
    src/ui/Ui.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${stb_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    glad_gl_core_33
    glm::glm
    imgui
)

# Copy assets to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
endif()

# Install configuration
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets
    DESTINATION share/${PROJECT_NAME}
)

install(FILES ${CMAKE_SOURCE_DIR}/README.md
    DESTINATION share/${PROJECT_NAME}
)

# CPack configuration for Linux standalone package
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "LambDA-VN")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Artemis Moon Orbiter Simulation")
set(CPACK_PACKAGE_DESCRIPTION "An interactive C++ simulation of an Artemis-style spacecraft orbiting the Moon, with real-time rendering, UI controls, and basic mission-style features.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/LambDA-VN/ArtemisMoonOrbiterSim")
set(CPACK_RESOURCE_FILE_README ${CMAKE_SOURCE_DIR}/README.md)

# Linux-specific packaging
if(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "TGZ;ZIP")
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-linux-x86_64")
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
    
    # Install launcher script
    install(PROGRAMS ${CMAKE_SOURCE_DIR}/scripts/artemis-launcher.sh
        DESTINATION bin
        RENAME run-artemis.sh
    )
endif()

include(CPack)
