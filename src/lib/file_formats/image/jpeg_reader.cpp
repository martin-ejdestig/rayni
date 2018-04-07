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

#include "lib/file_formats/image/jpeg_reader.h"

#include <jpeglib.h>

#include <csetjmp>
#include <cstdio>
#include <memory>
#include <string>

#include "lib/image.h"

namespace
{
	bool color_space_requires_manual_conversion(J_COLOR_SPACE color_space)
	{
		return color_space != JCS_GRAYSCALE && color_space != JCS_RGB && color_space != JCS_YCbCr;
	}
}

namespace Rayni
{
	Image JPEGReader::read_file(const std::string &file_name)
	{
		std::unique_ptr<std::FILE, decltype(&std::fclose)> file(std::fopen(file_name.c_str(), "rb"),
		                                                        std::fclose);
		if (!file)
			throw Exception(file_name, "failed to open JPEG image");

		Image image;

		if (!decode_file_to_image(*file, image))
			throw Exception(file_name, "failed to decode JPEG image");

		return image;
	}

	struct JPEGReader::ErrorManager : public jpeg_error_mgr
	{
		std::jmp_buf jump_buffer;
	};

	struct JPEGReader::Callbacks
	{
		static void error_exit(j_common_ptr jpeg_common)
		{
			// dynamic_cast not possible since no control over base class.
			// NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
			auto error_manager = static_cast<ErrorManager *>(jpeg_common->err);

			(*error_manager->output_message)(jpeg_common);

			std::longjmp(error_manager->jump_buffer, 1);
		}

		static void output_message(j_common_ptr /* jpeg_common */)
		{
			// Not sure if it is useful to store message and display it to user. Just do
			// nothing for now to prevent library from outputting on stderr in the
			// default callback. (output_message is called from other places than
			// the error_exit callback so just removing the call above does not silence
			// the library.)
		}
	};

	/**
	 * NOTE: Do not use any objects with destructors on the stack in this method.
	 *
	 * It exists solely to isolate libjpeg(-turbo)'s use of setjmp and longjmp from code that
	 * has objects with destructors on the stack. From 18.10 "Other runtime support" in the C++
	 * standard:
	 *
	 * "A setjmp/longjmp call pair has undefined behavior if replacing the setjmp and longjmp
	 * by catch and throw would invoke any non-trivial destructors for any automatic objects."
	 *
	 * A bit unclear if this only applies when unwinding or also for objects on the local stack.
	 * Have seen a few open source projects (e.g. Chromium) that call cleanup methods manually
	 * in the setjmp if-block to work with as many compilers as possible. Better to assume
	 * nothing and avoid destructors all together.
	 *
	 * Also note that the return value of various libjpeg(-turbo) functions are ignored below
	 * since they can only return false when a suspending input source is used according to the
	 * API documentation. Take advantage of this to limit calls to jpeg_destroy_decompress().
	 * See libjpeg.txt and example.c in the libjpeg-turbo source root directory for more
	 * information.
	 */
	bool JPEGReader::decode_file_to_image(std::FILE &file, Image &image) const
	{
		jpeg_decompress_struct jpeg_decompress;
		ErrorManager error_manager;

		jpeg_decompress.err = jpeg_std_error(&error_manager);
		error_manager.error_exit = Callbacks::error_exit;
		error_manager.output_message = Callbacks::output_message;

		if (setjmp(error_manager.jump_buffer))
		{
			jpeg_destroy_decompress(&jpeg_decompress);
			return false;
		}

		jpeg_create_decompress(&jpeg_decompress);
		jpeg_stdio_src(&jpeg_decompress, &file);
		jpeg_read_header(&jpeg_decompress, 1);

		if (color_space_requires_manual_conversion(jpeg_decompress.jpeg_color_space))
		{
			jpeg_destroy_decompress(&jpeg_decompress);
			return false;
		}

		jpeg_decompress.out_color_space = JCS_EXT_BGRA;
		jpeg_decompress.output_components = 4;
		image = Image(jpeg_decompress.image_width, jpeg_decompress.image_height);

		jpeg_start_decompress(&jpeg_decompress);

		while (jpeg_decompress.output_scanline < jpeg_decompress.output_height)
		{
			std::uint8_t *scanline = &image.start_of_row(jpeg_decompress.output_scanline);
			jpeg_read_scanlines(&jpeg_decompress, &scanline, 1);
		}

		jpeg_finish_decompress(&jpeg_decompress);
		jpeg_destroy_decompress(&jpeg_decompress);

		return true;
	}
}
