

#include "sdl_string_utility.h"

#include <SDL3/SDL_stdinc.h>

std::string sdl_string_utility::NarrowToUtf8(std::wstring_view wstr)
{
	const char* pData = reinterpret_cast<const char*>(wstr.data());
	size_t byteLength = (wstr.length() + 1) * sizeof(wchar_t);
	char* pStr = ::SDL_iconv_string("UTF-8", "WCHAR_T", pData, byteLength);
	if (pStr == nullptr)return {};

	std::string str(pStr);
	::SDL_free(pStr);

	return str;
}

std::wstring sdl_string_utility::WidenFromUtf8(std::string_view str)
{
	const char* pData = str.data();
	size_t byteLength = str.length() + 1;
	char* pStr = ::SDL_iconv_string("WCHAR_T", "UTF-8", pData, byteLength);
	if (pStr == nullptr)return {};

	std::wstring wstr(reinterpret_cast<const wchar_t*>(pStr));
	::SDL_free(pStr);

	return wstr;
}

std::string sdl_string_utility::Utf16ToUtf8(std::u16string_view u16str)
{
	const char* pData = reinterpret_cast<const char*>(u16str.data());
	size_t byteLength = (u16str.length() + 1) * sizeof(char16_t);
	char* pStr = ::SDL_iconv_string("UTF-8", "UTF-16LE", pData, byteLength);
	if (pStr == nullptr)return {};

	std::string str(pStr);
	::SDL_free(pStr);

	return str;
}

std::u16string sdl_string_utility::Utf8ToUtf16(std::string_view str)
{
	const char* pData = str.data();
	size_t byteLength = str.length() + 1;
	char* pStr = ::SDL_iconv_string("UTF-16LE", "UTF-8", pData, byteLength);
	if (pStr == nullptr)return {};

	std::u16string u16str(reinterpret_cast<const char16_t*>(pStr));
	::SDL_free(pStr);

	return u16str;
}

std::string sdl_string_utility::Utf32ToUtf8(std::u32string_view u32str)
{
	const char* pData = reinterpret_cast<const char*>(u32str.data());
	size_t byteLength = (u32str.length() + 1) * sizeof(char32_t);
	char* pStr = ::SDL_iconv_string("UTF-8", "UTF-32LE", pData, byteLength);
	if (pStr == nullptr)return {};

	std::string str(pStr);
	::SDL_free(pStr);

	return str;
}

std::u32string sdl_string_utility::Utf8ToUtf32(std::string_view str)
{
	const char* pData = str.data();
	size_t byteLength = str.length() + 1;
	char* pStr = ::SDL_iconv_string("UTF-32LE", "UTF-8", pData, byteLength);
	if (pStr == nullptr)return {};

	std::u32string u32str(reinterpret_cast<const char32_t*>(pStr));
	::SDL_free(pStr);

	return u32str;
}
