#pragma once
#include <3ds.h>
#include <string>

namespace util
{
	Result file_sha256(std::string file, u8 *hash);
}