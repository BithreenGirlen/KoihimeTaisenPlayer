
#include <array>
#include <map>

#include "koihime_taisen.h"

#include "../koihime_taisen/json_minimal.h"
#include "text_utility.h"
#include "path_utility.h"
#include "sdl_filesystem_utility.h"
#include "sdl_string_utility.h"

/* 内部用 */
namespace koihime_taisen
{
	/*
	* Usually, naming convention for enum class is PascalCase,
	* but here camelCase is adopted so as to conform to that of command string in script.
	*/
	enum class ScriptCommandType : uint8_t
	{
		unknown = static_cast<uint8_t>(-1U),
		soundLoad = 1,
		def,
		mesDisplay,
		animLoad,
		spineLoad,
		spineButton,
		set,
		fade,
		play,
		overlayIn,
		playAutoVo,
		fv,
		highlight,
		audioFadeOut,
		overlayOut,
		animLoadWait,
		setMotion,
		spineDisp,
		wait,
		camPos,
		camZoom,
		camReset,
		timeScale,
		removeAt
	};

	/// @brief 指令文名称と列挙型対応
	struct CommandEntry
	{
		std::string_view name;
		ScriptCommandType type;
	};

	/// @brief 指令文一単位
	struct Command
	{
		ScriptCommandType type = ScriptCommandType::unknown;
		std::vector<std::string_view> params;
	};

	/// @brief 指令文の名称から列挙型への写像
	static constexpr CommandEntry g_commandMap[] =
	{
		{"@soundLoad", ScriptCommandType::soundLoad},
		{"@def", ScriptCommandType::def},
		{"@mesDisplay", ScriptCommandType::mesDisplay},
		{"@animLoad", ScriptCommandType::animLoad},
		{"@@spineLoad", ScriptCommandType::spineLoad},
		{"@spineButton", ScriptCommandType::spineButton},
		{"@set", ScriptCommandType::set},
		{"@@fade", ScriptCommandType::fade},
		{"@play", ScriptCommandType::play},
		{"@@overlayIn", ScriptCommandType::overlayIn},
		{"@playAutoVo", ScriptCommandType::playAutoVo},
		{"@fv", ScriptCommandType::fv},
		{"@highlight", ScriptCommandType::highlight},
		{"@audioFadeOut", ScriptCommandType::audioFadeOut},
		{"@@overlayOut", ScriptCommandType::overlayOut},
		{"@animLoadWait", ScriptCommandType::animLoadWait},
		{"@setMotion", ScriptCommandType::setMotion},
		{"@spineDisp", ScriptCommandType::spineDisp},
		{"@wait", ScriptCommandType::wait},
		{"@camPos", ScriptCommandType::camPos},
		{"@camZoom", ScriptCommandType::camZoom},
		{"@camReset", ScriptCommandType::camReset},
		{"@timeScale", ScriptCommandType::timeScale},
		{"@removeAt", ScriptCommandType::removeAt},
	};

	/// @brief 指令文名称を元に列挙型探索
	static ScriptCommandType CommandNameToType(const std::string_view& commandName)
	{
		const auto& iter = std::find_if(std::begin(g_commandMap), std::end(g_commandMap),
			[&commandName](const CommandEntry& commandEntry)
			{
				return commandEntry.name == commandName;
			});

		if (iter != std::cend(g_commandMap))
		{
			return iter->type;
		}

		return ScriptCommandType::unknown;
	}

	/// @brief 註釋か否か
	template<typename CharType>
	static bool IsComment(std::basic_string_view<CharType> line)
	{
		if constexpr (std::is_same_v<CharType, wchar_t>)
		{
			return line.starts_with(LR"(//)") || line.starts_with(L'#') || line.starts_with((L"　"));
		}
		else if constexpr (std::is_same_v<CharType, char>)
		{
			/* 全角空白 {0xE3, 0x80, 0x80 }も註釋と看做す */
			return line.starts_with(R"(//)") || line.starts_with('#') || line.starts_with(reinterpret_cast<const char*>(u8"　"));
		}
	}

	/// @brief 指令文か否か
	template<typename CharType>
	static bool IsCommand(std::basic_string_view<CharType> line)
	{
		if constexpr (std::is_same_v<CharType, wchar_t>) return line.starts_with(L'@');
		else if constexpr (std::is_same_v<CharType, char>) return line.starts_with('@');
	}

