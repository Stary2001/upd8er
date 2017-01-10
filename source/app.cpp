#include <3ds.h>
#include <stdio.h>
#include "json11.hpp"
#include "util.h"
#include "http.h"
#include "app.h"

App::App(json11::Json manifest)
{
	update_checker = nullptr;

	name = manifest["name"].string_value();
	if(!manifest["update"].is_null())
	{
		std::string update_type = manifest["update"]["type"].string_value();
		if(update_type == "github")
		{
			std::string gh_user = manifest["update"]["user"].string_value();
			std::string gh_repo = manifest["update"]["repo"].string_value();
			update_checker = new GHReleasesChecker(name, gh_user, gh_repo);
		}
		else if(update_type == "hash")
		{
			std::string url = manifest["update"]["url"].string_value();
			update_checker = new HashUpdateChecker(name, url);
		}
	}
}

bool App::check()
{
	if(update_checker)
	{
		return update_checker->check();
	}
}

HashUpdateChecker::HashUpdateChecker(std::string _name, std::string _url)
{
	name = _name;
	update_url = _url;
	update_type = HashAndCompare;
}

bool HashUpdateChecker::check()
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

GHReleasesChecker::GHReleasesChecker(std::string _name, std::string _user, std::string _repo)
{
	name = _name;
	user = _user;
	repo = _repo;
	update_type = GithubReleases;
}

bool GHReleasesChecker::check()
{
	printf("GH releases todo!!\n");
	return false;
}