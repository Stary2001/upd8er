#pragma once
#include <3ds.h>
#include <string>
#include <stdio.h>

#define CHECKED(a) if(R_FAILED(res = (a))) { printf("%s:%i, %08x\n", __FILE__, __LINE__, res); return res; }

namespace util
{
	Result sha256(u8 *buff, size_t len, u8 *hash);
	Result file_sha256(std::string file, u8 *hash);
	std::string read_file(std::string file);

	Result install_cia(u8 *buffer, size_t len);
	u64 get_cia_title_id(u8 *buffer, size_t len);

	size_t align_up(size_t in, size_t align);
	u64 bswap64(u64 in);
}