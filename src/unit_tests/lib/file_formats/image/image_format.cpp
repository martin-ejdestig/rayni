// This file is part of Rayni.
//
// Copyright (C) 2015-2020 Martin Ejdestig <marejde@gmail.com>
//
// Rayni is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Rayni is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Rayni. If not, see <http://www.gnu.org/licenses/>.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "lib/file_formats/image/image_format.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "lib/io/file.h"
#include "lib/system/scoped_temp_dir.h"

namespace Rayni
{
	namespace
	{
		std::vector<std::uint8_t> exr_magic()
		{
			return {0x76, 0x2f, 0x31, 0x01};
		}

		std::vector<std::uint8_t> jpeg_magic()
		{
			return {0xff, 0xd8, 0xff};
		}

		std::vector<std::uint8_t> png_magic()
		{
			return {0x89, 'P', 'N', 'G'};
		}

		std::vector<std::uint8_t> webp_magic()
		{
			return {'R', 'I', 'F', 'F', 0xfe, 0xdc, 0xba, 0x98, 'W', 'E', 'B', 'P'}; // 4-7 irrelevant
		}
	}

	TEST(ImageFormat, Magic)
	{
		ScopedTempDir temp_dir;

		ASSERT_TRUE(file_write(temp_dir.path() / "exr_magic.bin", exr_magic()));
		EXPECT_EQ(ImageFormat::EXR, image_format_from_file(temp_dir.path() / "exr_magic.bin"));

		ASSERT_TRUE(file_write(temp_dir.path() / "jpeg_magic.bin", jpeg_magic()));
		EXPECT_EQ(ImageFormat::JPEG, image_format_from_file(temp_dir.path() / "jpeg_magic.bin"));

		ASSERT_TRUE(file_write(temp_dir.path() / "png_magic.bin", png_magic()));
		EXPECT_EQ(ImageFormat::PNG, image_format_from_file(temp_dir.path() / "png_magic.bin"));

		ASSERT_TRUE(file_write(temp_dir.path() / "webp_magic.bin", webp_magic()));
		EXPECT_EQ(ImageFormat::WEBP, image_format_from_file(temp_dir.path() / "webp_magic.bin"));
	}

	TEST(ImageFormat, ShortMagic)
	{
		ScopedTempDir temp_dir;

		auto shorten = [](auto magic) {
			magic.pop_back();
			return magic;
		};

		ASSERT_TRUE(file_write(temp_dir.path() / "exr_short_magic.bin", shorten(exr_magic())));
		EXPECT_EQ(ImageFormat::UNDETERMINED, image_format_from_file(temp_dir.path() / "exr_short_magic.bin"));

		ASSERT_TRUE(file_write(temp_dir.path() / "jpeg_short_magic.bin", shorten(jpeg_magic())));
		EXPECT_EQ(ImageFormat::UNDETERMINED, image_format_from_file(temp_dir.path() / "jpeg_short_magic.bin"));

		ASSERT_TRUE(file_write(temp_dir.path() / "png_short_magic.bin", shorten(png_magic())));
		EXPECT_EQ(ImageFormat::UNDETERMINED, image_format_from_file(temp_dir.path() / "png_short_magic.bin"));

		ASSERT_TRUE(file_write(temp_dir.path() / "webp_short_magic.bin", shorten(webp_magic())));
		EXPECT_EQ(ImageFormat::UNDETERMINED, image_format_from_file(temp_dir.path() / "webp_short_magic.bin"));

		ASSERT_TRUE(file_write(temp_dir.path() / "empty_short_magic.bin", {}));
		EXPECT_EQ(ImageFormat::UNDETERMINED, image_format_from_file(temp_dir.path() / "empty_magic.bin"));
	}

