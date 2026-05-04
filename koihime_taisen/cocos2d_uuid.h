#ifndef COCOS2D_UUID_H_
#define COCOS2D_UUID_H_

#include <string>

namespace cocos2d_uuid
{
	std::string DecodeUuid(std::string_view src);
	std::string EncodeUuid(std::string_view src);

	static constexpr size_t kDecodedLength = 36;
	static constexpr size_t kEncodedLength = 22;

	/// @brief Decode in buffer. Destination must be larger than 36 bytes.
	int DecodeUuidInBuffer(std::string_view src, char* dst);
	/// @brief Encode in buffer, Destination must be larger than 22 bytes.
	int EncodeUuidInBuffer(std::string_view src, char* dst);
}

#endif //COCOS2D_UUID_H_