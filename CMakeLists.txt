cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
        src/main.cpp
        src/context.cpp
        src/render2d.cpp
        src/swapchain.cpp
        src/shader.cpp
        src/tool.cpp
        src/render_process.cpp
        src/renderer.cpp
        src/commandManager.cpp
        src/buffer.cpp
        include/vertex.h
)

# Add executable
if(WIN32)
    add_executable(My_Learn_Vulkan WIN32 ${My_Learn_Vulkan-SRC})
else()
    add_executable(My_Learn_Vulkan ${My_Learn_Vulkan-SRC})
endif()

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

# decode shader
find_program(GLSLC_PROGRAM glslc REQUIRED)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.vert -o ${CMAKE_SOURCE_DIR}/vert.spv)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.frag -o ${CMAKE_SOURCE_DIR}/frag.spv)