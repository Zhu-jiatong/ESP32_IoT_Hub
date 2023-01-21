/*
 Name:		file_utils.h
 Created:	1/20/2023 10:51:06 PM
 Author:	jiaji
 Editor:	http://www.visualmicro.com
*/

#ifndef _file_utils_h
#define _file_utils_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <SD.h>
#include <Arduino_JSON.h>
#include <functional>

using for_each_file_cb = std::function<void(File&)>;

namespace cst
{
	struct path_info_t
	{
		String disk, dir;
	};

	size_t forward_fs_iterator(File root, const for_each_file_cb& cb, bool recursive = false);
	size_t reverse_fs_iterator(File root, const for_each_file_cb& cb, bool recursive = false);
	size_t recursive_delete(SDFS& disk, File root);
	JSONVar parse_json_file(File path);
	path_info_t parse_path(const String& path);
} // namespace cst


#endif

