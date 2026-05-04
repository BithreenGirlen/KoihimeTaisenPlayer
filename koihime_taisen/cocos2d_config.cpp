

#include "json_minimal.h"
#include "cocos2d_config.h"

namespace cocos_config
{
	static uint32_t StrToUint32(const char* src, size_t length)
	{
		char sBuffer[32]{};
		if (length > sizeof(sBuffer) - 1)return 0;
		memcpy(sBuffer, src, length);
		sBuffer[length] = '\0';

		return strtol(sBuffer, nullptr, 10);
	}

	static void ReadPath(const char* src, std::map<size_t, PathDatum>& pathMap)
	{
		const char* p = src;
		const char* pStart = nullptr, * pEnd = nullptr;
		bool bRet = json_minimal::FindNextObject(&src, "paths", &pStart, &pEnd);
		if (!bRet)return;

		p = pStart;
		for (;;)
		{
			PathDatum path;

			const char* pNameStart = nullptr, * pNameEnd = nullptr;
			const char* pValueStart = nullptr, * pValueEnd = nullptr;
			bRet = json_minimal::util::ReadNextKeyInObject(&p, &pNameStart, &pNameEnd, &pValueStart, &pValueEnd);
			if (!bRet)break;

			uint32_t index = StrToUint32(pNameStart, pNameEnd - pNameStart);

			const char* p2 = pValueStart;
			for (size_t i = 0;; ++i)
			{
				bRet = json_minimal::util::ReadNextValueInArray(&p2, &pValueStart, &pValueEnd);
				if (!bRet)break;

				if (i == 0)
				{
					path.path = std::string_view(pValueStart, pValueEnd - pValueStart);
				}
				else
				{
					uint32_t type = StrToUint32(pValueStart, pValueEnd - pValueStart);
					path.fileTypes.push_back(static_cast<uint8_t>(type));
				}
			}

			pathMap.insert({ index, std::move(path) });
		}
	}

	static void ReadPack(const char* src, std::vector<PackDatum>& packs)
	{
		const char* p = src;
		const char* pStart = nullptr, * pEnd = nullptr;
		bool bRet = json_minimal::FindNextObject(&src, "packs", &pStart, &pEnd);
		if (!bRet)return;

		p = pStart;
		for (;;)
		{
			PackDatum pack;

			const char* pNameStart = nullptr, * pNameEnd = nullptr;
			const char* pValueStart = nullptr, * pValueEnd = nullptr;
			bRet = json_minimal::util::ReadNextKeyInObject(&p, &pNameStart, &pNameEnd, &pValueStart, &pValueEnd);
			if (!bRet)break;

			pack.packName = std::string_view(pNameStart, pNameEnd - pNameStart);

			const char* p2 = pValueStart;
			for (;;)
			{
				bRet = json_minimal::util::ReadNextValueInArray(&p2, &pValueStart, &pValueEnd);
				if (!bRet)break;

				int iType = StrToUint32(pValueStart, pValueEnd - pValueStart);
				pack.indices.push_back(iType);
			}

			packs.push_back(std::move(pack));
		}
	}

	static void ReadVersion(const char* src, std::map<std::string_view, std::string_view>& versionMap, bool bNative)
	{
		const char* p = src;
		const char* pStart = nullptr, * pEnd = nullptr;
		bool bRet = json_minimal::FindNextObject(&src, "versions", &pStart, &pEnd);
		if (!bRet)return;

		p = pStart;
		const char* key = bNative ? "native" : "import";
		bRet = json_minimal::FindNextArray(&p, key, &pStart, &pEnd);
		if (!bRet)return;

		p = pStart;
		std::vector<std::string_view> temps;
		for (;;)
		{
			const char* pValueStart = nullptr, * pValueEnd = nullptr;
			bRet = json_minimal::util::ReadNextValueInArray(&p, &pValueStart, &pValueEnd);
			if (!bRet)break;

			temps.emplace_back(pValueStart, pValueEnd - pValueStart);
		}

		for (size_t i = 0; i < temps.size(); i += 2)
		{
			versionMap.insert({ std::move(temps[i]), std::move(temps[i + 1]) });
		}
	}

	static void ReadUuid(const char* src, std::vector<std::string>& uuids)
	{
		const char* p = src;
		const char* pStart = nullptr, * pEnd = nullptr;
		bool bRet = json_minimal::FindNextArray(&p, "uuids", &pStart, &pEnd);
		if (!bRet)return;

		p = pStart;
		for (;;)
		{
			const char* pValueStart = nullptr, * pValueEnd = nullptr;
			bool bRet = json_minimal::util::ReadNextValueInArray(&p, &pValueStart, &pValueEnd);
			if (!bRet)break;

			uuids.emplace_back(pValueStart, pValueEnd);
		}
	}

	static void ReadTypes(const char* src, std::map<uint8_t, std::string_view>& types)
	{
		const char* p = src;
		const char* pStart = nullptr, * pEnd = nullptr;
		bool bRet = json_minimal::FindNextObject(&src, "types", &pStart, &pEnd);
		if (!bRet)return;

		p = pStart;
		for (uint8_t i = 0;; ++i)
		{
			const char* pValueStart = nullptr, * pValueEnd = nullptr;
			bRet = json_minimal::util::ReadNextValueInArray(&p, &pValueStart, &pValueEnd);
			if (!bRet)break;

			types.insert({ i, std::string_view(pValueStart, pValueEnd - pValueStart) });
		}
	}

