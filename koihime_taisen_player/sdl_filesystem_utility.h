#ifndef SDL_FILESYSTEM_UTILITY_H_
#define SDL_FILESYSTEM_UTILITY_H_

#include <string>
#include <vector>

namespace sdl_filesystem_utility
{
	/// @brief Path type to enumerate
	enum class EPathType : uint8_t
	{
		File,
		Directory,
	};

	/// @brief Create file or folder path list in the directory
	[[nodiscard]] std::vector<std::string> CreateFilePaths(std::string_view directoryPath, const char* fileSpec, EPathType pathType);

	/// @brief Load file as string even if it were binary format
	[[nodiscard]] std::string LoadFileAsString(const char* filePath);
}

#endif // !SDL_FILESYSTEM_UTILITY_H_
