# CMakeLists.txt: Core CMake file

cmake_minimum_required (VERSION 3.8)

project("NV1Sim")

# Add source to this project's executable.
add_executable (NV1Sim 

"NV1Sim.cpp"

# Core
"core/core.cpp"
"core/core_input.cpp"
"core/core_renderer.cpp"

# NV1
"nv/core/nv1_core.cpp"

)

# Base include directories
include_directories(".")
# Required for glad for now
include_directories(${CMAKE_SOURCE_DIR}"/external/sdl3/x86_64-w64-mingw32/include")

# Libraries and copiler specific stuff

add_library(sdl3 STATIC IMPORTED)

set_target_properties(sdl3 PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/external/sdl3/x86_64-w64-mingw32/lib/libSDL3.dll.a)
target_link_libraries(NV1Sim sdl3)


# Build system-specific files
#if (WIN32)
#	target_sources(NV1Sim PUBLIC "sys/windows/sys_win32.cpp")
#else()
#	target_sources(NV1Sim PUBLIC "sys/linux/sys_linux.cpp")
#endif()

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET NV1Sim PROPERTY CXX_STANDARD 20)
endif()

#todo: linux
if (WIN32)
    file(COPY ${CMAKE_SOURCE_DIR}/external/sdl3/x86_64-w64-mingw32/bin/SDL3.dll DESTINATION ${CMAKE_BINARY_DIR})
endif()