	/// @brief 指令文解析
	template<typename CharType>
	static Command ParseCommand(std::basic_string_view<CharType> line)
	{
		Command commandDatum;
		const auto EmplaceBack = [&line, &commandDatum](size_t nStart, size_t nEnd)
			-> void
			{
				std::basic_string_view<CharType> s = line.substr(nStart, nEnd - nStart);
				if (nStart == 0)
				{
					commandDatum.type = CommandNameToType(s);
				}
				else
				{
					commandDatum.params.push_back(std::move(s));
				}
			};
		for (size_t nRead = 0;;)
		{
			size_t nPos = line.find(static_cast<CharType>(' '), nRead);
			if (nPos == std::basic_string_view<CharType>::npos)
			{
				EmplaceBack(nRead, line.length());
				break;
			}

			EmplaceBack(nRead, nPos);
			nRead = nPos + 1;
		}

		return commandDatum;
	}

	/// @brief 台本ファイル経路から基底階層導出
	template<typename CharType>
	static std::basic_string_view<CharType> DeriveBaseFolderPathFromScriptFilePath(const std::basic_string<CharType>& scriptFilePath)
	{
		static constexpr CharType key[] =
		{
			static_cast<CharType>('a'),
			static_cast<CharType>('d'),
			static_cast<CharType>('v'),
			static_cast<CharType>('\0')
		};
		size_t nPos = scriptFilePath.rfind(key);
		if (nPos == std::basic_string_view<CharType>::npos)return {};

		return std::basic_string_view<CharType>(scriptFilePath.data(), nPos);
	}

	/// @brief cc.TextAssetからデータ抽出
	static std::string ExtractDataFromCocosTextAsset(const std::string& textAsset)
	{
		if (textAsset.empty())return {};

		static constexpr std::array<size_t, 3> indices = { 5, 0, 2 };
		const char* p = &textAsset[0];
		const char* pStart = nullptr, * pEnd = nullptr;
		bool bRet = json_minimal::FindArrayValueByIndices(p, indices.data(), indices.size(), &pStart, &pEnd);
		if (!bRet)return {};

		if (*pStart == '"' && *(pEnd - 1) == '"')
		{
			++pStart;
			--pEnd;
		}

		std::string scenarioScript(pStart, pEnd);
		size_t unescapedLength = text_utility::UnescapeInPlace(scenarioScript.data());
		scenarioScript.resize(unescapedLength);

		return scenarioScript;
	}

	/// @brief cc.TextAssetファイル読み込みとデータ抽出
	template<typename CharType>
	static std::basic_string<CharType> LoadCocosTextAsset(const std::basic_string<CharType>& filePath)
	{
		if constexpr (std::is_same_v<CharType, wchar_t>)
		{
			std::string utf8FilePath = sdl_string_utility::NarrowToUtf8(filePath);
			std::string textAsset = sdl_filesystem_utility::LoadFileAsString(utf8FilePath.data());
			std::string scenarioScript = ExtractDataFromCocosTextAsset(textAsset);

			return sdl_string_utility::WidenFromUtf8(scenarioScript);
		}
		else if constexpr (std::is_same_v<CharType, char>)
		{
			std::string textAsset = sdl_filesystem_utility::LoadFileAsString(filePath.data());

			return ExtractDataFromCocosTextAsset(textAsset);
		}
	}

	/// @brief 置換文字列探索
	template<typename CharType>
	static std::basic_string_view<CharType> FindTag(std::basic_string_view<CharType> param)
	{
		if constexpr (std::is_same_v<CharType, wchar_t>)
		{
			size_t nPos1 = param.find(L"$[");
			if (nPos1 == std::wstring_view::npos)return {};
			nPos1 += 2;

			size_t nPos2 = param.find(L']', nPos1);
			if (nPos2 == std::wstring_view::npos)return {};

			return param.substr(nPos1, nPos2 - nPos1);
		}
		else if constexpr (std::is_same_v<CharType, char>)
		{
			size_t nPos1 = param.find("$[");
			if (nPos1 == std::string_view::npos)return {};
			nPos1 += 2;

			size_t nPos2 = param.find(']', nPos1);
			if (nPos2 == std::string_view::npos)return {};

			return param.substr(nPos1, nPos2 - nPos1);
		}
	}

