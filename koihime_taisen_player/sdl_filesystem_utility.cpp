

#include "sdl_filesystem_utility.h"

#include <SDL3/SDL.h>

namespace sdl_filesystem_utility
{
	static constexpr size_t kMaxPathLength = 1024;
}

std::vector<std::string> sdl_filesystem_utility::CreateFilePaths(std::string_view directoryPath, const char* fileSpec, EPathType pathType)
{
	char pathBuffer[kMaxPathLength]{};

	if (directoryPath.length() >= kMaxPathLength)return {};

	memcpy(pathBuffer, directoryPath.data(), directoryPath.length());
	pathBuffer[directoryPath.length()] = '\0';

	int fileCount = 0;
	char** fileNames = ::SDL_GlobDirectory(pathBuffer, fileSpec, 0, &fileCount);
	if (fileNames == nullptr)return {};

	std::vector<std::string> filePaths;
	filePaths.reserve(fileCount);

	SDL_PathType matchType = pathType == EPathType::File ?
		SDL_PathType::SDL_PATHTYPE_FILE :
		SDL_PathType::SDL_PATHTYPE_DIRECTORY;

	for (int i = 0; i < fileCount; ++i)
	{
		/* Resize to the length of parent directory */
		size_t bufferLength = directoryPath.length();
		pathBuffer[bufferLength] = '\0';

		const char* fileName = fileNames[i];
		size_t fileNameLength = ::SDL_strlen(fileName);
		if (fileNameLength + bufferLength + 1 >= kMaxPathLength)continue;
		
		/* Append filename */
#if defined(_WIN32)
		pathBuffer[bufferLength++] = '\\';
#else
		pathBuffer[bufferLength++] = '/';
#endif
		memcpy(&pathBuffer[bufferLength], fileName, fileNameLength);
		bufferLength += fileNameLength;
		pathBuffer[bufferLength] = '\0';

		SDL_PathInfo sdlPathInfo{};
		bool bRet = ::SDL_GetPathInfo(pathBuffer, &sdlPathInfo);
		if (!bRet)continue;

		if (sdlPathInfo.type == matchType)
		{
			filePaths.emplace_back(pathBuffer);
		}
	}

	::SDL_free(fileNames);

	return filePaths;
}

std::string sdl_filesystem_utility::LoadFileAsString(const char* filePath)
{
	std::string file;
	SDL_IOStream* pIoStream = nullptr;
	Sint64 dataEnd = -1;
	Sint64 fileSize = 0;
	Sint64 dataStart = -1;
	size_t nRead = 0;

	pIoStream = ::SDL_IOFromFile(filePath, "rb");
	if (pIoStream == nullptr)goto end;

	dataEnd = ::SDL_SeekIO(pIoStream, 0, SDL_IOWhence::SDL_IO_SEEK_END);
	if (dataEnd == -1)goto end;

	fileSize = ::SDL_TellIO(pIoStream);
	if (fileSize == -1)goto end;

	dataStart = ::SDL_SeekIO(pIoStream, 0, SDL_IOWhence::SDL_IO_SEEK_SET);
	if (dataStart == -1)goto end;

	file.resize(fileSize);
	nRead = ::SDL_ReadIO(pIoStream, &file[0], fileSize);

end:
	if (pIoStream != nullptr)
	{
		::SDL_CloseIO(pIoStream);
	}

	return file;
}
