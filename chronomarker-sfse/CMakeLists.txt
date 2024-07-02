cmake_minimum_required(VERSION 3.27)

project(
  chronomarker-sfse
  VERSION 1.0.0
  LANGUAGES CXX
)

set(VCPKG_TARGET_TRIPLET x64-windows-static)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(ZeroMQ CONFIG REQUIRED)

add_subdirectory(sfse/sfse_common)
add_subdirectory(sfse/sfse)
set_target_properties(
	sfse_common
	PROPERTIES
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
set_target_properties(
	sfse
	PROPERTIES
		MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

add_library(${PROJECT_NAME} SHARED)
target_link_libraries(${PROJECT_NAME} PUBLIC
    libzmq-static
    sfse64::sfse
)

include(sfse/cmake/versioning.cmake)
target_compile_definitions(
	${PROJECT_NAME}
	PUBLIC
		RUNTIME_VERSION=${RUNTIME_VERSION_PACKED}
)

file(GLOB sources CONFIGURE_DEPENDS sfse/sfse/*.cpp)
target_sources(${PROJECT_NAME} PUBLIC
    main.cpp
	commonlibsf-rel/VersionDatabase.h
	commonlibsf-rel/VersionDatabase.cpp
    ${sources}
)

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> E:/SteamLibrary/steamapps/common/Starfield/Data/SFSE/plugins
  COMMAND
    ${CMAKE_COMMAND} -E copy $<TARGET_PDB_FILE:${PROJECT_NAME}> E:/SteamLibrary/steamapps/common/Starfield/Data/SFSE/plugins
)
