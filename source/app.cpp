#include <3ds.h>
#include <stdio.h>
#include "util.h"
#include "http.h"
#include "app.h"

App::App(json manifest)
{
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

bool HashUpdate::check()
{
	u8 *buff;
	size_t len;
	if(R_FAILED(util::http::download_buffer(update_url, buff, len, 0x1000)))
	{
		return false;
	}

	u8 hash[0x20];
	util::sha256(buff, len, hash);
	for(int i = 0; i<0x20; i++)
	{
		printf("%02x", hash[i]);
	}
	printf("\n");

	return false;
}

GHReleasesUpdate::GHReleasesUpdate(std::string _user, std::string _repo)
{
	user = _user;
	repo = _repo;
	update_type = GithubReleases;
}

bool GHReleasesUpdate::check()
{
	printf("GH releases todo!!\n");
	return false;
}