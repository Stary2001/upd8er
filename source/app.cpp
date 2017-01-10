#include "json11.hpp"
#include "app.h"

App::App(json11::Json manifest)
{
	name = manifest["name"].string_value();
	update_url = manifest["url"].string_value();
}