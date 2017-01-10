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

	u8 *buff = (u8*)malloc(file_sz);
	memset(hash, 0, 0x20);

	f.read((char*)buff, file_sz);
		
	res = sha256(buff, file_sz, hash);

	free(buff);
	f.close();

	return res;
}

Result util::sha256(u8 *buffer, size_t len, u8 *hash)
{
	return FSUSER_UpdateSha256Context(buffer, len, hash);
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