	TEST(ImageFormat, CorruptMagic)
	{
		ScopedTempDir temp_dir;

		auto corrupt = [](auto magic) {
			magic.back() ^= 0x10;
			return magic;
		};

		ASSERT_TRUE(file_write(temp_dir.path() / "exr_corrupt_magic.bin", corrupt(exr_magic())));
		EXPECT_EQ(ImageFormat::UNDETERMINED, image_format_from_file(temp_dir.path() / "exr_corrupt_magic.bin"));

		ASSERT_TRUE(file_write(temp_dir.path() / "jpeg_corrupt_magic.bin", corrupt(jpeg_magic())));
		EXPECT_EQ(ImageFormat::UNDETERMINED,
		          image_format_from_file(temp_dir.path() / "jpeg_corrupt_magic.bin"));

		ASSERT_TRUE(file_write(temp_dir.path() / "png_corrupt_magic.bin", corrupt(png_magic())));
		EXPECT_EQ(ImageFormat::UNDETERMINED, image_format_from_file(temp_dir.path() / "png_corrupt_magic.bin"));

		ASSERT_TRUE(file_write(temp_dir.path() / "webp_corrupt_magic.bin", corrupt(webp_magic())));
		EXPECT_EQ(ImageFormat::UNDETERMINED,
		          image_format_from_file(temp_dir.path() / "webp_corrupt_magic.bin"));
	}

	TEST(ImageFormat, ExtensionOnly)
	{
		ScopedTempDir temp_dir;

		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only.exr", {}));
		EXPECT_EQ(ImageFormat::EXR, image_format_from_file(temp_dir.path() / "extension_only.exr"));

		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only.jpg", {}));
		EXPECT_EQ(ImageFormat::JPEG, image_format_from_file(temp_dir.path() / "extension_only.jpg"));
		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only.jpeg", {}));
		EXPECT_EQ(ImageFormat::JPEG, image_format_from_file(temp_dir.path() / "extension_only.jpeg"));
		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only.jpe", {}));
		EXPECT_EQ(ImageFormat::JPEG, image_format_from_file(temp_dir.path() / "extension_only.jpe"));

		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only.png", {}));
		EXPECT_EQ(ImageFormat::PNG, image_format_from_file(temp_dir.path() / "extension_only.png"));

		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only.icb", {}));
		EXPECT_EQ(ImageFormat::TGA, image_format_from_file(temp_dir.path() / "extension_only.icb"));
		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only.targa", {}));
		EXPECT_EQ(ImageFormat::TGA, image_format_from_file(temp_dir.path() / "extension_only.targa"));
		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only.tga", {}));
		EXPECT_EQ(ImageFormat::TGA, image_format_from_file(temp_dir.path() / "extension_only.tga"));
		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only.tpic", {}));
		EXPECT_EQ(ImageFormat::TGA, image_format_from_file(temp_dir.path() / "extension_only.tpic"));
		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only.vda", {}));
		EXPECT_EQ(ImageFormat::TGA, image_format_from_file(temp_dir.path() / "extension_only.vda"));
		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only.vst", {}));
		EXPECT_EQ(ImageFormat::TGA, image_format_from_file(temp_dir.path() / "extension_only.vst"));

		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only.webp", {}));
		EXPECT_EQ(ImageFormat::WEBP, image_format_from_file(temp_dir.path() / "extension_only.webp"));

		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only_mixed_case.jPeG", {}));
		EXPECT_EQ(ImageFormat::JPEG,
		          image_format_from_file(temp_dir.path() / "extension_only_mixed_case.jPeG"));

		ASSERT_TRUE(file_write(temp_dir.path() / "extension_only_mixed_case.PnG", {}));
		EXPECT_EQ(ImageFormat::PNG, image_format_from_file(temp_dir.path() / "extension_only_mixed_case.PnG"));
	}

	TEST(ImageFormat, WrongExtension)
	{
		ScopedTempDir temp_dir;

		ASSERT_TRUE(file_write(temp_dir.path() / "exr_wrong_extension.tga", exr_magic()));
		EXPECT_EQ(ImageFormat::EXR, image_format_from_file(temp_dir.path() / "exr_wrong_extension.tga"));

		ASSERT_TRUE(file_write(temp_dir.path() / "jpeg_wrong_extension.tga", jpeg_magic()));
		EXPECT_EQ(ImageFormat::JPEG, image_format_from_file(temp_dir.path() / "jpeg_wrong_extension.tga"));

		ASSERT_TRUE(file_write(temp_dir.path() / "png_wrong_extension.tga", png_magic()));
		EXPECT_EQ(ImageFormat::PNG, image_format_from_file(temp_dir.path() / "png_wrong_extension.tga"));

		ASSERT_TRUE(file_write(temp_dir.path() / "wepb_wrong_extension.tga", webp_magic()));
		EXPECT_EQ(ImageFormat::WEBP, image_format_from_file(temp_dir.path() / "wepb_wrong_extension.tga"));
	}
}
