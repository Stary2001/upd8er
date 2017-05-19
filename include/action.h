#pragma once
#include <3ds/types.h>
#include <starlight/_incLib/json.hpp>
using json = nlohmann::json;

class Action
{
public:
	static Action* parse(json j);
	static std::string do_specials(std::string s);

	virtual bool exec() = 0;
	std::string src;
	std::string dst;

	u8 *buff;
	size_t buff_len;
	bool wants_buff;

	Action *next;
};

class ZipExtractAction : public Action
{
public:
	ZipExtractAction(json j);
	std::string target;
	virtual bool exec();
};

class InstallCiaAction : public Action
{
public:
	InstallCiaAction(json j);
	virtual bool exec();
};