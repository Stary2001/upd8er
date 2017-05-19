#include <stdio.h>
#include "action.h"
#include "util.h"
#include "minizip/ioapi_mem.h"
#include "minizip/unzip.h"

Action* Action::parse(json j)
{
	printf("Loading action!!!!\n");

	if(j["action"].is_string())
	{
		std::string name = j["action"];
		if(name == "zipextract")
		{
			if(!j["src"].is_string() || !j["dst"].is_string() || !j["target"].is_string())
			{
				return nullptr;
			}

			return new ZipExtractAction(j);
		}
		else if(name == "installcia")
		{
			if(!j["src"].is_string())
			{
				return nullptr;
			}

			return new InstallCiaAction(j);
		}
	}
	return nullptr;
}

std::string Action::do_specials(std::string s)
{
	return s;
}

ZipExtractAction::ZipExtractAction(json j)
{
	src = Action::do_specials(j["src"]);
	if(src == "$last") wants_buff = true;

	target = Action::do_specials(j["target"]);
	dst = Action::do_specials(j["dst"]);
}

InstallCiaAction::InstallCiaAction(json j)
{
	src = Action::do_specials(j["src"]);
	if(src == "$last") wants_buff = true;
}

bool ZipExtractAction::exec()
{
	u8 *zip_buff = nullptr;
	size_t zip_len = 0;

	if(wants_buff)
	{
		zip_buff = this->buff;
		zip_len = this->buff_len;
	}
	else
	{
		util::read_file_buffer(src, zip_buff, zip_len);
	}

	// DO ZIP STUFF
	// BLAH

	int err = UNZ_OK;
	zlib_filefunc_def filefunc32 = {0};
	ourmemory_t unzmem = {0};
	unzmem.size = zip_len;
	unzmem.base = (char*)zip_buff;
	fill_memory_filefunc(&filefunc32, &unzmem);
	unzFile zip_file = unzOpen2("__notused__", &filefunc32);

	if (unzLocateFile(zip_file, target.c_str(), NULL) != UNZ_OK)
	{
		printf("File %s not found in zip!\n", target.c_str());
		unzClose(zip_file);
		return false;
	}

	u8 *file_buff = nullptr;
	size_t file_len = 0;

	unz_file_info file_info = {0};
	char filename_inzip[256] = {0};

	err = unzGetCurrentFileInfo(zip_file, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

	if(err != UNZ_OK)
	{
		printf("File info failed! %i\n", err);
		unzClose(zip_file);
		return false;
	}
	else
	{
		printf("Got file info!!\n");
	}

	file_buff = (u8*)malloc(file_info.uncompressed_size);
	file_len = file_info.uncompressed_size;

	err = unzOpenCurrentFile(zip_file);
	if(err != UNZ_OK)
	{
		printf("File open failed! %i\n", err);
		unzClose(zip_file);
		return false;
	}

	err = unzReadCurrentFile(zip_file, file_buff, file_len);
	if(err < 0)
	{
		printf("File read failed! %i\n", err);
		unzClose(zip_file);
		return false;
	}

	unzClose(zip_file);

	if(next && next->wants_buff)
	{
		next->buff = file_buff;
	}
	else
	{
		util::write_file_buffer(dst, file_buff, file_len);
		free(file_buff);
	}

	free(zip_buff);

	return true;
}

bool InstallCiaAction::exec()
{
	u8 *cia_buff = nullptr;
	size_t cia_len = 0;

	if(wants_buff)
	{
		cia_buff = this->buff;
		cia_len = this->buff_len;
	}
	else
	{
		util::read_file_buffer(src, cia_buff, cia_len);
	}

	if(wants_buff)
	{
		free(cia_buff);
	}

	return false;
}