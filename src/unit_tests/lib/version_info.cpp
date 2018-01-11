/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015-2018 Martin Ejdestig <marejde@gmail.com>
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

#include <regex>
#include <string>

#include "lib/version_info.h"

namespace Rayni
{
	TEST(VersionInfo, Authors)
	{
		EXPECT_FALSE(VersionInfo::authors().empty());

		for (auto &author : VersionInfo::authors())
			EXPECT_GE(author.size(), 4);
	}

	TEST(VersionInfo, Copyright)
	{
		const std::string copyright_string_and_sign = R"(Copyright ©)";
		const std::string year_or_years = "20[0-9]{2}(-20[0-9]{2})?";
		const std::string who = ".{4,}";

		std::regex copyright_regex(copyright_string_and_sign + " " + year_or_years + " " + who);

		EXPECT_TRUE(std::regex_match(VersionInfo::copyright(), copyright_regex));
	}

	TEST(VersionInfo, Version)
	{
		const std::string git_hash = "[0-9a-f]{7,40}";
		const std::string tag = "[0-9]+\\.[0-9]+";
		const std::string commits_after_tag = "[0-9]+";
		const std::string tag_description = tag + "(-" + commits_after_tag + "-g" + git_hash + ")?";
		const std::string tree_dirty = "-dirty";

		std::regex version_regex("(" + git_hash + "|" + tag_description + ")(" + tree_dirty + ")?");

		EXPECT_TRUE(std::regex_match(VersionInfo::version(), version_regex));
	}
}
