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
	Invalid,
	HashAndCompare,
	GithubReleases,
};

class Release
{
public:
	Release();
	Release(std::string tag, std::string timestamp);
	Release(u8 *sha_hash);
	Release(std::string sha_hash_str);
	Release(json j);

	std::string tag;
	std::string timestamp;
	u8 sha_hash[0x20];

	UpdateType update_type;

	std::string to_str();
	json to_json();
};

class Update
{
public:
	UpdateType update_type;
	virtual Release get_latest() = 0;
	virtual std::string get_url() = 0;

	std::vector<Action*> post_actions;
};

class GHReleasesUpdate : public Update
{
public:
	GHReleasesUpdate(std::string _user, std::string _repo, std::string _prefer);
	std::string repo;
	std::string user;
	std::string prefer;

	virtual Release get_latest();
	virtual std::string get_url();
};

class HashUpdate: public Update
{
public:
	HashUpdate(std::string _url);
	std::string update_url;

	virtual Release get_latest();
	virtual std::string get_url();
};

class App
{
public:
	App(json manifest);

	std::string id;
	std::string name;
	std::vector<AppType> types;
	std::vector<Action*> post_actions;

	std::string preferred_branch;
	std::map<std::string, Update *> update_branches;
};

inline bool operator==(const Release& a, const Release& b)
{
	if(a.update_type != b.update_type) { return false; }
	if(a.update_type == GithubReleases)
	{
		return a.timestamp == b.timestamp;
	}
	else if(a.update_type == HashAndCompare)
	{
		return memcmp(a.sha_hash, b.sha_hash, 0x20) == 0;
	}
}

inline bool operator!=(const Release& a, const Release& b)
{
	return !(a == b);
}