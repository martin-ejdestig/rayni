// This file is part of Rayni.
//
// Copyright (C) 2014-2019 Martin Ejdestig <marejde@gmail.com>
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

#include "lib/version_info.h"

#define RAYNI_GIT_DESCRIBE_VERSION_INCLUDED_FROM_SRC_LIB_VERSION_INFO_CPP
#include "git_describe_version.h"
#undef RAYNI_GIT_DESCRIBE_VERSION_INCLUDED_FROM_SRC_LIB_VERSION_INFO_CPP

namespace Rayni
{
	std::vector<std::string> VersionInfo::authors()
	{
		return {"Martin Ejdestig"};
	}

	std::string VersionInfo::copyright()
	{
		return R"(Copyright © 2013-2019 Martin Ejdestig)";
	}

	std::string VersionInfo::version()
	{
		return git_describe_version();
	}
}
