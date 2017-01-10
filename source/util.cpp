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

std::string util::read_file(std::string file)
{
	std::ifstream f(file.c_str(), std::ios::in | std::ios::binary);
	f.seekg(0, std::ios::end);
	size_t file_sz = f.tellg();
	f.seekg(0, std::ios::beg);

	std::string s;
	s.resize(file_sz);
	f.read((char*)s.data(), file_sz);
	f.close();
	return s;
}

size_t util::align_up(size_t in, size_t align)
{
	if(in % align == 0) return in;
	return in + (align - (in % align));
}

u64 util::bswap64(u64 in)
{
	u64 out = 0;
	char in_b[8];
	memcpy(in_b, &in, 8);

	for(int i = 0; i < 8; i++)
	{
		out |= ((u64)in_b[i]) << (8-i)*8;
	}
	return out;
}