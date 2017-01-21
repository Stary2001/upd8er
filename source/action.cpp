#include <stdio.h>
#include "action.h"

Action* Action::parse(json j)
{
	printf("Got action!\n");
	if(j["name"].is_string())
	{
		std::string name = j["action"];
		if(name == "zipextract")
		{
			if(!j["src"].is_string() || !j["dst"].is_string())
			{
				return nullptr;
			}

			return new ZipExtractAction(j);
		}
		else if(name == "installcia")
		{
			if(!j["src"].is_string())
			{
				return nullptr;
			}

			return new InstallCiaAction(j);
		}
	}
	return nullptr;
}

std::string Action::do_specials(std::string s)
{
	return s;
}

ZipExtractAction::ZipExtractAction(json j)
{
	src = Action::do_specials(j["src"]);
	dst = Action::do_specials(j["dst"]);
}

InstallCiaAction::InstallCiaAction(json j)
{
	src = Action::do_specials(j["src"]);
	dst = Action::do_specials(j["dst"]);
}

bool ZipExtractAction::exec()
{
	return false;
}

bool InstallCiaAction::exec()
{
	return false;
}