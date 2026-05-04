

#include "setup.h"
#include "sdl_filesystem_utility.h"
#include "../koihime_taisen/json_minimal.h"

namespace setup
{
	static constexpr size_t kMaxPathLength = 1024;

	static char g_fontFilePath[kMaxPathLength];
	static size_t g_fontFilePathLength = 0;
	static FontDatum g_fontDatum;

#ifdef _WIN32
	static void ApplyDefaultSetting()
	{
		static constexpr char defaultFontFilePath[] = "C:\\Windows\\Fonts\\yumin.ttf";
		static constexpr size_t defaultFontFilePathLength = sizeof(defaultFontFilePath) - 1;

		memcpy(&g_fontFilePath[0], defaultFontFilePath, defaultFontFilePathLength);
		g_fontFilePath[defaultFontFilePathLength] = '\0';
		g_fontFilePathLength = defaultFontFilePathLength;

		g_fontDatum.filePath = std::string_view(g_fontFilePath, g_fontFilePathLength);
		g_fontDatum.bold = true;
		g_fontDatum.italic = true;
		g_fontDatum.messageFontSize = kDefaultMessageFontSize;
		g_fontDatum.thickness = kDefaultThickness;
		g_fontDatum.uiFontSize = kDefaultThickness;
	}
#endif

	static bool HandleLoadingFailure()
	{
#ifdef _WIN32
		ApplyDefaultSetting();
		return true;
#else
		return false;
#endif
	}

	static bool UpdateFontFilePath(const char* pStart, const char* pEnd)
	{
		size_t nLength = pEnd - pStart;
		if (nLength >= kMaxPathLength)return false;

		memcpy(&g_fontFilePath[0], pStart, nLength);
		g_fontFilePath[nLength] = '\0';
		g_fontFilePathLength = nLength;

		g_fontDatum.filePath = std::string_view(g_fontFilePath, g_fontFilePathLength);

		return true;
	}

	static bool IsTrue(const char* pStart, const char* pEnd)
	{
		static constexpr std::string_view s_true = "true";
		if (pEnd - pStart < s_true.length())return false;

		return ::memcmp(pStart, s_true.data(), s_true.length()) == 0;
	}

	static uint32_t StrToUint32(const char* pStart, const char* pEnd)
	{
		size_t nLength = pEnd - pStart;

		/* The length of uint32_t is 10 characters at most. */
		char uint32Buffer[16]{};
		constexpr size_t bufferLength = sizeof(uint32Buffer) - 1;
		if (nLength > bufferLength)return static_cast<uint32_t>(-1LL);

		::memcpy(uint32Buffer, pStart, nLength);
		uint32Buffer[nLength] = '\0';

		return ::strtoul(uint32Buffer, nullptr, 10);
	}

	static bool ParseSettting(const std::string& settingFile, FontDatum& fontDatum)
	{
		const char* p = &settingFile[0];
		const char* pStart = nullptr, * pEnd = nullptr;
		bool bRet = json_minimal::FindNextObject(&p, "font", &pStart, &pEnd);
		if (!bRet)return false;

		p = pStart;
		bRet = json_minimal::FindValueByName(p, "filePath", &pStart, &pEnd);
		if (bRet)
		{
			/* This will overwrite the static buffer which should be restored when failed to parse. */
			if (!UpdateFontFilePath(pStart, pEnd))
			{
				return false;
			}
			fontDatum.filePath = std::string_view(g_fontFilePath, g_fontFilePathLength);
		}

		bRet &= json_minimal::FindValueByName(p, "bold", &pStart, &pEnd);
		if (bRet)
		{
			fontDatum.bold = IsTrue(pStart, pEnd);
		}

		bRet &= json_minimal::FindValueByName(p, "italic", &pStart, &pEnd);
		if (bRet)
		{
			fontDatum.italic = IsTrue(pStart, pEnd);
		}

		bRet &= json_minimal::FindValueByName(p, "messageFontSize", &pStart, &pEnd);
		if (bRet)
		{
			fontDatum.messageFontSize = StrToUint32(pStart, pEnd);
		}

		bRet &= json_minimal::FindValueByName(p, "thickness", &pStart, &pEnd);
		if (bRet)
		{
			fontDatum.thickness = StrToUint32(pStart, pEnd);
		}

		bRet &= json_minimal::FindValueByName(p, "uiFontSize", &pStart, &pEnd);
		if (bRet)
		{
			fontDatum.uiFontSize = StrToUint32(pStart, pEnd);
		}

		return bRet;
	}
}

bool setup::Initialise()
{
	std::string settingFile = sdl_filesystem_utility::LoadFileAsString("setting.txt");
	if (settingFile.empty())
	{
		return HandleLoadingFailure();
	}

	FontDatum fontDatum;
	if (!ParseSettting(settingFile, fontDatum))
	{
		return HandleLoadingFailure();
	}

	g_fontDatum = std::move(fontDatum);

	return true;
}

const setup::FontDatum& setup::GetFontDatum()
{
	return g_fontDatum;
}