	static std::string_view FindImageAssetFormat(const std::string_view& importJson)
	{
		const char* p = &importJson[0];
		const char* pStart = nullptr, * pEnd = nullptr;

		bool bRet = json_minimal::FindValueByName(p, "fmt", &pStart, &pEnd);
		if (!bRet)return {};

		return std::string_view(pStart, pEnd - pStart);
	}
}

void cocos_config::FindBundleVers(const std::string_view& settingJson, std::vector<BundleVer>& bundleVers)
{
	const char* p = &settingJson[0];
	const char* pStart = nullptr, * pEnd = nullptr;
	bool bRet = json_minimal::FindNextObject(&p, "bundleVers", &pStart, &pEnd);
	if (!bRet)return;

	p = pStart;
	for (;;)
	{
		const char* pKeyStart = nullptr, * pKeyEnd = nullptr;
		const char* pValueStart = nullptr, * pValueEnd = nullptr;
		bRet = json_minimal::util::ReadNextKeyInObject(&p, &pKeyStart, &pKeyEnd, &pValueStart, &pValueEnd);
		if (!bRet)break;

		BundleVer s
		{
			.name = std::string_view(pKeyStart, pKeyEnd - pKeyStart),
			.version = std::string_view(pValueStart, pValueEnd - pValueStart)
		};
		bundleVers.push_back(std::move(s));
	}
}

void cocos_config::ParseConfigJson(const std::string_view& configJson, ResourceInfo& r)
{
	ReadUuid(configJson.data(), r.uuids);
	ReadPath(configJson.data(), r.paths);
	ReadVersion(configJson.data(), r.importVersion, false);
	ReadVersion(configJson.data(), r.nativeVersion, true);
	ReadPack(configJson.data(), r.packs);
	ReadTypes(configJson.data(), r.types);
}

long long cocos_config::SearchPackIndices(size_t nIndex, const std::vector<PackDatum>& packs)
{
	for (size_t i = 0; i < packs.size(); ++i)
	{
		const auto& pack = packs[i];
		const auto& iter = std::find(pack.indices.cbegin(), pack.indices.cend(), nIndex);
		if (iter != pack.indices.cend())
		{
			return i;
		}
	}

	return -1;
}

std::string_view cocos_config::GuessExtension(uint8_t type, const std::map<uint8_t, std::string_view>& types)
{
	static const std::map<std::string_view, std::string_view> extensionMap =
	{
		{"cc.Asset", ".atlas"},
		{"cc.EffectAsset", ".effect"},
		{"cc.BitmapFont", ".fnt"},
		{"cc.SpriteFrame", ".png"},
		{"cc.JsonAsset", ".json"},
		{"cc.AnimationClip", ".anim"},
		{"cc.ImageAsset", ".webp"},
		{"cc.Texture2D", ".webp"},
		{"cc.TextAsset", ".json"},
		{"cc.VideoClip", ".mp4"},
		{"sp.SkeletonData", ".bin"},
		{"cc.Prefab, ", ".prefab"},
		{"cc.ParticleAsset", ".plist"},
		{"cc.Material", ".mtl"},
		{"cc.Mesh", ""},
		{"cc.AudioClip", ".mp3"},
		{"cc.RenderTexture", ""},
		{"cc.SpriteAtlas", ".atlas"},
		{"cc.Skeleton", ".skel"},
		{"cc.TiledMapAsset", ""},
		{"cc.TTFFont", ".ttf"},
	};

	const auto& iter = types.find(type);
	if (iter != types.cend())
	{
		const auto& iter2 = extensionMap.find(iter->second);
		if (iter2 != extensionMap.cend())
		{
			return iter2->second;
		}
	}

	return {};
}

bool cocos_config::IsImportSignificant(uint8_t typeIndex, const std::map<uint8_t, std::string_view>& types)
{
	const auto& iter = types.find(typeIndex);
	if (iter != types.cend())
	{
		return iter->second == "cc.TextAsset";
	}

	return false;
}

bool cocos_config::IsMesh(const PathDatum& path, const std::map<uint8_t, std::string_view>& types)
{
	if (path.path.find("fbx") != std::string_view::npos)return true;
	else
	{
		const auto& iter = types.find(path.fileTypes[0]);
		if (iter != types.cend())
		{
			return iter->second == "cc.Mesh";
		}
	}

	return false;
}

bool cocos_config::IsTexture2D(uint8_t typeIndex, const std::map<uint8_t, std::string_view>& types)
{
	const auto& iter = types.find(typeIndex);
	if (iter != types.cend())
	{
		return iter->second == "cc.Texture2D";
	}

	return false;
}

bool cocos_config::IsImageAsset(uint8_t typeIndex, const std::map<uint8_t, std::string_view>& types)
{
	const auto& iter = types.find(typeIndex);
	if (iter != types.cend())
	{
		return iter->second == "cc.ImageAsset";
	}

	return false;
}

std::string_view cocos_config::FindImageAssetExtension(const std::string_view& importJson)
{
	std::string_view format = FindImageAssetFormat(importJson);
	if (format.size() != 1)return{};

	/* https://github.com/cocos/cocos-engine/blob/v3.8.8/cocos/asset/assets/image-asset.ts#L631 */
	static constexpr std::string_view extensions[] = { ".png", ".jpg", ".jpeg", ".bmp", ".webp", ".pvr", "pkm", ".astc" };
	static constexpr int extensionCount = sizeof(extensions) / sizeof(std::string_view);
	int index = format[0] - '0';
	if (index >= extensionCount)return {};
	
	return extensions[index];
}
