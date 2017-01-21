#include <3ds.h>
#include <stdio.h>
#include "util.h"
#include "http.h"
#include "app.h"

App::App(json manifest)
{
	id = manifest["id"];
	name = manifest["name"];

	if(manifest["branches"].is_object())
	{
		for(auto it = manifest["branches"].begin(); it != manifest["branches"].end(); it++)
		{
			std::string name = it.key();
			json u = it.value();

			Update *update = nullptr;

			std::string update_type = u["type"];
			if(update_type == "github")
			{
				std::string gh_user = u["user"];
				std::string gh_repo = u["repo"];
				update = new GHReleasesUpdate(gh_user, gh_repo);
			}
			else if(update_type == "hash")
			{
				std::string url = u["url"];
				update = new HashUpdate(url);
			}

			if(update != nullptr)
			{
				update_branches[name] = update;
			}
		}
	}

	if(manifest["post"].is_array())
	{
		for(auto it = manifest["post"].begin(); it != manifest["post"].end(); it++)
		{
			post_actions.push_back(Action::parse(it.value()));
		}
	}
}

HashUpdate::HashUpdate(std::string _url)
{
	update_url = _url;
	update_type = HashAndCompare;
}

Release HashUpdate::get_latest()
{
	u8 *buff;
	size_t len;
	if(R_FAILED(util::http::download_buffer(update_url, buff, len, 0x1000)))
	{
		return Release();
	}

	u8 hash[0x20];
	util::sha256(buff, len, hash);
	for(int i = 0; i<0x20; i++)
	{
		printf("%02x", hash[i]);
	}
	printf("\n");

	return Release(hash);
}

GHReleasesUpdate::GHReleasesUpdate(std::string _user, std::string _repo)
{
	user = _user;
	repo = _repo;
	update_type = GithubReleases;
}

Release GHReleasesUpdate::get_latest()
{
	std::string url = "https://api.github.com/repos/";
	url += user + "/" + repo;
	url += "/releases/latest";

	std::string response;
	Result res = util::http::download_string(url, response);

	if(R_SUCCEEDED(res))
	{
		json j = json::parse(response);
		std::string tag = j["tag_name"];
		std::string at = j["published_at"];
		printf("tag %s at %s\n", tag.c_str(), at.c_str());
		return Release(tag, at);
	}

	return Release();
}

Release::Release()
{}

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

void to_hex(std::string &s, u8 *buff, size_t len)
{
	const char *chars = "0123456789abcdef";

	for(int i = 0; i < len; i++)
	{
		s += chars[(buff[i] & 0xf0) >> 4];
		s += chars[buff[i] & 0xf];
	}
}

void from_hex(u8 *buff, size_t len, std::string &s)
{
	for(int i = 0; i < len; i++)
	{
		u8 c = buff[i*2];
		if(c >= '0' && c <= '9') { buff[i] = c - '0'; }
		else if(c >= 'a' && c <= 'f') { buff[i] = c - 'a'; }
		c = buff[(i*2) + 1];
		if(c >= '0' && c <= '9') { buff[i] |= (c - '0') << 4; }
		else if(c >= 'a' && c <= 'f') { buff[i] |= (c - 'a') << 4; }
	}
}

Release::Release(std::string _sha_hash_str)
{
	from_hex(sha_hash, 0x20, _sha_hash_str);
}

std::string Release::to_str()
{
	if(update_type == HashAndCompare)
	{
		std::string s;
		to_hex(s, sha_hash, 4);
		return s;
	}
	else if(update_type == GithubReleases)
	{
		return tag + " at " + timestamp;
	}
}