	/// @brief 文字列の8バイト整数への変換
	template<typename CharType>
	static uint64_t StrToUInt64(std::basic_string_view<CharType> s)
	{
		if constexpr (std::is_same_v<CharType, wchar_t>)
		{
			/* The length of uint64_t is 20 characters at most. */
			wchar_t uint64Buffer[32]{};
			constexpr size_t bufferLength = sizeof(uint64Buffer) / sizeof(wchar_t) - 1;
			if (s.length() > bufferLength)return static_cast<uint64_t>(-1LL);

			::memcpy(uint64Buffer, s.data(), s.length() * sizeof(wchar_t));
			uint64Buffer[s.length()] = L'\0';

			return ::wcstoull(uint64Buffer, nullptr, 10);
		}
		else if constexpr (std::is_same_v<CharType, char>)
		{
			char uint64Buffer[32]{};
			constexpr size_t bufferLength = sizeof(uint64Buffer) - 1;
			if (s.length() > bufferLength)return static_cast<uint64_t>(-1LL);

			::memcpy(uint64Buffer, s.data(), s.length());
			uint64Buffer[s.length()] = '\0';

			return ::strtoull(uint64Buffer, nullptr, 10);
		}
	}

	/// @brief 文字列の単精度浮動小数点数への変換
	template<typename CharType>
	static float StrToFloat(std::basic_string_view<CharType> s)
	{
		if constexpr (std::is_same_v<CharType, wchar_t>)
		{
			wchar_t floatBuffer[16]{};
			constexpr size_t bufferLength = sizeof(floatBuffer) / sizeof(wchar_t) - 1;
			if (s.length() > bufferLength) return 1.f;

			::memcpy(floatBuffer, s.data(), s.length() * sizeof(wchar_t));
			floatBuffer[s.length()] = L'\0';

			return ::wcstof(floatBuffer, nullptr);
		}
		else if constexpr (std::is_same_v<CharType, char>)
		{
			char floatBuffer[16]{};
			constexpr size_t bufferLength = sizeof(floatBuffer) - 1;
			if (s.length() > bufferLength) return 1.f;

			::memcpy(floatBuffer, s.data(), s.length());
			floatBuffer[s.length()] = '\0';

			return ::strtof(floatBuffer, nullptr);
		}
	}

	/// @brief 連番画像ファイル経路作成
	/// @param baseFolderPath 基底階層
	/// @param stillFolderName 連番画像格納フォルダ名
	/// @param startNumber 開始番号
	/// @param endNumber 終了番号
	/// @return 全画像ファイル経路
	template<typename CharType>
	static std::vector<std::basic_string<CharType>> CreateAnimationFilePaths(
		std::basic_string_view<CharType> baseFolderPath,
		std::basic_string_view<CharType> stillFolderName,
		std::basic_string_view<CharType> startNumber,
		std::basic_string_view<CharType> endNumber)
	{
		uint64_t nStartIndex = StrToUInt64(startNumber);
		uint64_t nEndIndex = StrToUInt64(endNumber);
		if (nStartIndex == static_cast<uint64_t>(-1LL) || nEndIndex == static_cast<uint64_t>(-1LL) || nEndIndex < nStartIndex) return {};
		std::vector<std::basic_string<CharType>> filePaths;

		for (uint64_t index = nStartIndex; index <= nEndIndex; ++index)
		{
			if constexpr (std::is_same_v<CharType, wchar_t>)
			{
				wchar_t buffer[512]{};
				swprintf_s(buffer, LR"(%.*s\adv\stillAnim\%.*s\image_%05llu.webp)",
					static_cast<int>(baseFolderPath.length()), baseFolderPath.data(),
					static_cast<int>(stillFolderName.length()), stillFolderName.data(),
					index
				);

				filePaths.emplace_back(buffer);
			}
			else if constexpr (std::is_same_v<CharType, char>)
			{
				char buffer[512]{};
				sprintf_s(buffer, R"(%.*s/adv/stillAnim/%.*s/image_%05llu.webp)",
					static_cast<int>(baseFolderPath.length()), baseFolderPath.data(),
					static_cast<int>(stillFolderName.length()), stillFolderName.data(),
					index
				);

				filePaths.emplace_back(buffer);
			}
		}

		return filePaths;
	}

