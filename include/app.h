#pragma once
#include <string>
#include "json11.hpp"

enum AppType
{
	THREEDSX,
	CIA,
	A9LH
};

class App
{
public:
	App(json11::Json manifest);

	std::string name;
	std::string update_url;
	std::vector<AppType> types;
};