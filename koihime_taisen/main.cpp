
#include <algorithm>

#include "file_utility.h"
#include "static_string.h"
#include "cocos2d_config.h"
#include "cocos2d_uuid.h"

/* Avoid string literal for the benefit of IP. */
static constexpr const char g_rawHostDir[] =
{
		0x68, 0x74, 0x74, 0x70, 0x73, 0x3A, 0x2F, 0x2F, 0x70, 0x72, 0x6F, 0x64, 0x2D, 0x63, 0x64, 0x6E,
		0x2D, 0x64, 0x6D, 0x6D, 0x2E, 0x6B, 0x6F, 0x69, 0x68, 0x69, 0x6D, 0x65, 0x74, 0x61, 0x69, 0x73,
		0x65, 0x6E, 0x2E, 0x63, 0x6F, 0x6D, 0x2F, 0x62, 0x72, 0x6F, 0x77, 0x73, 0x65, 0x72, 0x2F, 0x00,
};
static constexpr std::string_view g_hostDir = std::string_view(g_rawHostDir, sizeof(g_rawHostDir) - 1);

static std::string_view ExtractString(std::string_view data, std::string_view start, const char end)
{
	size_t nPos1 = data.find(start);
	if (nPos1 == std::string_view::npos)return {};
	nPos1 += start.length();

	size_t nPos2 = data.find(end, nPos1);
	if (nPos2 == std::string_view::npos)return {};

	return data.substr(nPos1, nPos2 - nPos1);
}

static void UnquoteInPlace(std::string_view& s)
{
	size_t nPos1 = s.find_first_of("\"'");
	if (nPos1 == std::string_view::npos)return;

	const char quoteEnd = s[nPos1];
	++nPos1;

	size_t nPos2 = s.find(quoteEnd, nPos1);
	if (nPos2 == std::string_view::npos)return;

	s.remove_prefix(nPos1);
	s.remove_suffix(s.length() - (nPos2 - nPos1));
}

static void FlattenDirectoryInPlace(std::string_view& s)
{
	if (s.starts_with("./"))
	{
		s.remove_prefix(2);
	}
}

static std::string LoadSettingJson()
{
	/*
	* Trace the following files.
	*
	* 1. (HostDir)/index.html
	* => System.import('./index.XXXXX.js')
	*
	* 2. (HostDir)/index.XXXXX.js
	* => System.register(["./application.XXXXX.js"]
	*
	* 3. (HostDir)/application.XXXXX.js
	* => this.settingsPath = 'src/settings.XXXXX.json';
	*/

	StaticString512 urlBuffer;

	urlBuffer.append(g_hostDir).append("index.html");
	std::string indexHtml = file_utility::LoadInternetResourceAsString(urlBuffer.data());
	if (indexHtml.empty())return {};

	const auto CreateUrl = [&urlBuffer](std::string_view& s)
		-> void
		{
			UnquoteInPlace(s);
			FlattenDirectoryInPlace(s);
			urlBuffer.shrink(g_hostDir.length());
			urlBuffer.append(s);
		};

	std::string_view indexJsName = ExtractString(indexHtml, "System.import(", ')');
	if (indexJsName.empty())return {};
	CreateUrl(indexJsName);

	std::string indexJs = file_utility::LoadInternetResourceAsString(urlBuffer.data());
	if (indexJs.empty())return {};
	std::string_view applicationJsName = ExtractString(indexJs, "System.register([", ']');
	if (applicationJsName.empty())return {};
	CreateUrl(applicationJsName);

	std::string applicationJs = file_utility::LoadInternetResourceAsString(urlBuffer.data());
	if (applicationJs.empty())return {};
	std::string_view settingJsonName = ExtractString(applicationJs, "this.settingsPath", ';');
	if (settingJsonName.empty())return {};
	CreateUrl(settingJsonName);

	return file_utility::LoadInternetResourceAsString(urlBuffer.data());
}

static const std::string_view BundleNameToAssetDir(const cocos_config::BundleVer& bundleVer)
{
	return bundleVer.name == "remoteAssets" ? "remote/" : "assets/";
}

static std::string LoadConfigJson(const cocos_config::BundleVer& bundleVer)
{
	const std::string_view assetDir = BundleNameToAssetDir(bundleVer);

	StaticString512 configUrlBuffer;
	configUrlBuffer.append(g_hostDir).append(assetDir).append(bundleVer.name).append("/config.").append(bundleVer.version).append(".json");

#ifdef _DEBUG
	file_utility::SaveInternetResourceToFileCreatingNestedFolder(configUrlBuffer.string_view());
#endif

	return file_utility::LoadInternetResourceAsString(configUrlBuffer.data());
}

static const std::string_view CreateDirectoryForBundle(const cocos_config::BundleVer& bundleVer)
{
	static char s_DirectoryPath[512]{};
	size_t nWritten = 0;
	StaticString512 directoryNameBuffer;
	directoryNameBuffer.append("browser/").append(BundleNameToAssetDir(bundleVer)).append(bundleVer.name);
	file_utility::CreateDirectoryInBuffer(directoryNameBuffer.string_view(), s_DirectoryPath, sizeof(s_DirectoryPath) - 1, nWritten);

	return { s_DirectoryPath , nWritten };
}