	/// @brief 音声ファイル経路供給器
	template<typename CharType>
	class CVoiceFilePathReservoir
	{
	public:
		CVoiceFilePathReservoir() = default;
		~CVoiceFilePathReservoir() = default;

		/// @brief 音声ファイル一覧作成
		/// @param baseFolderPath 基底階層
		/// @param folderName 基底階層からみた相対経路
		void setup(std::basic_string_view<CharType> baseFolderPath, std::basic_string_view<CharType> folderName)
		{
			if constexpr (std::is_same_v<CharType, wchar_t>)
			{
				clear();

				wchar_t buffer[512]{};
				int bufferLength = swprintf_s(buffer, L"%.*s%.*s",
					static_cast<int>(baseFolderPath.length()), baseFolderPath.data(),
					static_cast<int>(folderName.length()), folderName.data()
				);
				if (bufferLength == -1)return;

				/* Actually, this is not necessary because L'/' character does not cuase problem; just for readability in debugging. */
#ifdef _WIN32
				std::replace(buffer, buffer + bufferLength, L'/', L'\\');
#endif
				std::wstring_view folderPath(buffer, bufferLength);
				if (folderPath.back() == L'\\' || folderPath.back() == L'/')
				{
					folderPath.remove_suffix(1);
				}

				m_voiceFilePaths = sdl_filesystem_utility::CreateFilePaths(sdl_string_utility::NarrowToUtf8(folderPath), "*.mp3", sdl_filesystem_utility::EPathType::File);
			}
			else if constexpr (std::is_same_v<CharType, char>)
			{
				clear();

				char buffer[512]{};
				int bufferLength = sprintf_s(buffer, "%.*s%.*s",
					static_cast<int>(baseFolderPath.length()), baseFolderPath.data(),
					static_cast<int>(folderName.length()), folderName.data()
				);
				if (bufferLength == -1)return;

				std::string_view folderPath(buffer, bufferLength);
				if (folderPath.back() == '/')
				{
					folderPath.remove_suffix(1);
				}

				m_voiceFilePaths = sdl_filesystem_utility::CreateFilePaths(folderPath, "*.mp3", sdl_filesystem_utility::EPathType::File);
			}
		}

		/// @brief 予約
		void reserve()
		{
			if (m_voiceFilePathIndex < m_voiceFilePaths.size())
			{
				m_pReservedFilePath = &m_voiceFilePaths[m_voiceFilePathIndex];
				++m_voiceFilePathIndex;
			}
		}
		/// @brief 予約済みか
		bool hasReservation() const
		{
			return m_pReservedFilePath != nullptr;
		}
		/// @brief 受け取り
		[[nodiscard]] std::basic_string<CharType>* getReserved()
		{
			std::basic_string<CharType>* pToServe = m_pReservedFilePath;
			m_pReservedFilePath = nullptr;

			return pToServe;
		}
	private:
		std::vector<std::basic_string<CharType>> m_voiceFilePaths;
		size_t m_voiceFilePathIndex = 0;
		std::basic_string<CharType>* m_pReservedFilePath = nullptr;

		void clear()
		{
			m_voiceFilePaths.clear();
			m_voiceFilePathIndex = 0;
			m_pReservedFilePath = nullptr;
		}
	};

	/// @brief 固定長バッファ上での連番画像ファイル経路管理
	class CAnimationImageDatumDeque
	{
	public:
		CAnimationImageDatumDeque() = default;
		~CAnimationImageDatumDeque() = default;

		void push_back(adv::ImageDatum& imageDatum)
		{
			if (m_nCount >= kCapacity) return;

			uint8_t nNextIndex = (m_nStartIndex + m_nCount) % kCapacity;
			m_animationImageData[nNextIndex] = std::move(imageDatum);
			++m_nCount;
		}

		void push_front(adv::ImageDatum& imageDatum)
		{
			if (m_nCount >= kCapacity) return;

			m_nStartIndex = (m_nStartIndex + kCapacity - 1) % kCapacity;
			m_animationImageData[m_nStartIndex] = std::move(imageDatum);
			++m_nCount;
		}

