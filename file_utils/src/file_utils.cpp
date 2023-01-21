/*
 Name:		file_utils.cpp
 Created:	1/20/2023 10:51:06 PM
 Author:	jiaji
 Editor:	http://www.visualmicro.com
*/

#include "file_utils.h"

namespace cst {
	void _recursive_forward_fs_iterator_impl(File& root, const for_each_file_cb& cb, size_t& file_count)
	{
		for (; auto next_file = root.openNextFile(); ++file_count)
		{
			cb(next_file);
			if (next_file.isDirectory())
				_recursive_forward_fs_iterator_impl(next_file, cb, file_count);
		}
	}

	void _recursive_reverse_fs_iterator_impl(File& root, const for_each_file_cb& cb, size_t& file_count)
	{
		for (; auto next_file = root.openNextFile(); ++file_count)
		{
			if (next_file.isDirectory())
				_recursive_reverse_fs_iterator_impl(next_file, cb, file_count);
			cb(next_file);
		}
	}

	size_t forward_fs_iterator(File root, const for_each_file_cb& cb, bool recursive)
	{
		size_t file_count = 0;
		if (recursive)
			_recursive_forward_fs_iterator_impl(root, cb, file_count);
		else
			for (; auto next_file = root.openNextFile(); ++file_count)
				cb(next_file);
		return file_count;
	}

	size_t reverse_fs_iterator(File root, const for_each_file_cb& cb, bool recursive)
	{
		size_t file_count = 0;
		if (recursive)
			_recursive_reverse_fs_iterator_impl(root, cb, file_count);
		else
			for (; auto next_file = root.openNextFile(); ++file_count)
				cb(next_file);
		return file_count;
	}

	size_t recursive_delete(SDFS& disk, File root)
	{
		auto f = [&](File& f)
		{
			f.isDirectory() ? disk.rmdir(f.path()) : disk.remove(f.path());
		};
		return reverse_fs_iterator(root, f);
	}

	JSONVar parse_json_file(File path)
	{
		return JSON.parse(path.readString());
	}

	path_info_t parse_path(const String& path)
	{
		auto path_split_index = path.indexOf("/", 1);
		auto disk = path.substring(0, path_split_index);
		auto dir = path.substring(path_split_index);
		return { disk, dir };
	}

}