static std::string_view UuidIndexToString(size_t index)
{
	static char s_buffer[64]{};
	int length = sprintf(s_buffer, "%zu", index);

	return { s_buffer, static_cast<size_t>(length) };
}

static StaticString512 CreateAssetsFileUrl(const cocos_config::BundleVer& bundleVer, std::string_view uuid, std::string_view version, bool isNative)
{
	const std::string_view assetDir = BundleNameToAssetDir(bundleVer);

	StaticString512 urlBuffer;
	urlBuffer.append(g_hostDir).append(assetDir).append(bundleVer.name).push_back('/');
	urlBuffer.append(isNative ? "native/" : "import/");

	char decodedBuffer[cocos2d_uuid::kDecodedLength + 1]{};
	cocos2d_uuid::DecodeUuidInBuffer(uuid, decodedBuffer);
	urlBuffer.append(&decodedBuffer[0], 2).append("/").append(decodedBuffer, cocos2d_uuid::kDecodedLength)
		.append(".").append(version);

	return urlBuffer;
}

static StaticString512 CreatePackJsonUrl(const cocos_config::BundleVer& bundleVer, std::string_view packName, std::string_view version)
{
	const std::string_view assetDir = BundleNameToAssetDir(bundleVer);

	StaticString512 urlBuffer;
	urlBuffer.append(g_hostDir).append(assetDir).append(bundleVer.name)
		.append("/import/").append(&packName[0], 2).append("/").append(packName).append(version).append(".json");

	return urlBuffer;
}

static bool IsForAdv(std::string_view path)
{
	static constexpr const std::string_view filters[] =
	{
		"adv/scenario",
		"adv/spine",
		"adv/stillAnim",
		"characters",
		"sound/se",
		"sound/voice/adv"
	};

	return std::ranges::any_of(filters, [&path](const std::string_view& filter)
		{
			return path.starts_with(filter);
		});
}

static void DownloadBundleResources(const cocos_config::BundleVer& bundleVer)
{
	std::string configJson = LoadConfigJson(bundleVer);
	cocos_config::ResourceInfo r;
	cocos_config::ParseConfigJson(configJson, r);
	if (r.uuids.empty())return;

	const std::string_view folderPath = CreateDirectoryForBundle(bundleVer);

	for (size_t i = 5; i < r.uuids.size(); ++i)
	{
		const auto& uuid = r.uuids[i];
		const auto& path = r.paths.find(i);
		if (path == r.paths.end())continue;
#if 1
		if (!IsForAdv(path->second.path))continue;
#endif
		if (cocos_config::IsMesh(path->second, r.types))continue;
		const uint8_t typeIndex = path->second.fileTypes[0];

		std::string_view extension = cocos_config::GuessExtension(typeIndex, r.types);
		std::string_view index = UuidIndexToString(i);

		if (cocos_config::IsImportSignificant(typeIndex, r.types))
		{
			const auto& iterImport = r.importVersion.find(index);
			if (iterImport != r.importVersion.cend())
			{
				StaticString512 url = CreateAssetsFileUrl(bundleVer, uuid, iterImport->second, false);
				url.append(".json");

				StaticString512 filePath;
				filePath.append(path->second.path).append(".json");

				file_utility::SaveInternetResourceToFileCreatingNestedFolder(url.string_view(), filePath.string_view(), folderPath);
			}

			continue;
		}
		else if (cocos_config::IsImageAsset(typeIndex, r.types))
		{
			const auto& iterImport = r.importVersion.find(index);
			if (iterImport != r.importVersion.cend())
			{
				StaticString512 importUrl = CreateAssetsFileUrl(bundleVer, uuid, iterImport->second, false);
				importUrl.append(".json");

				std::string importFile = file_utility::LoadInternetResourceAsString(importUrl.data());
				if (!importFile.empty())
				{
					extension = cocos_config::FindImageAssetExtension(importFile);
				}
			}
		}
		const auto& iterNative = r.nativeVersion.find(index);
		if (iterNative != r.nativeVersion.cend())
		{
			StaticString512 url = CreateAssetsFileUrl(bundleVer, uuid, iterNative->second, true);
			url.append(extension);

			StaticString512 filePath;
			filePath.append(path->second.path).append(extension);

			bool bRet = file_utility::SaveInternetResourceToFileCreatingNestedFolder(url.string_view(), filePath.string_view(), folderPath);
#ifdef _DEBUG
			if (!bRet)
			{
				/* If failed, find extension from import file. */
				const auto& iterImport = r.importVersion.find(index);
				if (iterImport != r.importVersion.cend())
				{
					StaticString512 importUrl = CreateAssetsFileUrl(bundleVer, uuid, iterImport->second, false);
					importUrl.append(".json");
				}
			}
#endif
		}
	}
}

static void DownloadBundles()
{
	std::string settingJson = LoadSettingJson();
	if (settingJson.empty())return;

	std::vector<cocos_config::BundleVer> bundleVers;
	cocos_config::FindBundleVers(settingJson, bundleVers);

	for (const auto& bundleVer : bundleVers)
	{
		/* Not interested in other bundles */
		if (bundleVer.name != "remoteAssets")continue;
		DownloadBundleResources(bundleVer);
	}
}

int main(int argc, char* argv[])
{
	DownloadBundles();
}