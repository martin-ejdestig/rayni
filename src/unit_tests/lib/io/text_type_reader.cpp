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

#include "lib/io/text_type_reader.h"

#include <gtest/gtest.h>

#include <fstream>
#include <string>

#include "lib/system/scoped_temp_dir.h"

namespace Rayni
{
	namespace
	{
		class BoolReader : public TextTypeReader<bool>
		{
		private:
			bool read() override
			{
				if (skip_string("true"))
					return true;
				if (skip_string("false"))
					return false;

				throw Exception(position(), "expected \"true\" or \"false\"");
			}
		};
	}

	TEST(TextTypeReader, ReadFile)
	{
		auto text_to_file = [](const std::string &path, const std::string &text) {
			std::ofstream file(path);
			file << text;
			if (!file.good())
				FAIL() << "Failed to write to file " << path << ".";
		};

		ScopedTempDir temp_dir;
		const std::string good_path = temp_dir.path() / "good.txt";
		const std::string bad_path = temp_dir.path() / "bad.txt";
		const std::string does_not_exist_path = temp_dir.path() / "does_not_exist.txt";

		text_to_file(good_path, "true");
		text_to_file(bad_path, "bad");

		BoolReader reader;

		EXPECT_TRUE(reader.read_file(good_path));

		EXPECT_THROW(reader.read_file(bad_path), BoolReader::Exception);
		EXPECT_THROW(reader.read_file(does_not_exist_path), BoolReader::Exception);
	}

	TEST(TextTypeReader, ReadString)
	{
		BoolReader reader;

		EXPECT_TRUE(reader.read_string("true"));
		EXPECT_FALSE(reader.read_string("false"));

		EXPECT_THROW(reader.read_string("abc"), BoolReader::Exception);
		EXPECT_THROW(reader.read_string(""), BoolReader::Exception);
	}
}
