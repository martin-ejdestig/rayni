// This file is part of Rayni.
//
// Copyright (C) 2016-2021 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/string/base85.h"

#include <array>
#include <cstdint>

namespace
{
	using Base85Alphabet = std::array<char, 85>;
	using Base85DecodingTable = std::array<std::uint8_t, 256>;

	constexpr Base85Alphabet BASE85_ALPHABET = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
	                                            'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	                                            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c',
	                                            'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
	                                            'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '!', '#', '$',
	                                            '%', '&', '(', ')', '*', '+', '-', ';', '<', '=', '>', '?', '@',
	                                            '^', '_', '`', '{', '|', '}', '~'};

	constexpr Base85DecodingTable base85_generate_decoding_table()
	{
		Base85DecodingTable table = {};

		for (std::uint8_t i = 0; i < static_cast<std::uint8_t>(BASE85_ALPHABET.size()); i++)
		{
			auto index = static_cast<std::uint8_t>(BASE85_ALPHABET[i]);
			table[index] = i + 1;
		}

		return table;
	}

	constexpr Base85DecodingTable BASE85_DECODING_TABLE = base85_generate_decoding_table();
}

namespace Rayni
{
	std::optional<std::vector<std::uint8_t>> base85_decode(const std::string &str)
	{
		std::vector<std::uint8_t> decoded_data;

		for (std::string::size_type pos = 0; pos < str.length(); pos += 5)
		{
			std::uint32_t accumulator = 0;

			for (unsigned int i = 0; i < 5; i++)
			{
				auto c = static_cast<std::uint8_t>(pos + i < str.length() ? str[pos + i] : '~');
				std::uint8_t decoded_byte = BASE85_DECODING_TABLE[c];

				if (decoded_byte == 0)
					return std::nullopt;

				decoded_byte--;

				if (i == 4)
				{
					if (accumulator > 0xffffffff / 85 ||
					    accumulator * 85 > 0xffffffff - decoded_byte)
						return std::nullopt;
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
