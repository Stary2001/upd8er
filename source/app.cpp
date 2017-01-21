#include <3ds.h>
#include <stdio.h>
#include "util.h"
#include "http.h"
#include "app.h"

App::App(json manifest)
{
	if(manifest["id"].is_string()) id = manifest["id"];
	if(manifest["name"].is_string()) name = manifest["name"];

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