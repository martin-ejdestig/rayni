// This file is part of Rayni.
//
// Copyright (C) 2015-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/file_formats/image.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "lib/function/result.h"
#include "lib/graphics/image.h"
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

		std::vector<std::uint8_t> exr_data_1x1()
		{
			return {0x76, 0x2f, 0x31, 0x01, 0x02, 0x00, 0x00, 0x00, 0x63, 0x68, 0x61, 0x6e, 0x6e, 0x65,
			        0x6c, 0x73, 0x00, 0x63, 0x68, 0x6c, 0x69, 0x73, 0x74, 0x00, 0x37, 0x00, 0x00, 0x00,
			        0x42, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
			        0x01, 0x00, 0x00, 0x00, 0x47, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x52, 0x00, 0x01, 0x00, 0x00, 0x00,
			        0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x63,
			        0x6f, 0x6d, 0x70, 0x72, 0x65, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x00, 0x63, 0x6f, 0x6d,
			        0x70, 0x72, 0x65, 0x73, 0x73, 0x69, 0x6f, 0x6e, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
			        0x64, 0x61, 0x74, 0x61, 0x57, 0x69, 0x6e, 0x64, 0x6f, 0x77, 0x00, 0x62, 0x6f, 0x78,
			        0x32, 0x69, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x69, 0x73, 0x70, 0x6c,
			        0x61, 0x79, 0x57, 0x69, 0x6e, 0x64, 0x6f, 0x77, 0x00, 0x62, 0x6f, 0x78, 0x32, 0x69,
			        0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x69, 0x6e, 0x65, 0x4f, 0x72, 0x64,
			        0x65, 0x72, 0x00, 0x6c, 0x69, 0x6e, 0x65, 0x4f, 0x72, 0x64, 0x65, 0x72, 0x00, 0x01,
			        0x00, 0x00, 0x00, 0x00, 0x70, 0x69, 0x78, 0x65, 0x6c, 0x41, 0x73, 0x70, 0x65, 0x63,
			        0x74, 0x52, 0x61, 0x74, 0x69, 0x6f, 0x00, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x00, 0x04,
			        0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f, 0x73, 0x63, 0x72, 0x65, 0x65, 0x6e, 0x57,
			        0x69, 0x6e, 0x64, 0x6f, 0x77, 0x43, 0x65, 0x6e, 0x74, 0x65, 0x72, 0x00, 0x76, 0x32,
			        0x66, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			        0x73, 0x63, 0x72, 0x65, 0x65, 0x6e, 0x57, 0x69, 0x6e, 0x64, 0x6f, 0x77, 0x57, 0x69,
			        0x64, 0x74, 0x68, 0x00, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x00, 0x04, 0x00, 0x00, 0x00,
			        0x00, 0x00, 0x80, 0x3f, 0x00, 0x41, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			        0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x3c, 0x00, 0x3c};
		}

		std::vector<std::uint8_t> jpeg_magic()
		{
			return {0xff, 0xd8, 0xff};
		}

		std::vector<std::uint8_t> jpeg_data_1x1()
		{
			return {0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46, 0x00, 0x01, 0x01, 0x02,
			        0x00, 0x1c, 0x00, 0x1c, 0x00, 0x00, 0xff, 0xdb, 0x00, 0x43, 0x00, 0x02, 0x01, 0x01,
			        0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x03,
			        0x02, 0x02, 0x02, 0x02, 0x05, 0x04, 0x04, 0x03, 0x04, 0x06, 0x05, 0x06, 0x06, 0x06,
			        0x05, 0x06, 0x06, 0x06, 0x07, 0x09, 0x08, 0x06, 0x07, 0x09, 0x07, 0x06, 0x06, 0x08,
			        0x0b, 0x08, 0x09, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x06, 0x08, 0x0b, 0x0c, 0x0b, 0x0a,
			        0x0c, 0x09, 0x0a, 0x0a, 0x0a, 0xff, 0xc0, 0x00, 0x0b, 0x08, 0x00, 0x01, 0x00, 0x01,
			        0x01, 0x01, 0x11, 0x00, 0xff, 0xc4, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
			        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0xff, 0xc4,
			        0x00, 0x14, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xda, 0x00, 0x08, 0x01, 0x01, 0x00, 0x00,
			        0x3f, 0x00, 0x7f, 0x1f, 0xff, 0xd9};
		}

		std::vector<std::uint8_t> png_magic()
		{
			return {0x89, 'P', 'N', 'G'};
		}

		std::vector<std::uint8_t> png_data_1x1()
		{
			return {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49,
			        0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x02,
			        0x00, 0x00, 0x00, 0x90, 0x77, 0x53, 0xde, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48,
			        0x59, 0x73, 0x00, 0x00, 0x0b, 0x13, 0x00, 0x00, 0x0b, 0x13, 0x01, 0x00, 0x9a,
			        0x9c, 0x18, 0x00, 0x00, 0x00, 0x0c, 0x49, 0x44, 0x41, 0x54, 0x08, 0xd7, 0x63,
			        0xf8, 0xff, 0xff, 0x3f, 0x00, 0x05, 0xfe, 0x02, 0xfe, 0xdc, 0xcc, 0x59, 0xe7,
			        0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82};
		}

		std::vector<std::uint8_t> tga_data_1x1()
		{
			return {0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			        0x00, 0x01, 0x00, 0x01, 0x00, 0x18, 0x00, 0xff, 0xff, 0xff};
		}

		std::vector<std::uint8_t> webp_magic()
		{
			return {'R', 'I', 'F', 'F', 0xfe, 0xdc, 0xba, 0x98, 'W', 'E', 'B', 'P'}; // 4-7 irrelevant
		}

		std::vector<std::uint8_t> webp_data_1x1()
		{
			return {0x52, 0x49, 0x46, 0x46, 0x24, 0x00, 0x00, 0x00, 0x57, 0x45, 0x42,
			        0x50, 0x56, 0x50, 0x38, 0x20, 0x18, 0x00, 0x00, 0x00, 0x30, 0x01,
			        0x00, 0x9d, 0x01, 0x2a, 0x01, 0x00, 0x01, 0x00, 0x02, 0x00, 0x34,
			        0x25, 0xa4, 0x00, 0x03, 0x70, 0x00, 0xfe, 0xfb, 0x94, 0x00, 0x00};
		}

		void test_read_file(const std::string &suffix, const std::vector<std::uint8_t> &valid_data_1x1)
		{
			ScopedTempDir temp_dir = ScopedTempDir::create().value_or({});
			ASSERT_FALSE(temp_dir.path().empty());
			const std::string read_success_path = temp_dir.path() / ("valid." + suffix);
			const std::string type_determinable_read_fail_path = temp_dir.path() / ("empty." + suffix);

			ASSERT_TRUE(file_write(read_success_path, valid_data_1x1));
			ASSERT_TRUE(file_write(type_determinable_read_fail_path, {}));

			Image image = image_read_file(read_success_path).value_or(Image());
			EXPECT_EQ(1U, image.width()) << suffix;
			EXPECT_EQ(1U, image.height()) << suffix;

			EXPECT_FALSE(image_read_file(type_determinable_read_fail_path)) << suffix;
		}
	}

	TEST(ImageFormat, Magic)
	{
		ScopedTempDir temp_dir = ScopedTempDir::create().value_or({});
		ASSERT_FALSE(temp_dir.path().empty());

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
		ScopedTempDir temp_dir = ScopedTempDir::create().value_or({});
		ASSERT_FALSE(temp_dir.path().empty());

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
		ScopedTempDir temp_dir = ScopedTempDir::create().value_or({});
		ASSERT_FALSE(temp_dir.path().empty());

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
		ScopedTempDir temp_dir = ScopedTempDir::create().value_or({});
		ASSERT_FALSE(temp_dir.path().empty());

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
		ScopedTempDir temp_dir = ScopedTempDir::create().value_or({});
		ASSERT_FALSE(temp_dir.path().empty());

		ASSERT_TRUE(file_write(temp_dir.path() / "exr_wrong_extension.tga", exr_magic()));
		EXPECT_EQ(ImageFormat::EXR, image_format_from_file(temp_dir.path() / "exr_wrong_extension.tga"));

		ASSERT_TRUE(file_write(temp_dir.path() / "jpeg_wrong_extension.tga", jpeg_magic()));
		EXPECT_EQ(ImageFormat::JPEG, image_format_from_file(temp_dir.path() / "jpeg_wrong_extension.tga"));

		ASSERT_TRUE(file_write(temp_dir.path() / "png_wrong_extension.tga", png_magic()));
		EXPECT_EQ(ImageFormat::PNG, image_format_from_file(temp_dir.path() / "png_wrong_extension.tga"));

		ASSERT_TRUE(file_write(temp_dir.path() / "wepb_wrong_extension.tga", webp_magic()));
		EXPECT_EQ(ImageFormat::WEBP, image_format_from_file(temp_dir.path() / "wepb_wrong_extension.tga"));
	}

	TEST(ImageFormat, FileDoesNotExist)
	{
		ScopedTempDir temp_dir = ScopedTempDir::create().value_or({});
		ASSERT_FALSE(temp_dir.path().empty());

		EXPECT_EQ(ImageFormat::UNDETERMINED,
		          image_format_from_file(temp_dir.path() / "does_not_exist_and_unknown_extension.foo"));

		EXPECT_EQ(ImageFormat::JPEG,
		          image_format_from_file(temp_dir.path() / "does_noe_exist_and_known_extension.jpg"));
	}

	TEST(ImageReadFile, EXR)
	{
		test_read_file("exr", exr_data_1x1());
	}

	TEST(ImageReadFile, JPEG)
	{
		test_read_file("jpeg", jpeg_data_1x1());
	}

	TEST(ImageReadFile, PNG)
	{
		test_read_file("png", png_data_1x1());
	}

	TEST(ImageReadFile, TGA)
	{
		test_read_file("tga", tga_data_1x1());
	}

	TEST(ImageReadFile, WebP)
	{
		test_read_file("webp", webp_data_1x1());
	}
}
