#include <fstream>
#include <3ds.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

Result util::file_sha256(std::string file, u8 *hash)
{
	Result res;

	std::ifstream f(file.c_str(), std::ios::in | std::ios::binary);
	f.seekg(0, std::ios::end);
	size_t file_sz = f.tellg();
	f.seekg(0, std::ios::beg);

	size_t chunk = file_sz; // updatesha256context sucks, apparently??

	u8 *buff = (u8*)malloc(chunk);
	memset(hash, 0, 0x20);

	size_t hash_len;
	while(true)
	{
		f.read((char*)buff, chunk);
		hash_len = f.gcount();
		
		if(hash_len == 0)
		{
			break;
		}

		if(R_FAILED(res = FSUSER_UpdateSha256Context(buff, hash_len, hash)))
		{
			free(buff);
			f.close();

			return res;
		}
	}

	free(buff);
	f.close();

	return 0;
}