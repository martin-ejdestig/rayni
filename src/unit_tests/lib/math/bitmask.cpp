/**
 * This file is part of Rayni.
 *
 * Copyright (C) 2017-2019 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/math/bitmask.h"

#include <gtest/gtest.h>

#include <cstdint>

namespace Rayni
{
	namespace
	{
		enum class Foo : std::uint8_t
		{
			BAR = 1,
			BAZ = 2,
			QUX = 4
		};

		using FooMask = Bitmask<Foo>;
		RAYNI_BITMASK_GLOBAL_OPERATORS(FooMask)
	}

	TEST(Bitmask, ZeroByDefault)
	{
		EXPECT_EQ(0u, FooMask().value());
	}

	TEST(Bitmask, And)
	{
		EXPECT_EQ(1u, (Foo::BAR & Foo::BAR).value());
		EXPECT_EQ(1u, (Foo::BAR & FooMask(Foo::BAR)).value());
		EXPECT_EQ(1u, (FooMask(Foo::BAR) & Foo::BAR).value());
		EXPECT_EQ(1u, (FooMask(Foo::BAR) & FooMask(Foo::BAR)).value());

		EXPECT_EQ(0u, (Foo::BAR & Foo::BAZ).value());
		EXPECT_EQ(0u, (Foo::BAR & FooMask(Foo::BAZ)).value());
		EXPECT_EQ(0u, (FooMask(Foo::BAR) & Foo::BAZ).value());
		EXPECT_EQ(0u, (FooMask(Foo::BAR) & FooMask(Foo::BAZ)).value());
	}

	TEST(Bitmask, AndAssign)
	{
		{
			FooMask m = Foo::BAR;
			m &= Foo::BAR;
			EXPECT_EQ(1u, m.value());
		}

		{
			FooMask m = Foo::BAR;
			m &= FooMask(Foo::BAR);
			EXPECT_EQ(1u, m.value());
		}

		{
			FooMask m = Foo::BAR;
			m &= Foo::BAZ;
			EXPECT_EQ(0u, m.value());
		}

		{
			FooMask m = Foo::BAR;
			m &= FooMask(Foo::BAZ);
			EXPECT_EQ(0u, m.value());
		}
	}

	TEST(Bitmask, Or)
	{
		EXPECT_EQ(1u, (Foo::BAR | Foo::BAR).value());
		EXPECT_EQ(1u, (Foo::BAR | FooMask(Foo::BAR)).value());
		EXPECT_EQ(1u, (FooMask(Foo::BAR) | Foo::BAR).value());
		EXPECT_EQ(1u, (FooMask(Foo::BAR) | FooMask(Foo::BAR)).value());

		EXPECT_EQ(3u, (Foo::BAR | Foo::BAZ).value());
		EXPECT_EQ(3u, (Foo::BAR | FooMask(Foo::BAZ)).value());
		EXPECT_EQ(3u, (FooMask(Foo::BAR) | Foo::BAZ).value());
		EXPECT_EQ(3u, (FooMask(Foo::BAR) | FooMask(Foo::BAZ)).value());
	}

	TEST(Bitmask, OrAssign)
	{
		{
			FooMask m(Foo::BAR);
			m |= Foo::BAR;
			EXPECT_EQ(1u, m.value());
		}

		{
			FooMask m(Foo::BAR);
			m |= FooMask(Foo::BAR);
			EXPECT_EQ(1u, m.value());
		}

		{
			FooMask m(Foo::BAR);
			m |= Foo::BAZ;
			EXPECT_EQ(3u, m.value());
		}

		{
			FooMask m(Foo::BAR);
			m |= FooMask(Foo::BAZ);
			EXPECT_EQ(3u, m.value());
		}
	}

	TEST(Bitmask, Xor)
	{
		EXPECT_EQ(0u, (Foo::BAR ^ Foo::BAR).value());
		EXPECT_EQ(0u, (Foo::BAR ^ FooMask(Foo::BAR)).value());
		EXPECT_EQ(0u, (FooMask(Foo::BAR) ^ Foo::BAR).value());
		EXPECT_EQ(0u, (FooMask(Foo::BAR) ^ FooMask(Foo::BAR)).value());

		EXPECT_EQ(3u, (Foo::BAR ^ Foo::BAZ).value());
		EXPECT_EQ(3u, (Foo::BAR ^ FooMask(Foo::BAZ)).value());
		EXPECT_EQ(3u, (FooMask(Foo::BAR) ^ Foo::BAZ).value());
		EXPECT_EQ(3u, (FooMask(Foo::BAR) ^ FooMask(Foo::BAZ)).value());
	}

	TEST(Bitmask, XorAssign)
	{
		{
			FooMask m(Foo::BAR);
			m ^= Foo::BAR;
			EXPECT_EQ(0u, m.value());
		}

		{
			FooMask m(Foo::BAR);
			m ^= FooMask(Foo::BAR);
			EXPECT_EQ(0u, m.value());
		}

		{
			FooMask m(Foo::BAR);
			m ^= Foo::BAZ;
			EXPECT_EQ(3u, m.value());
		}

		{
			FooMask m(Foo::BAR);
			m ^= FooMask(Foo::BAZ);
			EXPECT_EQ(3u, m.value());
		}
	}

	TEST(Bitmask, Not)
	{
		EXPECT_EQ(0b11111110u, (~Foo::BAR).value());
		EXPECT_EQ(0b11111110u, (~FooMask(Foo::BAR)).value());
	}

	TEST(Bitmask, Equal)
	{
		EXPECT_TRUE(Foo::BAR == FooMask(Foo::BAR));
		EXPECT_TRUE(FooMask(Foo::BAR) == Foo::BAR);
		EXPECT_TRUE(FooMask(Foo::BAR) == FooMask(Foo::BAR));

		EXPECT_FALSE(Foo::BAR == FooMask(Foo::BAZ));
		EXPECT_FALSE(FooMask(Foo::BAR) == Foo::BAZ);
		EXPECT_FALSE(FooMask(Foo::BAR) == FooMask(Foo::BAZ));
	}

	TEST(Bitmask, NotEqual)
	{
		EXPECT_FALSE(Foo::BAR != FooMask(Foo::BAR));
		EXPECT_FALSE(FooMask(Foo::BAR) != Foo::BAR);
		EXPECT_FALSE(FooMask(Foo::BAR) != FooMask(Foo::BAR));

		EXPECT_TRUE(Foo::BAR != FooMask(Foo::BAZ));
		EXPECT_TRUE(FooMask(Foo::BAR) != Foo::BAZ);
		EXPECT_TRUE(FooMask(Foo::BAR) != FooMask(Foo::BAZ));
	}

	TEST(Bitmask, IsSet)
	{
		const FooMask mask_1bit(Foo::BAR);
		const FooMask mask_2bits(Foo::BAR | Foo::BAZ);

		EXPECT_TRUE(mask_1bit.is_set(Foo::BAR));
		EXPECT_FALSE(mask_1bit.is_set(Foo::BAZ));

		EXPECT_FALSE(mask_1bit.is_set(Foo::BAR | Foo::BAZ));

		EXPECT_TRUE(mask_2bits.is_set(Foo::BAR));
		EXPECT_TRUE(mask_2bits.is_set(Foo::BAZ));
		EXPECT_FALSE(mask_2bits.is_set(Foo::QUX));

		EXPECT_TRUE(mask_2bits.is_set(Foo::BAR | Foo::BAZ));
		EXPECT_FALSE(mask_2bits.is_set(Foo::BAR | Foo::QUX));
		EXPECT_FALSE(mask_2bits.is_set(Foo::BAZ | Foo::QUX));

		EXPECT_FALSE(mask_2bits.is_set(Foo::BAR | Foo::BAZ | Foo::QUX));

		EXPECT_TRUE(mask_1bit.is_set(FooMask()));
		EXPECT_TRUE(mask_2bits.is_set(FooMask()));
	}

	TEST(Bitmask, Empty)
	{
		EXPECT_TRUE(FooMask().empty());
		EXPECT_FALSE(FooMask(Foo::BAR).empty());
	}
}
