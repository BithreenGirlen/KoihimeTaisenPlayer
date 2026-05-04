cmake_minimum_required (VERSION 3.25)
project(KoihimeTaisenPlayer-deps)

set(SDL_MAJOR_VERSION "3")

set(SDL_MINOR_VERSION ".4.4")
set(SDL_IMAGE_MINOR_VERSION ".4.2")
set(SDL_TTF_MINOR_VERSION ".2.2")
set(SDL_MIXER_MINOR_VERSION ".2.0")

set(SDL_VERSION ${SDL_MAJOR_VERSION}${SDL_MINOR_VERSION})
set(SDL_IMAGE_VERSION ${SDL_MAJOR_VERSION}${SDL_IMAGE_MINOR_VERSION})
set(SDL_TTF_VERSION ${SDL_MAJOR_VERSION}${SDL_TTF_MINOR_VERSION})
set(SDL_MIXER_VERSION ${SDL_MAJOR_VERSION}${SDL_MIXER_MINOR_VERSION})

set(SDL_MAJOR_VERSION_PREFIX "SDL${SDL_MAJOR_VERSION}")

set(SDL_DIR "${CMAKE_CURRENT_LIST_DIR}/${SDL_MAJOR_VERSION_PREFIX}-${SDL_VERSION}")
set(SDL_IMAGE_DIR "${CMAKE_CURRENT_LIST_DIR}/${SDL_MAJOR_VERSION_PREFIX}_image-${SDL_IMAGE_VERSION}")
set(SDL_TTF_DIR "${CMAKE_CURRENT_LIST_DIR}/${SDL_MAJOR_VERSION_PREFIX}_ttf-${SDL_TTF_VERSION}")
set(SDL_MIXER_DIR "${CMAKE_CURRENT_LIST_DIR}/${SDL_MAJOR_VERSION_PREFIX}_mixer-${SDL_MIXER_VERSION}")

set(IMGUI_VERSION "1.92.7-docking")
set(IMGUI_DIR "${CMAKE_CURRENT_LIST_DIR}/imgui-docking")

set(SPINE_VERSION "3.8")
set(SPINE_RUNTIME_LANGUAGE "spine-cpp")
set(SPINE_RUNTIME_DIR ${CMAKE_CURRENT_LIST_DIR}/${SPINE_RUNTIME_LANGUAGE}-${SPINE_VERSION})

set(TEMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/temp)

# In times of SDL2, there was not a folder indicating its major version, so one added it for clarity.
# But in SDL3, this process is no longer necessary because it already has.
if(SDL_MAJOR_VERSION GREATER_EQUAL 3)
	set(SDL_INCLUDE_DIR "include/SDL3")
	set(SDL_IMAGE_INCLUDE_DIR "include/SDL3_image")
	set(SDL_TTF_INCLUDE_DIR "include/SDL3_ttf")
	set(SDL_MIXER_INCLUDE_DIR "include/SDL3_mixer")
else() # SDL2
	set(SDL_INCLUDE_DIR "include")
	set(SDL_IMAGE_INCLUDE_DIR "include")
	set(SDL_TTF_INCLUDE_DIR "include")
	set(SDL_MIXER_INCLUDE_DIR "include")
endif()

