#include <vector>
#include <3ds.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <inttypes.h>

#include "http.h"
#include "util.h"
#include "app.h"

bool load_apps(std::vector<App> &apps)
{
	std::string prefix = "/upd8er/manifests/";
	DIR *dir = opendir(prefix.c_str());

	if(dir == nullptr)
	{
		return false;
	}

	struct dirent *dir_entry;
	while(dir_entry = readdir(dir))
	{
		std::string name = prefix + dir_entry->d_name;
		printf("found %s\n", name.c_str());

		std::string json_errmsg;

		json val = json::parse(util::read_file(name));
		if(!val.is_null())
		{
			App app(val);
			apps.push_back(app);
			printf("added %s\n", app.name.c_str());
		}
		else
		{
			//printf("Invalid JSON: '%s'", json_errmsg.c_str());
		}
	}
	closedir(dir);

	return true;
}


bool load_state(std::map<std::string, Release> &state)
{
	if(!util::file_exists("/upd8er/state.json"))
	{
		return false;
	}
	else
	{
		json j = json::parse(util::read_file("/upd8er/state.json"));
		if(j.is_null()) return false;
		for(auto it = j.begin(); it != j.end(); it++)
		{
			std::string k = it.key();
			json v = it.value();
			state[k] = Release(v); 
		}
	}

	return true;
}

bool save_state(std::string filename, std::map<std::string, Release> &state)
{
	json j = json::object();
	for(auto a : state)
	{
		j[a.first] = a.second.to_json();
	}
	std::string s = j.dump();
	util::write_file(filename, s);
	return true;
}

int main(int argc, char **argv)
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	httpcInit(0x10000);
	amInit();

	printf("Loading items..\n");

	std::map<std::string, Release> state;
	std::vector<App> apps;
	if(!load_apps(apps))
	{
		printf("loading items failed\n");
	}

	if(!load_state(state))
	{
		printf("loading state failed\n");
	}

	printf("Checking for updates...\n");
	for(auto a: apps)
	{
		printf("%s...\n", a.name.c_str());
		for(auto b: a.update_branches)
		{
			std::string k = a.id + "/" + b.first;

			Release r = b.second->get_latest();
			if(r != state[k])
			{
				printf("New update for %s/%s\n%s vs %s!\n", a.name.c_str(), b.first.c_str(), r.to_str().c_str(), state[k].to_str().c_str());
				state[k] = r;
			}
			else
			{
				printf("%s is up to date.\n", b.first.c_str());
			}
		}
	}

	save_state("upd8er/state.json", state);

	while (aptMainLoop())
	{
		hidScanInput();
		u32 keys = hidKeysDown();
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
		if (keys & KEY_START)
		{
			break; 
		}
	}

	amExit();
	httpcExit();
	gfxExit();
	return 0;
}