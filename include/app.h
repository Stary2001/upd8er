#pragma once
#include <string>
#include <map>
#include <vector>
#include <starlight/_incLib/json.hpp>
using json = nlohmann::json;

#include "action.h"
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

class Update
{
public:
	UpdateType update_type;
	virtual bool check() = 0;
	std::vector<Action*> post_actions;
};

class GHReleasesUpdate : public Update
{
public:
	GHReleasesUpdate(std::string _user, std::string _repo);
	std::string repo;
	std::string user;

	virtual bool check();
};

class HashUpdate: public Update
{
public:
	HashUpdate(std::string _url);
	std::string update_url;

	virtual bool check();
};

class App
{
public:
	App(json manifest);

	std::string name;
	std::vector<AppType> types;
	std::vector<Action*> post_actions;
	std::map<std::string, Update *> update_branches;
};