function(download_and_extract HOST_URL ARCHIVE_NAME)
	if (NOT EXISTS "${TEMP_DIR}/${ARCHIVE_NAME}.zip")
		file(DOWNLOAD "${HOST_URL}" "${TEMP_DIR}/${ARCHIVE_NAME}.zip" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${TEMP_DIR}/${ARCHIVE_NAME}")
		file(ARCHIVE_EXTRACT INPUT "${TEMP_DIR}/${ARCHIVE_NAME}.zip" DESTINATION "${TEMP_DIR}")
	endif()
endfunction()

function(copy_files SRC_GLOB DST_DIR MATCH_CONDITION)
	file(GLOB SRC_FILES "${SRC_GLOB}")
	foreach(FILE IN LISTS SRC_FILES)
		cmake_path(GET FILE FILENAME FILINOMEN)
		if (NOT MATCH_CONDITION OR NOT FILINOMEN MATCHES "${MATCH_CONDITION}")
			file(COPY ${FILE} DESTINATION "${DST_DIR}")
		endif()
	endforeach()
endfunction()

function(replace_names_in_file FILE_PATH OLD_NAME NEW_NAME)
	file(READ ${FILE_PATH} FILE)
	string(REPLACE ${OLD_NAME} ${NEW_NAME} MODIFIED_FILE "${FILE}")
	file(WRITE ${FILE_PATH} "${MODIFIED_FILE}")
endfunction()

# Download SDL binaries for VC
if (NOT EXISTS "${SDL_DIR}")
	set(SDL_VC_BUILT "SDL${SDL_MAJOR_VERSION}-devel-${SDL_VERSION}-VC")
	download_and_extract("https://github.com/libsdl-org/SDL/releases/download/release-${SDL_VERSION}/${SDL_VC_BUILT}.zip" ${SDL_VC_BUILT})
	
	copy_files("${TEMP_DIR}/${SDL_MAJOR_VERSION_PREFIX}-${SDL_VERSION}/${SDL_INCLUDE_DIR}/*.h" "${SDL_DIR}/include/${SDL_MAJOR_VERSION_PREFIX}" "test")

	copy_files("${TEMP_DIR}/${SDL_MAJOR_VERSION_PREFIX}-${SDL_VERSION}/lib/*" "${SDL_DIR}/lib" "test")
endif()

set(
	HEADERS_TO_RENAME
	"\"SDL.h\":<${SDL_MAJOR_VERSION_PREFIX}/SDL.h>"
	"\"SDL_version.h\":<${SDL_MAJOR_VERSION_PREFIX}/SDL_version.h>"
	"\"begin_code.h\":<${SDL_MAJOR_VERSION_PREFIX}/begin_code.h>"
	"\"close_code.h\":<${SDL_MAJOR_VERSION_PREFIX}/close_code.h>"
)

# Download SDL_image for VC
if (NOT EXISTS "${SDL_IMAGE_DIR}")
	set(SDL_IMAGE_VC_BUILT "${SDL_MAJOR_VERSION_PREFIX}_image-devel-${SDL_IMAGE_VERSION}-VC")
	download_and_extract("https://github.com/libsdl-org/SDL_image/releases/download/release-${SDL_IMAGE_VERSION}/${SDL_IMAGE_VC_BUILT}.zip" ${SDL_IMAGE_VC_BUILT})

	copy_files("${TEMP_DIR}/${SDL_MAJOR_VERSION_PREFIX}_image-${SDL_IMAGE_VERSION}/${SDL_IMAGE_INCLUDE_DIR}/*.h" "${SDL_IMAGE_DIR}/include/${SDL_MAJOR_VERSION_PREFIX}_image" "")
	copy_files("${TEMP_DIR}/${SDL_MAJOR_VERSION_PREFIX}_image-${SDL_IMAGE_VERSION}/lib/*" "${SDL_IMAGE_DIR}/lib" "")

	# Rename include header
	foreach(HEADERS IN LISTS HEADERS_TO_RENAME)
		string(REPLACE ":" ";" HEADER ${HEADERS})
		list(GET HEADER 0 OLD_HEADER)
		list(GET HEADER 1 NEW_HEADER)
		replace_names_in_file("${SDL_IMAGE_DIR}/include/${SDL_MAJOR_VERSION_PREFIX}_image/SDL_image.h" ${OLD_HEADER} ${NEW_HEADER})
	endforeach()
endif()

# Download SDL_TTF for VC
if (NOT EXISTS "${SDL_TTF_DIR}")
	set(SDL_TTF_VC_BUILT "${SDL_MAJOR_VERSION_PREFIX}_ttf-devel-${SDL_TTF_VERSION}-VC")
	download_and_extract("https://github.com/libsdl-org/SDL_ttf/releases/download/release-${SDL_TTF_VERSION}/${SDL_TTF_VC_BUILT}.zip" ${SDL_TTF_VC_BUILT})

	copy_files("${TEMP_DIR}/${SDL_MAJOR_VERSION_PREFIX}_ttf-${SDL_TTF_VERSION}/${SDL_TTF_INCLUDE_DIR}/*.h" "${SDL_TTF_DIR}/include/${SDL_MAJOR_VERSION_PREFIX}_ttf" "")
	copy_files("${TEMP_DIR}/${SDL_MAJOR_VERSION_PREFIX}_ttf-${SDL_TTF_VERSION}/lib/*" "${SDL_TTF_DIR}/lib" "")

	# Rename include header
	foreach(HEADERS IN LISTS HEADERS_TO_RENAME)
		string(REPLACE ":" ";" HEADER ${HEADERS})
		list(GET HEADER 0 OLD_HEADER)
		list(GET HEADER 1 NEW_HEADER)
		replace_names_in_file("${SDL_TTF_DIR}/include/${SDL_MAJOR_VERSION_PREFIX}_ttf/SDL_ttf.h" ${OLD_HEADER} ${NEW_HEADER})
	endforeach()
endif()

# Download SDL_mixer for VC
if (NOT EXISTS "${SDL_MIXER_DIR}")
	set(SDL_MIXER_VC_BUILT "${SDL_MAJOR_VERSION_PREFIX}_mixer-devel-${SDL_MIXER_VERSION}-VC")
	download_and_extract("https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDL_MIXER_VERSION}/${SDL_MIXER_VC_BUILT}.zip" ${SDL_MIXER_VC_BUILT})

	copy_files("${TEMP_DIR}/${SDL_MAJOR_VERSION_PREFIX}_mixer-${SDL_MIXER_VERSION}/${SDL_MIXER_INCLUDE_DIR}/*.h" "${SDL_MIXER_DIR}/include/${SDL_MAJOR_VERSION_PREFIX}_mixer" "")
	copy_files("${TEMP_DIR}/${SDL_MAJOR_VERSION_PREFIX}_mixer-${SDL_MIXER_VERSION}/lib/*" "${SDL_MIXER_DIR}/lib" "")

	# Rename include header
	foreach(HEADERS IN LISTS HEADERS_TO_RENAME)
		string(REPLACE ":" ";" HEADER ${HEADERS})
		list(GET HEADER 0 OLD_HEADER)
		list(GET HEADER 1 NEW_HEADER)
		replace_names_in_file("${SDL_MIXER_DIR}/include/${SDL_MAJOR_VERSION_PREFIX}_mixer/SDL_mixer.h" ${OLD_HEADER} ${NEW_HEADER})
	endforeach()
endif()

# Download Dear-ImGui docking
if (NOT EXISTS "${IMGUI_DIR}")
	set(IMGUI_SRC_URL "https://github.com/ocornut/imgui/archive/refs/tags/v${IMGUI_VERSION}.zip")
	set(IMGUI_SRC_ARCHIVE_PATH "${TEMP_DIR}/${IMGUI_VERSION}")

	if (NOT EXISTS "${IMGUI_SRC_ARCHIVE_PATH}.zip")
		file(DOWNLOAD "${IMGUI_SRC_URL}" "${IMGUI_SRC_ARCHIVE_PATH}.zip" SHOW_PROGRESS)
	endif()

	if (NOT EXISTS "${IMGUI_SRC_ARCHIVE_PATH}")
		file(ARCHIVE_EXTRACT INPUT "${IMGUI_SRC_ARCHIVE_PATH}.zip" DESTINATION "${TEMP_DIR}")
	endif()

	file(COPY "${TEMP_DIR}/imgui-${IMGUI_VERSION}/" DESTINATION "${IMGUI_DIR}")
endif()

# Download Spine generic runtime
if (NOT EXISTS "${SPINE_RUNTIME_DIR}/src")
	set (SPINE_RUNTIME_URL "https://github.com/EsotericSoftware/spine-runtimes/archive/refs/heads/${SPINE_VERSION}.zip")
	set (SPINE_FOLDER_NAME "spine-runtimes-${SPINE_VERSION}")
	download_and_extract("${SPINE_RUNTIME_URL}" "${SPINE_FOLDER_NAME}")

	file(COPY "${TEMP_DIR}/${SPINE_FOLDER_NAME}/${SPINE_RUNTIME_LANGUAGE}/${SPINE_RUNTIME_LANGUAGE}/src" DESTINATION "${SPINE_RUNTIME_DIR}")
	file(COPY "${TEMP_DIR}/${SPINE_FOLDER_NAME}/${SPINE_RUNTIME_LANGUAGE}/${SPINE_RUNTIME_LANGUAGE}/include" DESTINATION "${SPINE_RUNTIME_DIR}")
endif()

# Delete temporary files
file(REMOVE_RECURSE "${TEMP_DIR}")

# Build Spine generic runtime as static library
add_subdirectory (spine-cpp-3.8)

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

# Install prebuilt dynamic libraries to the executable directory
install(
	FILES
	"${SDL_DIR}/lib/${BUILD_TARGET}/${SDL_MAJOR_VERSION_PREFIX}.dll"
	"${SDL_IMAGE_DIR}/lib/${BUILD_TARGET}/${SDL_MAJOR_VERSION_PREFIX}_image.dll"
	"${SDL_IMAGE_DIR}/lib/${BUILD_TARGET}/optional/libwebp-7.dll"
	"${SDL_IMAGE_DIR}/lib/${BUILD_TARGET}/optional/libwebpdemux-2.dll"
	"${SDL_IMAGE_DIR}/lib/${BUILD_TARGET}/optional/libwebpmux-3.dll"
	"${SDL_IMAGE_DIR}/lib/${BUILD_TARGET}/optional/LICENSE.webp.txt"
	"${SDL_TTF_DIR}/lib/${BUILD_TARGET}/${SDL_MAJOR_VERSION_PREFIX}_ttf.dll"
	"${SDL_MIXER_DIR}/lib/${BUILD_TARGET}/${SDL_MAJOR_VERSION_PREFIX}_mixer.dll"
	"setting.txt"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/../koihime_taisen_player"
)
