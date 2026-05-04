cmake_minimum_required(VERSION 3.20)
project(KoihimeTaisenPlayer)

set(DEPS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../deps")
set(SDL_DIR "${DEPS_DIR}/SDL3-3.4.4")
set(SDL_IMAGE_DIR "${DEPS_DIR}/SDL3_image-3.4.2")
set(SDL_TTF_DIR "${DEPS_DIR}/SDL3_ttf-3.2.2")
set(SDL_MIXER_DIR "${DEPS_DIR}/SDL3_mixer-3.2.0")
set(IMGUI_DOCKING_DIR "${DEPS_DIR}/imgui-docking")
set(SPINE_GENERIC_RUNTIME_DIR "${DEPS_DIR}/spine-cpp-3.8")

set(SDL_SPINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdl3-spine-cpp")
set(SDL_IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sdl3-imgui")

set(TARGET_NAME "KoihimeTaisenPlayer")

add_executable(${TARGET_NAME} WIN32
	"${CMAKE_CURRENT_SOURCE_DIR}/dpiAwareness.manifest"
	"main.cpp"
	"adv.h"
	"text_utility.h"
	"path_utility.h"
	"../koihime_taisen/json_minimal.cpp" "../koihime_taisen/json_minimal.h"
	"koihime_taisen.cpp" "koihime_taisen.h"
	"koihime_taisen_scene_player.cpp" "koihime_taisen_scene_player.h"
	"sdl_main_window.cpp" "sdl_main_window.h"
	"sdl_clock.cpp" "sdl_clock.h"
	"sdl_text_drawer.cpp" "sdl_text_drawer.h"
	"sdl_audio_player.cpp" "sdl_audio_player.h"
	"sdl_filesystem_utility.cpp" "sdl_filesystem_utility.h"
	"sdl_string_utility.cpp" "sdl_string_utility.h"
	"setup.cpp" "setup.h"
	"${SDL_SPINE_DIR}/sdl_spine.cpp" "${SDL_SPINE_DIR}/sdl_spine.h"
	"${SDL_SPINE_DIR}/sdl_spine_extension.cpp" "${SDL_SPINE_DIR}/sdl_spine_extension.h"
	"${SDL_SPINE_DIR}/sdl_spine_loader.cpp" "${SDL_SPINE_DIR}/sdl_spine_loader.h"
	"${SDL_SPINE_DIR}/sdl_spine_player.cpp" "${SDL_SPINE_DIR}/sdl_spine_player.h"
	"${SDL_SPINE_DIR}/spine_file_verifier.cpp" "${SDL_SPINE_DIR}/spine_file_verifier.h"
	"${SDL_SPINE_DIR}/spine_player.cpp" "${SDL_SPINE_DIR}/spine_player.h"
	"${IMGUI_DOCKING_DIR}/imconfig.h"
	"${IMGUI_DOCKING_DIR}/imgui.cpp" "${IMGUI_DOCKING_DIR}/imgui.h"
	"${IMGUI_DOCKING_DIR}/imgui_demo.cpp"
	"${IMGUI_DOCKING_DIR}/imgui_draw.cpp"
	"${IMGUI_DOCKING_DIR}/imgui_internal.h"
	"${IMGUI_DOCKING_DIR}/imgui_tables.cpp"
	"${IMGUI_DOCKING_DIR}/imgui_widgets.cpp"
	"${IMGUI_DOCKING_DIR}/backends/imgui_impl_sdl3.cpp" "${IMGUI_DOCKING_DIR}/backends/imgui_impl_sdl3.h"
	"${IMGUI_DOCKING_DIR}/backends/imgui_impl_sdlrenderer3.cpp" "${IMGUI_DOCKING_DIR}/backends/imgui_impl_sdlrenderer3.h"
	"${SDL_IMGUI_DIR}/sdl_renderer_imgui_init.cpp" "${SDL_IMGUI_DIR}/sdl_renderer_imgui_init.h"
)

target_include_directories(${TARGET_NAME} PRIVATE
	"${SDL_DIR}/include"
	"${SDL_IMAGE_DIR}/include"
	"${SDL_TTF_DIR}/include"
	"${SDL_MIXER_DIR}/include"
	"${SPINE_GENERIC_RUNTIME_DIR}/include"
	${IMGUI_DOCKING_DIR}
)

# Identify architecture
if((CMAKE_SYSTEM_PROCESSOR MATCHES "(arm64|aarch64|ARM64)"))
	set (BUILD_TARGET "arm64")
else()
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set (BUILD_TARGET "x64")
	elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set (BUILD_TARGET "x86")
	endif()
endif()

target_link_libraries(${TARGET_NAME} PRIVATE
	"${SDL_DIR}/lib/${BUILD_TARGET}/SDL3.lib"
	"${SDL_IMAGE_DIR}/lib/${BUILD_TARGET}/SDL3_image.lib"
	"${SDL_TTF_DIR}/lib/${BUILD_TARGET}/SDL3_ttf.lib"
	"${SDL_MIXER_DIR}/lib/${BUILD_TARGET}/SDL3_mixer.lib"
	"spine-cpp-3.8"
)

target_compile_definitions(
	${TARGET_NAME} PRIVATE 
	UNICODE _UNICODE
	SDL_SPINE_CPP
	SPINE_38
	PRIORITISE_USING_DEDICATED_GPU
)

target_precompile_headers(${TARGET_NAME} PRIVATE
	<string>
	<vector>
	<memory>
	<algorithm>
	<array>
	<map>
)

if(MSVC)
	target_compile_definitions(${TARGET_NAME} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
	target_compile_options(${TARGET_NAME} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/d1trimfile:${CMAKE_SOURCE_DIR}\\>")
	target_link_options(${TARGET_NAME} PRIVATE "/pdbaltpath:%_PDB%")
endif()
