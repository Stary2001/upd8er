#pragma once
#include <string>
#include "json11.hpp"

enum AppType
{
	THREEDSX,
	CIA,
	A9LH
};

enum UpdateType
{
	HashAndCompare,
	GithubReleases,
};

class UpdateChecker
{
public:
	std::string name;
	UpdateType update_type;
	virtual bool check() = 0;
};

class GHReleasesChecker : public UpdateChecker
{
public:
	GHReleasesChecker(std::string name, std::string _user, std::string _repo);
	std::string repo;
	std::string user;

	virtual bool check();
};

class HashUpdateChecker: public UpdateChecker
{
public:
	HashUpdateChecker(std::string name, std::string _url);
	std::string update_url;

	virtual bool check();
};

class App
{
public:
	App(json11::Json manifest);

	bool check();

	std::string name;
	std::vector<AppType> types;
	UpdateChecker *update_checker;
};