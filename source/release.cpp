#include <3ds/types.h>
#include "util.h"
#include "app.h"

Release::Release()
{
	update_type = Invalid;
}

Release::Release(std::string _tag, std::string _timestamp)
{
	update_type = GithubReleases;
	tag = _tag;
	timestamp = _timestamp;
}

Release::Release(u8 *_hash)
{
	update_type = HashAndCompare;
	memcpy(sha_hash, _hash, 0x20);
}

Release::Release(std::string _sha_hash_str)
{
	util::from_hex(sha_hash, 0x20, _sha_hash_str);
}

Release::Release(json j)
{
	if(j["type"].is_number())
	{
		int t = j["type"];
		update_type = (UpdateType)t;
		if(update_type == HashAndCompare)
		{
			std::string s = j["hash"];
			util::from_hex(sha_hash, 0x20, s);
		}
		else if(update_type == GithubReleases)
		{
			tag = j["tag"];
			timestamp = j["timestamp"];
		}
	}
}

std::string Release::to_str()
{
	if(update_type == HashAndCompare)
	{
		std::string s;
		util::to_hex(s, sha_hash, 4);
		return s;
	}
	else if(update_type == GithubReleases)
	{
		return tag + " at " + timestamp;
	}

	return "invalid";
}

json Release::to_json()
{
	json j = json::object();
	j["type"] = (int)update_type;
	if(update_type == HashAndCompare)
	{
		std::string s;
		util::to_hex(s, sha_hash, 0x20);
		j["hash"] = s;
	}
	else if(update_type == GithubReleases)
	{
		j["tag"] = tag;
		j["timestamp"] = timestamp;
	}

	return j;
}