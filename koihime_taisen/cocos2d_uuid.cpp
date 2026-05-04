
#include "cocos2d_uuid.h"

namespace cocos2d_uuid
{
	static constexpr std::string_view g_base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	static constexpr std::string_view g_hexDigits = "0123456789abcdef";
}

std::string cocos2d_uuid::DecodeUuid(std::string_view src)
{
	const size_t nPos = src.find('@');
	if (nPos != std::string_view::npos)
	{
		src.remove_suffix(src.length() - nPos);
	}
	if (src.size() != 22)return std::string(src);

	char decodedBuffer[30 + 1]{};
	char* pWritten = decodedBuffer;
	for (size_t i = 2; i < 22; i += 2)
	{
		size_t l = g_base64.find(src[i]);
		size_t r = g_base64.find(src[i + 1]);
		if (l == std::string_view::npos || r == std::string_view::npos)return {};

		*pWritten++ = g_hexDigits[l >> 2];
		*pWritten++ = g_hexDigits[((l & 3) << 2) | r >> 4];
		*pWritten++ = g_hexDigits[r & 0xF];
	}

	std::string decoded;
	decoded.append(&src[0], 2);
	decoded.append(&decodedBuffer[0], 6);
	decoded += '-';
	decoded.append(&decodedBuffer[6], 4);
	decoded += '-';
	decoded.append(&decodedBuffer[10], 4);
	decoded += '-';
	decoded.append(&decodedBuffer[14], 4);
	decoded += '-';
	decoded.append(&decodedBuffer[18]);

	return decoded;
}

std::string cocos2d_uuid::EncodeUuid(std::string_view src)
{
	if (src.size() < 36)return {};

	char encodedBuffer[32 + 1]{};
	constexpr const size_t nDecodedSize = sizeof(encodedBuffer) - 1;
	char* pWritten = encodedBuffer;
	for (size_t nRead = 0; (pWritten != &encodedBuffer[nDecodedSize]) && nRead < 36; ++nRead)
	{
		if (src[nRead] == '-')continue;
		*pWritten++ = src[nRead];
	}

	std::string encoded;
	encoded.resize(22);
	memcpy(&encoded[0], &src[0], 2);
	pWritten = &encoded[2];
	for (size_t i = 2; i < nDecodedSize; i += 3)
	{
		size_t l = g_hexDigits.find(encodedBuffer[i]);
		size_t m = g_hexDigits.find(encodedBuffer[i + 1]);
		size_t r = g_hexDigits.find(encodedBuffer[i + 2]);
		if (l == std::string_view::npos || m == std::string_view::npos || r == std::string_view::npos)return {};

		*pWritten++ = g_base64[(l << 2) | (m >> 2)];
		*pWritten++ = g_base64[((m & 3) << 4) | r];
	}

	return encoded;
}

int cocos2d_uuid::DecodeUuidInBuffer(std::string_view src, char* dst)
{
	if (src.size() < kEncodedLength)return 0;

	char decodedBuffer[30 + 1]{};
	char* pBufferWritten = decodedBuffer;

	for (size_t i = 2; i < kEncodedLength; i += 2)
	{
		size_t l = g_base64.find(src[i]);
		size_t r = g_base64.find(src[i + 1]);
		if (l == std::string_view::npos || r == std::string_view::npos)return {};

		*pBufferWritten++ = g_hexDigits[l >> 2];
		*pBufferWritten++ = g_hexDigits[((l & 3) << 2) | (r >> 4)];
		*pBufferWritten++ = g_hexDigits[r & 0xF];
	}

	char* pDstWritten = dst;

	*pDstWritten++ = src[0];
	*pDstWritten++ = src[1];

	memcpy(pDstWritten, &decodedBuffer[0], 6); pDstWritten += 6;
	*pDstWritten++ = '-';

	memcpy(pDstWritten, &decodedBuffer[6], 4); pDstWritten += 4;
	*pDstWritten++ = '-';

	memcpy(pDstWritten, &decodedBuffer[10], 4); pDstWritten += 4;
	*pDstWritten++ = '-';

	memcpy(pDstWritten, &decodedBuffer[14], 4); pDstWritten += 4;
	*pDstWritten++ = '-';

	memcpy(pDstWritten, &decodedBuffer[18], 12); pDstWritten += 12;

	return static_cast<int>(pDstWritten - dst);
}

int cocos2d_uuid::EncodeUuidInBuffer(std::string_view src, char* dst)
{
	if (src.size() < kDecodedLength)return 0;

	char encodedBuffer[32 + 1]{};
	constexpr size_t decodedBufferSize = sizeof(encodedBuffer) - 1;
	char* pBufferWritten = encodedBuffer;

	for (size_t nRead = 0; nRead < kDecodedLength && pBufferWritten != &encodedBuffer[decodedBufferSize]; ++nRead)
	{
		if (src[nRead] == '-')continue;
		*pBufferWritten++ = src[nRead];
	}

	char* pDstWritten = dst;

	*pDstWritten++ = encodedBuffer[0];
	*pDstWritten++ = encodedBuffer[1];

	for (size_t i = 2; i < decodedBufferSize; i += 3)
	{
		size_t l = g_hexDigits.find(encodedBuffer[i]);
		size_t m = g_hexDigits.find(encodedBuffer[i + 1]);
		size_t r = g_hexDigits.find(encodedBuffer[i + 2]);
		if (l == std::string_view::npos || m == std::string_view::npos || r == std::string_view::npos)return {};

		*pDstWritten++ = g_base64[(l << 2) | (m >> 2)];
		*pDstWritten++ = g_base64[((m & 3) << 4) | r];
	}

	return static_cast<int>(pDstWritten - dst);
}
