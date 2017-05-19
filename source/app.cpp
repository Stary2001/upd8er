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
				std::string prefer;
				if(u["prefer"].is_string())
				{
					prefer = u["prefer"];
				}

				update = new GHReleasesUpdate(gh_user, gh_repo, prefer);
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

	Action *last_action = nullptr;
	if(manifest["post"].is_array())
	{
		for(auto it = manifest["post"].begin(); it != manifest["post"].end(); it++)
		{
			Action *this_action = Action::parse(it.value());
			if(this_action != nullptr)
			{
				if(last_action != nullptr)
				{
					last_action->next = this_action;
				}
				last_action = this_action;
				post_actions.push_back(this_action);
			}
		}
	}
	else if(manifest["post"].is_object())
	{
		Action *this_action = Action::parse(manifest["post"]);
		if(this_action != nullptr)
		{
			if(last_action != nullptr)
			{
				last_action->next = this_action;
			}
			last_action = this_action;
			post_actions.push_back(this_action);
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

std::string HashUpdate::get_url()
{
	return update_url;
}

GHReleasesUpdate::GHReleasesUpdate(std::string _user, std::string _repo, std::string _prefer)
{
	user = _user;
	repo = _repo;
	prefer = _prefer;
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

std::string GHReleasesUpdate::get_url()
{
	std::string url = "https://api.github.com/repos/";
	url += user + "/" + repo;
	url += "/releases/latest";

	std::string response;
	Result res = util::http::download_string(url, response);

	if(R_SUCCEEDED(res))
	{
		json j = json::parse(response);
		std::string url;

		for(auto it = j["assets"].begin(); it != j["assets"].end(); it++)
		{
			json asset = it.value();
			url = asset["browser_download_url"];
			std::string name = asset["name"];

			printf("maybe preferring %s\n", name.c_str());

			if(prefer != "")
			{
				if(name.substr(name.length() - prefer.length(), prefer.length()) == prefer)
				{
					break;
				}
			}
		}
		return url;
	}

	return "";
}