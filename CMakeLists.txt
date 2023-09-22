cmake_minimum_required(VERSION 3.15)
project(navigator)

find_package(Bullet REQUIRED)
find_package(CURL REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(lua REQUIRED)
find_package(OpenAL REQUIRED)
find_package(SndFile REQUIRED)
find_package(stb REQUIRED)
find_package(watcher REQUIRED)

IF(DEFINED imgui_INCLUDE_DIRS_DEBUG)
set(imgui_RES_DIRS "${imgui_INCLUDE_DIRS_DEBUG}/../res/")
ELSE()
set(imgui_RES_DIRS "${imgui_INCLUDE_DIRS_RELEASE}/../res/")
ENDIF()

include_directories(
  ${imgui_RES_DIRS}/bindings
)

file(GLOB SOURCE_FILES
  "src/*.cpp"
  "src/core/lib/*.c"
  "src/core/lib/*.cpp"
  "src/core/*.cpp"
  "src/gl/primitives/*.cpp"
  "src/gl/textures/*.cpp"
  "src/gl/voxels/*.cpp"
  "src/gl/*.cpp"
  ${imgui_RES_DIRS}/bindings/imgui_impl_glfw.cpp
  ${imgui_RES_DIRS}/bindings/imgui_impl_opengl3.cpp
)

IF(WIN32)
  list(APPEND SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/win32.rc)
  set_property(
    SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/win32.rc APPEND PROPERTY OBJECT_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/icon.ico
  )
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") 
ENDIF()

add_subdirectory(src/core/lib/FastNoise2)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_compile_definitions(
  ${PROJECT_NAME} PRIVATE
  VERSION="0.0.1"
  VERNUM1=0 VERNUM2=0 VERNUM3=1 VERNUM4=0
)
target_link_libraries(
  ${PROJECT_NAME}
  Bullet::Bullet
  CURL::libcurl
  FastNoise
  glad::glad
  glfw
  glm::glm
  imgui::imgui
  lua::lua
  OpenAL::OpenAL
  SndFile::sndfile
  stb::stb
  watcher::watcher
)
IF(WIN32)
target_link_libraries(${PROJECT_NAME} Dwmapi)
ENDIF()
