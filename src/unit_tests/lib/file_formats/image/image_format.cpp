/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015-2016 Martin Ejdestig <marejde@gmail.com>
 *
 * Rayni is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Rayni is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Rayni. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <vector>

#include "lib/file_formats/image/image_format.h"
#include "lib/system/scoped_temp_dir.h"

namespace Rayni
{
	class ImageFormatTest : public testing::Test
	{
	protected:
		ImageFormat::Type create_and_determine_type(const std::string &file_name,
		                                            const std::vector<std::uint8_t> &data)
		{
			std::experimental::filesystem::path path = temp_dir.path() / file_name;
			write_data_to_file(path, data);
			return ImageFormat::determine_type_from_file(path);
		}

		static std::vector<std::uint8_t> exr_magic()
		{
			return {0x76, 0x2f, 0x31, 0x01};
		}

		static std::vector<std::uint8_t> jpeg_magic()
		{
			return {0xff, 0xd8, 0xff};
		}

		static std::vector<std::uint8_t> png_magic()
		{
			return {0x89, 'P', 'N', 'G'};
		}

		static std::vector<std::uint8_t> webp_magic()
		{
			return {'R', 'I', 'F', 'F', 0xfe, 0xdc, 0xba, 0x98, 'W', 'E', 'B', 'P'}; // 4-7 irrelevant
		}

	private:
		static void write_data_to_file(const std::string &path, const std::vector<std::uint8_t> &data)
		{
			std::ofstream file;
			file.open(path, std::ios_base::binary | std::ios_base::out);
			std::copy(data.begin(), data.end(), std::ostream_iterator<std::uint8_t>(file));
			if (!file.good())
				FAIL() << "Failed to write to " << path;
		}

		ScopedTempDir temp_dir;
	};

	TEST_F(ImageFormatTest, Magic)
	{
		EXPECT_EQ(ImageFormat::Type::EXR, create_and_determine_type("exr_magic.bin", exr_magic()));
		EXPECT_EQ(ImageFormat::Type::JPEG, create_and_determine_type("jpeg_magic.bin", jpeg_magic()));
		EXPECT_EQ(ImageFormat::Type::PNG, create_and_determine_type("png_magic.bin", png_magic()));
		EXPECT_EQ(ImageFormat::Type::WEBP, create_and_determine_type("wepb_magic.bin", webp_magic()));
	}

	TEST_F(ImageFormatTest, ShortMagic)
	{
		auto shorten = [](auto magic) {
			magic.pop_back();
			return magic;
		};

		EXPECT_EQ(ImageFormat::Type::UNDETERMINED,
		          create_and_determine_type("exr_short_magic.bin", shorten(exr_magic())));
		EXPECT_EQ(ImageFormat::Type::UNDETERMINED,
		          create_and_determine_type("jpeg_short_magic.bin", shorten(jpeg_magic())));
		EXPECT_EQ(ImageFormat::Type::UNDETERMINED,
		          create_and_determine_type("png_short_magic.bin", shorten(png_magic())));
		EXPECT_EQ(ImageFormat::Type::UNDETERMINED,
		          create_and_determine_type("wepb_short_magic.bin", shorten(webp_magic())));

		EXPECT_EQ(ImageFormat::Type::UNDETERMINED, create_and_determine_type("empty_magic.bin", {}));
	}

	TEST_F(ImageFormatTest, CorruptMagic)
	{
		auto corrupt = [](auto magic) {
			magic.back() ^= 0x10;
			return magic;
		};

		EXPECT_EQ(ImageFormat::Type::UNDETERMINED,
		          create_and_determine_type("exr_corrupt_magic.bin", corrupt(exr_magic())));

		EXPECT_EQ(ImageFormat::Type::UNDETERMINED,
		          create_and_determine_type("jpeg_corrupt_magic.bin", corrupt(jpeg_magic())));

		EXPECT_EQ(ImageFormat::Type::UNDETERMINED,
		          create_and_determine_type("png_corrupt_magic.bin", corrupt(png_magic())));

		EXPECT_EQ(ImageFormat::Type::UNDETERMINED,
		          create_and_determine_type("wepb_corrupt_magic.bin", corrupt(webp_magic())));
	}

	TEST_F(ImageFormatTest, ExtensionOnly)
	{
		EXPECT_EQ(ImageFormat::Type::EXR, create_and_determine_type("extension_only.exr", {}));

		EXPECT_EQ(ImageFormat::Type::JPEG, create_and_determine_type("extension_only.jpg", {}));
		EXPECT_EQ(ImageFormat::Type::JPEG, create_and_determine_type("extension_only.jpeg", {}));
		EXPECT_EQ(ImageFormat::Type::JPEG, create_and_determine_type("extension_only.jpe", {}));

		EXPECT_EQ(ImageFormat::Type::PNG, create_and_determine_type("extension_only.png", {}));

		EXPECT_EQ(ImageFormat::Type::TGA, create_and_determine_type("extension_only.icb", {}));
		EXPECT_EQ(ImageFormat::Type::TGA, create_and_determine_type("extension_only.targa", {}));
		EXPECT_EQ(ImageFormat::Type::TGA, create_and_determine_type("extension_only.tga", {}));
		EXPECT_EQ(ImageFormat::Type::TGA, create_and_determine_type("extension_pnly.tpic", {}));
		EXPECT_EQ(ImageFormat::Type::TGA, create_and_determine_type("extension_only.vda", {}));
		EXPECT_EQ(ImageFormat::Type::TGA, create_and_determine_type("extension_only.vst", {}));

		EXPECT_EQ(ImageFormat::Type::WEBP, create_and_determine_type("extension_only.webp", {}));

		EXPECT_EQ(ImageFormat::Type::JPEG, create_and_determine_type("extension_only_mixed_case.jPeG", {}));
		EXPECT_EQ(ImageFormat::Type::PNG, create_and_determine_type("extension_only_mixed_case.PnG", {}));
	}

	TEST_F(ImageFormatTest, WrongExtension)
	{
		EXPECT_EQ(ImageFormat::Type::EXR, create_and_determine_type("exr_wrong_extension.tga", exr_magic()));
		EXPECT_EQ(ImageFormat::Type::JPEG, create_and_determine_type("jpeg_wrong_extension.tga", jpeg_magic()));
		EXPECT_EQ(ImageFormat::Type::PNG, create_and_determine_type("png_wrong_extension.tga", png_magic()));
		EXPECT_EQ(ImageFormat::Type::WEBP, create_and_determine_type("wepb_wrong_extension.tga", webp_magic()));
	}
}
