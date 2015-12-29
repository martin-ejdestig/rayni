/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2015 Martin Ejdestig <marejde@gmail.com>
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
		const std::string COPYRIGHT_STRING_AND_SIGN = "Copyright \xc2\xa9";
		const std::string YEAR_OR_YEARS = "20[0-9-]{2}(-20[0-9]{2})?";
		const std::string WHO = ".{4,}";

		std::regex copyright_regex(COPYRIGHT_STRING_AND_SIGN + " " + YEAR_OR_YEARS + " " + WHO);

		EXPECT_TRUE(std::regex_match(VersionInfo::copyright(), copyright_regex));
	}

	TEST(VersionInfo, Version)
	{
		const std::string GIT_HASH = "[0-9a-f]{7,40}";
		const std::string TAG = "[0-9]+\\.[0-9]+";
		const std::string COMMITS_AFTER_TAG = "[0-9]+";
		const std::string TAG_DESCRIPTION = TAG + "(-" + COMMITS_AFTER_TAG + "-g" + GIT_HASH + ")?";
		const std::string TREE_DIRTY = "-dirty";

		std::regex version_regex("(" + GIT_HASH + "|" + TAG_DESCRIPTION + ")(" + TREE_DIRTY + ")?");

		EXPECT_TRUE(std::regex_match(VersionInfo::version(), version_regex));
	}
}
