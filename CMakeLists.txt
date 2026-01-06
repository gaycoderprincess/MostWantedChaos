cmake_minimum_required(VERSION 3.27)
project(NFSMWChaos)

SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
SET(CMAKE_RANLIB i686-w64-mingw32-ranlib)

SET(CMAKE_CXX_STANDARD 20)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -masm=intel -s -static")

link_directories(${CMAKE_SOURCE_DIR}/../nya-common/lib32)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    link_directories($ENV{VCPKG_ROOT}/installed/x86-mingw-static/lib)
else()
    if(CMAKE_BUILD_TYPE MATCHES Debug)
        link_directories($ENV{VCPKG_ROOT}/installed/x86-windows-static/debug/lib)
    else()
        link_directories($ENV{VCPKG_ROOT}/installed/x86-windows-static/lib)
    endif()
endif()

add_compile_definitions(IMGUI_USER_CONFIG="../../nya_imconfig.h" CWOEECHAOS_VERSION="2.20" NYA_BACKEND_DX9 NYA_AUDIO_BASS NYA_FONTNAME="MenuStyles/MenuStyleBubbly/FranxurterTotallyMedium-gxwjp.ttf" NYA_FONTSIZE=32.0)
add_library(NFSMWChaos SHARED
        ../nya-common/3rdparty/imgui/imgui.cpp
        ../nya-common/3rdparty/imgui/backends/imgui_impl_dx9.cpp
        ../nya-common/3rdparty/imgui/imgui_draw.cpp
        ../nya-common/3rdparty/imgui/imgui_tables.cpp
        ../nya-common/3rdparty/imgui/imgui_widgets.cpp
        ../nya-common/nya_commonfuncs.cpp
        ../nya-common/nya_commondraw.cpp
        ../nya-common/nya_commontimer.cpp
        ../nya-common/nya_commonaudio.cpp
        ../nya-common/nya_dx9_hookbase.cpp
        IRCClient/src/IRCClient.cpp
        IRCClient/src/IRCHandler.cpp
        IRCClient/src/IRCSocket.cpp
        main.cpp)
target_include_directories(NFSMWChaos PUBLIC ../nya-common)
target_include_directories(NFSMWChaos PUBLIC ../nya-common/3rdparty)
target_include_directories(NFSMWChaos PUBLIC ../nya-common/3rdparty/imgui)
target_include_directories(NFSMWChaos PUBLIC ../nya-common/3rdparty/imgui/backends)
target_include_directories(NFSMWChaos PUBLIC ../nya-common-nfsmw)
target_include_directories(NFSMWChaos PUBLIC ../CwoeeMenuLib)
target_include_directories(NFSMWChaos PUBLIC ../CwoeeModelImporter)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_include_directories(NFSMWChaos PUBLIC $ENV{VCPKG_ROOT}/installed/x86-mingw-static/include)
else()
    target_include_directories(NFSMWChaos PUBLIC $ENV{VCPKG_ROOT}/installed/x86-windows-static/include)
endif()

# Windows Clang additions
if(MSVC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_include_directories(NFSMWChaos PUBLIC $ENV{DXSDK_DIR}Include)
    target_link_directories(NFSMWChaos PUBLIC $ENV{DXSDK_DIR}Lib/x86)
    target_compile_options(NFSMWChaos PRIVATE "-Wno-narrowing")
    target_compile_options(NFSMWChaos PRIVATE
            $<$<CONFIG:Debug>:/MTd>
            $<$<CONFIG:Release>:/MT>
            $<$<CONFIG:RelWithDebInfo>:/MT>
    )
endif()

target_link_options(NFSMWChaos PRIVATE "-Wl,--exclude-all-symbols")

set_target_properties(NFSMWChaos PROPERTIES PREFIX "")
set_target_properties(NFSMWChaos PROPERTIES SUFFIX "_gcp.asi")

target_link_libraries(NFSMWChaos d3dx9)
target_link_libraries(NFSMWChaos bass)
target_link_libraries(NFSMWChaos ws2_32)