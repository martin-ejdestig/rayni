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

#ifndef _RAYNI_UNIT_TESTS_LIB_EXPECT_REAL_EQ_H_
#define _RAYNI_UNIT_TESTS_LIB_EXPECT_REAL_EQ_H_

#ifdef RAYNI_DOUBLE_PRECISION
#define EXPECT_REAL_EQ EXPECT_DOUBLE_EQ
#else
#define EXPECT_REAL_EQ EXPECT_FLOAT_EQ
#endif

#endif // _RAYNI_UNIT_TESTS_LIB_EXPECT_REAL_EQ_H_