		[[nodiscard]] adv::ImageDatum pop_back()
		{
			if (m_nCount == 0)return {};

			uint8_t nBackIndex = (m_nStartIndex + m_nCount - 1) % kCapacity;
			--m_nCount;

			return std::move(m_animationImageData[nBackIndex]);
		}

		[[nodiscard]] adv::ImageDatum pop_front()
		{
			if (m_nCount == 0)return {};

			uint8_t nFrontIndex = m_nStartIndex;
			++m_nStartIndex;
			if (m_nStartIndex >= kCapacity)m_nStartIndex = 0;
			--m_nCount;

			return std::move(m_animationImageData[nFrontIndex]);
		}

	private:
		/* 変更される可能性は極めて低いので態々<deque>をインクルードしたくない。 */
		enum AnimationIndex : uint8_t
		{
			Animation0 = 0,
			Animation1,
			kMax
		};
		static constexpr uint8_t kCapacity = static_cast<uint8_t>(AnimationIndex::kMax);
		adv::ImageDatum m_animationImageData[kCapacity];

		uint8_t m_nStartIndex = 0;
		uint8_t m_nCount = 0;
	};

} /* namespace koihime_taisen */

bool koihime_taisen::ReadScenario(
	const std::string& scenarioFilePath,
	std::vector<adv::TextDatum>& textData,
	std::vector<adv::ImageDatum>& imageData,
	std::vector<adv::CutInDatum>& cutInData,
	std::vector<adv::SceneDatum>& sceneData,
	std::vector<adv::LabelDatum>& labelData)
{
	std::string_view baseFolderPath = DeriveBaseFolderPathFromScriptFilePath(scenarioFilePath);
	if (baseFolderPath.empty())return false;

	std::string scenarioScript = LoadCocosTextAsset(scenarioFilePath);
	if (scenarioScript.empty())return false;

	std::vector<std::string_view> lines;
	text_utility::TextToLines(scenarioScript, lines);

	/* 文章単位で切り分けるための各種バッファ */
	std::map<std::string_view, std::string_view> tagMap; /* タグ名写像 */
	CVoiceFilePathReservoir<char> voiceFilePathResevoir; /* 全音声ファイル経路 */
	CAnimationImageDatumDeque animationImageDatumDeque; /* 連番画像ファイル経路 */
	std::vector<std::string> spineFilePaths; /* Spineファイル経路 */
	std::string* lastLabel = nullptr; /* 静画切り替わり場面での静画名称 */
	bool toShowCutinInTheScene = false; /* カットイン有無 */

	for (auto& line : lines)
	{
		size_t nPos = line.find_first_not_of("\r\n\t");
		if (nPos != std::string_view::npos)
		{
			line.remove_prefix(nPos);
		}
		if (line.empty())continue;
		else if (IsComment(line))continue;
		else if (IsCommand(line))
		{
			Command command = ParseCommand(line);
			if (command.type == ScriptCommandType::unknown)continue;

			switch (command.type)
			{
			case ScriptCommandType::def:
				if (command.params.size() > 1)
				{
					if (command.params[0] == "SVP")
					{
						/* 音声ファイル探索 */
						voiceFilePathResevoir.setup(baseFolderPath, command.params[1]);
					}
					else
					{
						/* タグ登録 */
						tagMap.insert({ command.params[0], command.params[1] });
					}
				}
				break;
			case ScriptCommandType::animLoad:
				/* 連番ファイル読み込み */
				if (command.params.size() > 3)
				{
					/*
					* param[0]: 識別名
					* param[1]: ファイル名書式
					* param[2]: 開始番号
					* param[3]: 終了番号
					*/
					std::string_view key = FindTag(command.params[1]);
					const auto& entry = tagMap.find(key);
					if (entry != tagMap.cend())
					{
						adv::ImageDatum imageDatum
						{
							.name = std::string("animation").append(command.params[0]),
							.filePaths = CreateAnimationFilePaths(baseFolderPath, entry->second, command.params[2], command.params[3])
						};

						animationImageDatumDeque.push_back(imageDatum);
					}
				}
				break;
			case ScriptCommandType::spineLoad:
				/* Spineファイル経路指定 */
				if (!command.params.empty())
				{
					std::string spineFilePath = std::string(baseFolderPath).append(command.params[0]);
#ifdef _WIN32
					std::replace(spineFilePath.begin(), spineFilePath.end(), '/', '\\');
#endif
					spineFilePaths.emplace_back(std::move(spineFilePath));
				}
				break;
			case ScriptCommandType::set:
				/* 背景・一枚絵指定 */
				if (command.params.size() > 2)
				{
					const auto& path = command.params[2];
					/* 一枚絵 */
					if (command.params[0] == "bg" && path.find("stillAnim") != std::wstring_view::npos)
					{
						/*
						* param[0]: 描画先
						* param[1]: 描画順
						* param[2]: ファイル名
						*/
						std::string filePath = std::string(baseFolderPath).append(path).append(".webp");
#ifdef _WIN32
						std::replace(filePath.begin(), filePath.end(), '/', '\\');
#endif
						std::vector<std::string> filePaths;
						filePaths.push_back(std::move(filePath));
						adv::ImageDatum imageDatum
						{
							.name = std::string(path_utility::TruncateFilePath(path)),
							.filePaths = std::move(filePaths)
						};

						imageData.push_back(std::move(imageDatum));
						lastLabel = &imageData.back().name;
					}
				}
				break;
			case ScriptCommandType::playAutoVo:
				/* 音声ファイル取り出し */
				voiceFilePathResevoir.reserve();
				break;
			case ScriptCommandType::setMotion:
				/* パラパラ漫画・もしくはSpineによるアニメーション処理 */
				if (command.params.size() > 3)
				{
					/*
					* param[0]: Spineか連番画像か
					* param[1]: 識別名
					* param[2]: 画像: 不使用, Spine: 動作名
					* param[3]: 画像: 表示・消去, Spine: ループ有無
					*/
					if (command.params[0] == "anim" && command.params[3] == "1")
					{
						imageData.push_back(animationImageDatumDeque.pop_front());
						lastLabel = &imageData.back().name;
					}
					else if (command.params[0] == "spine")
					{
						/*
						* 連番画像は配列番号0と1を使って分けて読み込んでいるのだが、
						* Spineには配列番号がなく、0番目に読み込んだものを削除した上で新規読み込みしている。
						*/
						if (!spineFilePaths.empty())
						{
							adv::CutInDatum cutInDatum
							{
								.filePath = spineFilePaths.back(),
								.animationName = std::string(command.params[2].data(), static_cast<int>(command.params[2].length())),
								.loop = command.params[3] == "1"
							};

							cutInData.push_back(std::move(cutInDatum));
						}
					}
				}
				break;
			case ScriptCommandType::spineDisp:
				/* Spine表示・非表示切り替え */
				if (!command.params.empty())
				{
					toShowCutinInTheScene = command.params[0] == "1";
				}
				break;
			case ScriptCommandType::timeScale:
				/* Spine再生速度指定 */
				if (command.params.size() > 2)
				{
					if (!cutInData.empty())
					{
						cutInData.back().timeScale = StrToFloat(command.params[2]);
					}
				}
				break;
			default:
				break;
			}
		}
		else
		{
			/* 発話者・文章 */
			adv::TextDatum textDatum
			{
				.message = std::string(line),
				.voicePath = voiceFilePathResevoir.hasReservation() ? *voiceFilePathResevoir.getReserved() : std::string{}
			};
			text_utility::EliminateTagInPlace(textDatum.message);
			textData.push_back(std::move(textDatum));

			adv::SceneDatum sceneDatum
			{
				.nTextIndex = textData.size() - 1,
				.nImageIndex = imageData.empty() ? 0 : imageData.size() - 1,
				.hasCutIn = toShowCutinInTheScene,
				.nCutInIndex = cutInData.empty() ? 0 : cutInData.size() - 1,
			};
			sceneData.push_back(std::move(sceneDatum));

			if (lastLabel != nullptr)
			{
				adv::LabelDatum labelDatum
				{
					.caption = *lastLabel,
					.nSceneIndex = sceneData.size() - 1
				};
				labelData.push_back(std::move(labelDatum));

				lastLabel = nullptr;
			}
		}
	}

	return true;
}