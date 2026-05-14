cmake_minimum_required(VERSION 3.16)
project(MahirTV VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2       REQUIRED sdl2)
pkg_check_modules(SDL2_TTF   REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(GLEW       REQUIRED glew)

find_package(OpenGL REQUIRED)

set(SOURCES
    src/main.cpp
    src/App.cpp
    src/Shader.cpp
    src/TextRenderer.cpp
    src/screens/LanguageScreen.cpp
    src/screens/AccountScreen.cpp
    src/screens/SaveDataScreen.cpp
    src/screens/LoadingScreen.cpp
    src/screens/HomeScreen.cpp
)

add_executable(MahirTV ${SOURCES})

target_include_directories(MahirTV PRIVATE
    src
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(MahirTV PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${GLEW_LIBRARIES}
    ${OPENGL_LIBRARIES}
)

target_compile_definitions(MahirTV PRIVATE
    ASSETS_PATH="${CMAKE_SOURCE_DIR}/assets/"
)

install(TARGETS MahirTV DESTINATION bin)
install(DIRECTORY assets DESTINATION share/mahirtv)
