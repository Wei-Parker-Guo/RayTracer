# Original file Copyright CMU462 Fall 2015: 
# Kayvon Fatahalian, Keenan Crane,
# Sky Gao, Bryce Summers, Michael Choquette.
cmake_minimum_required(VERSION 2.8)

# Application source

# enable this to build starter code
# set(APPLICATION_SOURCE
#     example_01.cpp
# )

set(ASSESSMENT2_SOURCE
    "shaders/sketch_shader.cpp"
    "shaders/translucent_shader.cpp"
    "shaders/layered_toon_shader.cpp"
    "shaders/basic_shaders.cpp"
    materials.cpp
    lights.cpp
    cameras.cpp
    fast_math.cpp
    geometry.cpp
    ray.cpp
    raypool.cpp
    rasterizer.cpp
    render_thread.cpp
    main.cpp
)

# enable this to build GPU version
# set(ASSESSMENT1_GPU_SOURCE
#     layered_toon_shader.cpp
#     basic_shaders.cpp
#     fast_math.cpp
#     assessment_1cu.cu
# )

#-------------------------------------------------------------------------------
# Set include directories
#-------------------------------------------------------------------------------
include_directories(
  ${GLEW_INCLUDE_DIRS}
  ${GLFW_INCLUDE_DIRS}
  ${ASSIMP_INCLUDE_DIRS}
#  ${FREETYPE_INCLUDE_DIRS} disabled freetype since it causes malfunc on my Win10 system
)

#-------------------------------------------------------------------------------
# Set link directories
#-------------------------------------------------------------------------------
link_directories(
  ${GLEW_LIBRARY_DIRS}
  ${GLFW_LIBRARY_DIRS}
  ${ASSIMP_LIBRARY_DIRS}
#  ${CGL_LIBRARY_DIRS}
#  ${FREETYPE_LIBRARY_DIRS}
)

#-------------------------------------------------------------------------------
# Add executable
#-------------------------------------------------------------------------------

# enable this to build cuda GPU code
# add_executable(as1cu ${ASSESSMENT1_GPU_SOURCE})
# target_link_libraries( as1cu
#      glew ${GLEW_LIBRARIES}
#      glfw ${GLFW_LIBRARIES}
#      ${OPENGL_LIBRARIES}
#  #    ${FREETYPE_LIBRARIES}
#      ${CMAKE_THREADS_INIT}
#  )

# enable this to build GPU version
# # Request that particles be built with -std=c++11
# # As this is a public compile feature anything that links to 
# # particles will also build with -std=c++11
# target_compile_features(as1cu PUBLIC cxx_std_11)
 
# # We need to explicitly state that we need all CUDA files in the 
# # particle library to be built with -dc as the member functions 
# # could be called by other libraries and executables
# set_target_properties( as1cu
#                        PROPERTIES CUDA_SEPARABLE_COMPILATION ON)


add_executable(raytracer ${ASSESSMENT2_SOURCE})

target_link_libraries( raytracer
    glew ${GLEW_LIBRARIES}
    glfw ${GLFW_LIBRARIES}
    assimp ${ASSIMP_LIBRARIES}
    ${OPENGL_LIBRARIES}
#    ${FREETYPE_LIBRARIES}
    ${CMAKE_THREADS_INIT}
)

#-------------------------------------------------------------------------------
# Platform-specific configurations for target
#-------------------------------------------------------------------------------
if(APPLE)
  set_property( TARGET raytracer APPEND_STRING PROPERTY COMPILE_FLAGS
                "-Wno-deprecated-declarations -Wno-c++11-extensions")
endif(APPLE)

# Put executable in build directory root
set(EXECUTABLE_OUTPUT_PATH ..)

# Install to project root, add as1cu to build GPU version also
install(TARGETS raytracer DESTINATION ${Assignment2_SOURCE_DIR})
