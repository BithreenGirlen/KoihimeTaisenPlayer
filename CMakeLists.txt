cmake_minimum_required (VERSION 3.20)
project (KoihimeTaisen)

set(CMAKE_CXX_STANDARD 20)

# Hot reload
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Download external libraries required for the player
add_subdirectory (deps)

# Build something
add_subdirectory (koihime_taisen)

# Build player
add_subdirectory (koihime_taisen_player)
