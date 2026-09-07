cmake_minimum_required(VERSION 3.16)
project(OxTarget VERSION 1.0.0 LANGUAGES CXX)

option(OXTARGET_BUILD_PLUGIN "Build the SA-MP/open.mp legacy plugin" ON)
option(OXTARGET_BUILD_TESTS "Build unit tests" ON)
option(OXTARGET_BUILD_BENCHMARK "Build the standalone spatial/ray benchmark" ON)
option(OXTARGET_BUILD_SAMP "Build the legacy SA-MP compatible adapter" ON)
option(OXTARGET_BUILD_OPENMP "Document/build compatibility with open.mp legacy_plugins" ON)
option(OXTARGET_BUILD_X86 "Build 32-bit binaries (required by legacy SA-MP)" OFF)
option(OXTARGET_BUILD_X64 "Build native 64-bit binaries (recommended for open.mp x64)" ON)

if(OXTARGET_BUILD_X86 AND OXTARGET_BUILD_X64)
    message(FATAL_ERROR "Choose one architecture per build directory: X86 or X64")
endif()
if(OXTARGET_BUILD_X86 AND NOT MSVC)
    add_compile_options(-m32)
    add_link_options(-m32)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(oxtarget_core STATIC
    src/core/Intersection.cpp
    src/core/SpatialIndex.cpp
    src/core/EntityRegistry.cpp
    src/core/RayEngine.cpp
    src/core/TargetManager.cpp
    src/core/PlayerTargetState.cpp
)
target_include_directories(oxtarget_core PUBLIC src)
set_target_properties(oxtarget_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    target_compile_options(oxtarget_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(oxtarget_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

if(OXTARGET_BUILD_PLUGIN)
    include(FetchContent)
    FetchContent_Declare(samp_plugin_sdk
        GIT_REPOSITORY https://github.com/maddinat0r/samp-plugin-sdk.git
        GIT_TAG a5ce36a9b6ebbea6ad36705603f653bf3d4f41c5
        GIT_SHALLOW FALSE
    )
    FetchContent_MakeAvailable(samp_plugin_sdk)

    add_library(oxtarget SHARED
        src/plugin.cpp
        src/pawn/Natives.cpp
        src/pawn/Callbacks.cpp
        src/platform/samp/SampAdapter.cpp
        src/platform/openmp/OpenMpAdapter.cpp
        src/adapters/ColAndreasAdapter.cpp
        src/adapters/StreamerAdapter.cpp
        ${samp_plugin_sdk_SOURCE_DIR}/amxplugin.cpp
    )
    target_link_libraries(oxtarget PRIVATE oxtarget_core)
    target_include_directories(oxtarget PRIVATE src ${samp_plugin_sdk_SOURCE_DIR})
    target_compile_definitions(oxtarget PRIVATE $<$<PLATFORM_ID:Linux>:LINUX>)
    if(WIN32)
        target_compile_definitions(oxtarget PRIVATE HAVE_STDINT_H)
    endif()
    set_target_properties(oxtarget PROPERTIES PREFIX "" OUTPUT_NAME "oxtarget")
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            target_sources(oxtarget PRIVATE src/oxtarget-x64.def)
        elseif(MSVC)
            target_sources(oxtarget PRIVATE src/oxtarget-msvc32.def)
        else()
            target_sources(oxtarget PRIVATE src/oxtarget.def)
        endif()
    endif()
    if(MINGW)
        target_link_options(oxtarget PRIVATE -static)
    elseif(UNIX AND NOT APPLE)
        target_link_options(oxtarget PRIVATE -static-libstdc++ -static-libgcc)
    endif()
    if(MSVC)
        target_compile_options(oxtarget PRIVATE /W4 /permissive-)
    else()
        target_compile_options(oxtarget PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow -fvisibility=hidden)
    endif()

    add_custom_command(TARGET oxtarget POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:oxtarget>/pawno/include"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/include/oxtarget.inc"
            "$<TARGET_FILE_DIR:oxtarget>/pawno/include/oxtarget.inc"
    )
    install(TARGETS oxtarget RUNTIME DESTINATION plugins LIBRARY DESTINATION plugins)
    install(FILES include/oxtarget.inc DESTINATION pawno/include)
    install(DIRECTORY examples/ DESTINATION examples FILES_MATCHING PATTERN "*.pwn")
    install(FILES README.md LICENSE CHANGELOG.md DESTINATION .)
endif()

if(OXTARGET_BUILD_TESTS)
    enable_testing()
    add_executable(oxtarget_tests tests/tests.cpp)
    target_link_libraries(oxtarget_tests PRIVATE oxtarget_core)
    add_test(NAME oxtarget_tests COMMAND oxtarget_tests)
endif()

if(OXTARGET_BUILD_BENCHMARK)
    add_executable(oxtarget_benchmark tests/benchmark.cpp)
    target_link_libraries(oxtarget_benchmark PRIVATE oxtarget_core)
endif()
