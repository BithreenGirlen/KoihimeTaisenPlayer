#ifndef SETUP_H_
#define SETUP_H_

#include <string_view>

namespace setup
{
	static constexpr uint32_t kDefaultMessageFontSize = 32;
	static constexpr uint32_t kDefaultThickness = 4;
	static constexpr uint32_t kDefaultUiFontSize = 20;

	struct FontDatum
	{
		/// @brief Reference to static buffer/length
		std::string_view filePath;
		bool bold = true;
		bool italic = true;

		/// @brief Font size for text message
		uint32_t messageFontSize = kDefaultMessageFontSize;
		uint32_t thickness = kDefaultThickness;

		/// @brief Font size for UI
		uint32_t uiFontSize = kDefaultUiFontSize;
	};

	/// @brief Read setting file.
	bool Initialise();

	/// @brief Get reference to static variable.
	const FontDatum& GetFontDatum();
}
#endif // !SETUP_H_
