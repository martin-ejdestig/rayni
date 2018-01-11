/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2016-2018 Martin Ejdestig <marejde@gmail.com>
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

#include <cstdint>
#include <vector>

#include "lib/string/base85.h"

namespace Rayni
{
	TEST(Base85, Decode)
	{
		using Data = std::vector<std::uint8_t>;

		EXPECT_EQ(Data(), base85_decode("").value());

		EXPECT_EQ(Data(), base85_decode("0").value());
		EXPECT_EQ(Data({0}), base85_decode("00").value());
		EXPECT_EQ(Data({0, 0}), base85_decode("000").value());
		EXPECT_EQ(Data({0, 0, 0}), base85_decode("0000").value());
		EXPECT_EQ(Data({0, 0, 0, 0}), base85_decode("00000").value());
		EXPECT_EQ(Data({0, 0, 0, 0}), base85_decode("000000").value());
		EXPECT_EQ(Data({0, 0, 0, 0, 0}), base85_decode("0000000").value());
		EXPECT_EQ(Data({0, 0, 0, 0, 0, 0}), base85_decode("00000000").value());
		EXPECT_EQ(Data({0, 0, 0, 0, 0, 0, 0}), base85_decode("000000000").value());
		EXPECT_EQ(Data({0, 0, 0, 0, 0, 0, 0, 0}), base85_decode("0000000000").value());

		EXPECT_EQ(Data({0x01}), base85_decode("0R").value());
		EXPECT_EQ(Data({0x01, 0x23}), base85_decode("0V4").value());
		EXPECT_EQ(Data({0x01, 0x23, 0x45}), base85_decode("0V72").value());
		EXPECT_EQ(Data({0x01, 0x23, 0x45, 0x67}), base85_decode("0V73c").value());
		EXPECT_EQ(Data({0x01, 0x23, 0x45, 0x67, 0x89}), base85_decode("0V73ci2").value());
		EXPECT_EQ(Data({0x01, 0x23, 0x45, 0x67, 0x89, 0xab}), base85_decode("0V73ciK_").value());
		EXPECT_EQ(Data({0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd}), base85_decode("0V73ciL1>").value());
		EXPECT_EQ(Data({0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef}), base85_decode("0V73ciL1@;").value());

		EXPECT_EQ(Data({0xff, 0xff, 0xff, 0xff}), base85_decode("|NsC0").value());
	}

	TEST(Base85, DecodeInvalidChar)
	{
		EXPECT_FALSE(base85_decode(" 0000"));
		EXPECT_FALSE(base85_decode("0 000"));
		EXPECT_FALSE(base85_decode("00 00"));
		EXPECT_FALSE(base85_decode("000 0"));
		EXPECT_FALSE(base85_decode("0000 "));
	}

	TEST(Base85, DecodeOverflow)
	{
		EXPECT_FALSE(base85_decode("|NsC1"));
		EXPECT_FALSE(base85_decode("|NsD0"));
		EXPECT_FALSE(base85_decode("|NtC0"));
		EXPECT_FALSE(base85_decode("|OsC0"));
		EXPECT_FALSE(base85_decode("}NsC0"));
	}
}
