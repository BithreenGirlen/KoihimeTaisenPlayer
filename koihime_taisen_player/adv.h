#ifndef ADV_H_
#define ADV_H_

#include <string>

namespace adv
{
	struct TextDatum
	{
		std::string message;
		std::string voicePath;
	};

	struct ImageDatum
	{
		std::string name;
		std::vector<std::string> filePaths;
	};

	struct CutInDatum
	{
		std::string filePath;
		std::string animationName;

		bool loop = true;
		float timeScale = 1.f;
	};

	struct SceneDatum
	{
		size_t nTextIndex = 0;
		size_t nImageIndex = 0;

		/* Not inclined to include <optional> */
		bool hasCutIn = false;
		size_t nCutInIndex = 0;
	};

	struct LabelDatum
	{
		std::string caption;
		size_t nSceneIndex = 0;
	};
}

#endif // !ADV_H_
