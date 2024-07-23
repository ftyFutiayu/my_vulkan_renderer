cmake_minimum_required(VERSION 3.5)

project(My_Learn_Vulkan)

find_package(OpenGL REQUIRED)
find_package(Vulkan REQUIRED)

# Include directories
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/glm)
if(DEFINED GLFW_SOURCE_DIR)
    include_directories("${GLFW_SOURCE_DIR}/deps")
endif()

# GLFW configuration
set(GLFW_BUILD_DOCS OFF CACHE BOOL "GLFW lib only")
set(GLFW_INSTALL OFF CACHE BOOL "GLFW lib only")

add_subdirectory(glfw)

# Option to use GLAD from GLFW
option(My_Learn_Vulkan-USE-GLFW-GLAD "Use GLAD from GLFW" ON)

# GLAD header
if(My_Learn_Vulkan-USE-GLFW-GLAD)
    set(GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h")
endif()

# Set source files
set(My_Learn_Vulkan-SRC
        main.cpp
        render2d/context.cpp
        render2d/render2d.cpp
        render2d/tool.h
        render2d/swapchain.cpp
        render2d/swapchain.h
)

# Add executable
add_executable(My_Learn_Vulkan WIN32 ${My_Learn_Vulkan-SRC})

# Link libraries with keyword arguments
target_link_libraries(My_Learn_Vulkan PUBLIC
        ${OPENGL_LIBRARIES}
        glfw
        Vulkan::Vulkan
)

# MSVC specific linker flags
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
    if(${CMAKE_VERSION} VERSION_LESS "3.6.0")
        message("\n\t[ WARNING ]\n\n\tCMake version lower than 3.6.\n\n\t - Please update CMake and rerun; OR\n\t - Manually set 'My_Learn_Vulkan' as StartUp Project in Visual Studio.\n")
    else()
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT My_Learn_Vulkan)
    endif()
endif()