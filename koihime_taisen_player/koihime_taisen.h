#ifndef KOIHIME_TAISEN_H_
#define KOIHIME_TAISEN_H_

#include <string>
#include <vector>

#include "adv.h"

namespace koihime_taisen
{
	/// @brief 台本読み取り
	bool ReadScenario(
		const std::string& scenarioFilePath,
		std::vector<adv::TextDatum>& textData,
		std::vector<adv::ImageDatum>& imageData,
		std::vector<adv::CutInDatum>& cutInData,
		std::vector<adv::SceneDatum>& sceneData,
		std::vector<adv::LabelDatum>& labelData
	);
}

#endif // !KOIHIME_TAISEN_H_
