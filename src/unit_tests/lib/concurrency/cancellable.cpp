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

#include "lib/concurrency/cancellable.h"

#include <gtest/gtest.h>

namespace Rayni
{
	TEST(Cancellable, NotCancelledByDefault)
	{
		Cancellable cancellable;

		EXPECT_FALSE(cancellable.cancelled());
	}

	TEST(Cancellable, Cancel)
	{
		Cancellable cancellable;

		EXPECT_FALSE(cancellable.cancelled());
		cancellable.cancel();
		EXPECT_TRUE(cancellable.cancelled());
	}

	TEST(Cancellable, Reset)
	{
		Cancellable cancellable;

		cancellable.cancel();
		cancellable.reset();
		EXPECT_FALSE(cancellable.cancelled());
	}
}
