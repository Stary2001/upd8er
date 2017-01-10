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
	std::string prefix = "/upd8er/";
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

		json11::Json val = json11::Json::parse(util::read_file(name), json_errmsg);
		if(!val.is_null())
		{
			App app(val);
			apps.push_back(app);
			printf("added %s\n", app.name.c_str());
		}
		else
		{
			printf("Invalid JSON: '%s'", json_errmsg.c_str());
		}
	}
	closedir(dir);

	return true;
}

int main(int argc, char **argv)
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	httpcInit(0x10000);
	amInit();

	printf("Loading items..\n");

	std::vector<App> apps;
	if(!load_apps(apps))
	{
		printf("loading items failed\n :(");
	}

	while (aptMainLoop())
	{
		hidScanInput();
		u32 keys = hidKeysDown();
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
		if (keys & KEY_START)
		{
			printf("aa\n");
			break;
		}
	}

	amExit();
	httpcExit();
	gfxExit();
	return 0;
}