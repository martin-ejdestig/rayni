/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2016-2017 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/string/base85.h"

#include <algorithm>
#include <array>
#include <cstdint>

namespace
{
	using Base85Alphabet = std::array<char, 85>;
	using Base85DecodingTable = std::array<std::uint8_t, 256>;

	const Base85Alphabet &base85_alphabet()
	{
		static const Base85Alphabet alphabet = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
		                                        'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		                                        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c',
		                                        'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
		                                        'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '!', '#', '$',
		                                        '%', '&', '(', ')', '*', '+', '-', ';', '<', '=', '>', '?', '@',
		                                        '^', '_', '`', '{', '|', '}', '~'};
		return alphabet;
	}

	Base85DecodingTable base85_generate_decoding_table()
	{
		Base85DecodingTable table;

		std::fill(table.begin(), table.end(), 0);

		for (unsigned int i = 0; i < base85_alphabet().size(); i++)
		{
			auto index = static_cast<std::uint8_t>(base85_alphabet()[i]);
			table[index] = static_cast<std::uint8_t>(i + 1);
		}

		return table;
	}
}

namespace Rayni
{
	std::experimental::optional<std::vector<std::uint8_t>> base85_decode(const std::string &str)
	{
		static const Base85DecodingTable decoding_table = base85_generate_decoding_table();
		std::vector<std::uint8_t> decoded_data;

		for (std::string::size_type pos = 0; pos < str.length(); pos += 5)
		{
			std::uint32_t accumulator = 0;

			for (unsigned int i = 0; i < 5; i++)
			{
				std::uint8_t c = static_cast<std::uint8_t>(pos + i < str.length() ? str[pos + i] : '~');
				std::uint8_t decoded_byte = decoding_table[c];

				if (decoded_byte == 0)
					return std::experimental::nullopt;

				decoded_byte--;

				if (i == 4)
				{
					if (accumulator > 0xffffffff / 85 ||
					    accumulator * 85 > 0xffffffff - decoded_byte)
						return std::experimental::nullopt;
				}

				accumulator = accumulator * 85 + decoded_byte;
			}

			unsigned int num_bytes = str.length() - pos < 5 ? str.length() - pos - 1 : 4;

			for (unsigned int i = 0; i < num_bytes; i++)
			{
				decoded_data.emplace_back((accumulator >> 24) & 0xff);
				accumulator = accumulator << 8;
			}
		}

		return decoded_data;
	}
}
