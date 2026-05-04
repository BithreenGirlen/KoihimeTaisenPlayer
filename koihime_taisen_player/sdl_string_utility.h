#ifndef SDL_STRING_UTILITY_H_
#define SDL_STRING_UTILITY_H_

#include <string>

namespace sdl_string_utility
{
	/// @brief OSのwchar_t表現に応じて、wchar_t文字列をUTF8に変換
	[[nodiscard]] std::string NarrowToUtf8(std::wstring_view wstr);

	/// @brief OSのwchar_t表現に応じて、UTF8をwchar_t文字列に変換
	[[nodiscard]] std::wstring WidenFromUtf8(std::string_view str);

	/// @brief UTF16LEからUTF8への変換
	[[nodiscard]] std::string Utf16ToUtf8(std::u16string_view u16str);

	/// @brief UTF8からUTF16LEへの変換
	[[nodiscard]] std::u16string Utf8ToUtf16(std::string_view str);

	/// @brief UTF32LEからUTF8への変換
	[[nodiscard]] std::string Utf32ToUtf8(std::u32string_view u32str);

	/// @brief UTF8からUTF32LEへの変換
	[[nodiscard]] std::u32string Utf8ToUtf32(std::string_view str);
}

#endif // !SDL_STRING_UTILITY_H_
