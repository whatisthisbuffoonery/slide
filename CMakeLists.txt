cmake_minimum_required(VERSION 3.20)
project(SlideReadable LANGUAGES C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(slide_core
    src/math/vec3.c src/math/vec3_utils.c
    src/content/course.cpp src/game/game.cpp src/game/physics.cpp src/audio/wav.cpp)
target_include_directories(slide_core PUBLIC src third_party)
add_executable(slide_tests tests/tests.cpp)
target_link_libraries(slide_tests PRIVATE slide_core)
if(MSVC)
    target_compile_options(slide_tests PRIVATE /UNDEBUG)
else()
    target_compile_options(slide_tests PRIVATE -UNDEBUG)
endif()
enable_testing()
add_test(NAME content_and_gameplay COMMAND slide_tests "${CMAKE_CURRENT_SOURCE_DIR}/assets")
if(WIN32)
    add_executable(Slide WIN32
        src/main.cpp src/app.cpp src/smoke_test.cpp
        src/platform/windows.cpp src/platform/input.cpp src/audio/audio.cpp
        src/render/d3d_init.cpp src/render/draw_frame.cpp
        src/ui/drawing.cpp src/ui/message_cards.cpp src/ui/screens.cpp)
    target_compile_definitions(Slide PRIVATE UNICODE _UNICODE NOMINMAX)
    target_link_libraries(Slide PRIVATE slide_core d3d11 dxgi d3dcompiler winmm gdi32 user32)
    if(MINGW)
        target_link_options(Slide PRIVATE -static -municode)
    endif()
    add_custom_command(TARGET Slide POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:Slide>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/shaders" "$<TARGET_FILE_DIR:Slide>/shaders"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/PLAY.txt" "$<TARGET_FILE_DIR:Slide>/PLAY.txt")
endif()
