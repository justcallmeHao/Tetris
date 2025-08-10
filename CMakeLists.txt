cmake_minimum_required(VERSION 3.16)
project(Tetris CXX)
set(CMAKE_CXX_STANDARD 17)

# Point to the local raylib folder
set(RAYLIB_DIR "${CMAKE_SOURCE_DIR}/raylib")

add_executable(Tetris src/main.cpp)

# Copies the whole assets folder to the build output dir every build
add_custom_command(TARGET Tetris POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:Tetris>/assets")

# Tell the compiler where raylib.h lives
target_include_directories(Tetris PRIVATE "${RAYLIB_DIR}/include")

# Tell the linker where the .a/.dll import lib lives
target_link_directories(Tetris PRIVATE "${RAYLIB_DIR}/lib")

# Link libraries (MinGW)
target_link_libraries(Tetris PRIVATE raylib opengl32 gdi32 winmm)

# Copy raylib.dll next to the built .exe
add_custom_command(TARGET Tetris POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${RAYLIB_DIR}/lib/raylib.dll"
        "$<TARGET_FILE_DIR:Tetris>/raylib.dll")


