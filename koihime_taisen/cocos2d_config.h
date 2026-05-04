#ifndef COCOS2D_CONFIG_H_
#define COCOS2D_CONFIG_H_

#include <string>
#include <vector>
#include <map>

namespace cocos_config
{
	struct BundleVer
	{
		std::string_view name;
		std::string_view version;
	};

	/*
	* Note that some developpers specify bundle name with launched version on setting json,
	* and associate the latest version on "start.XXXXX.js".
	*/

	void FindBundleVers(const std::string_view& settingJson, std::vector<BundleVer> &bundleVers);

	struct PathDatum
	{
		std::string_view path;
		std::vector<uint8_t> fileTypes;
	};

	struct PackDatum
	{
		std::string_view packName;
		std::vector<size_t> indices;
	};

	using version_map = std::map<std::string_view, std::string_view>;
	using type_map = std::map<uint8_t, std::string_view>;

	struct ResourceInfo
	{
		std::vector<std::string> uuids;
		std::map<size_t, PathDatum> paths;
		std::vector<PackDatum> packs;
		std::map<std::string_view, std::string_view> importVersion;
		std::map<std::string_view, std::string_view> nativeVersion;
		std::map<uint8_t, std::string_view> types;
	};

	void ParseConfigJson(const std::string_view& configJson, ResourceInfo& r);

	long long SearchPackIndices(size_t nIndex, const std::vector<PackDatum>& packs);

	/*
	* Actually, extension can be obtained from corresponding import file,
	* But it would be better to check whether the file is already downloaded or not by deduction
	* so as to avoid downloading already downloaded one.
	*/

	std::string_view GuessExtension(uint8_t type, const std::map<uint8_t, std::string_view>& types);

	/* Some types, like cc.textAsset, exist only as import file. */

	bool IsImportSignificant(uint8_t typeIndex, const std::map<uint8_t, std::string_view>& types);
	bool IsMesh(const PathDatum& path, const std::map<uint8_t, std::string_view>& types);
	bool IsTexture2D(uint8_t typeIndex, const std::map<uint8_t, std::string_view>& types);

	/* Extension/format of image file is specified by index in import file. */

	bool IsImageAsset(uint8_t typeIndex, const std::map<uint8_t, std::string_view>& types);
	std::string_view FindImageAssetExtension(const std::string_view& importJson);
}

#endif // COCOS2D_CONFIG_H_