#pragma once

#include <starlight/_incLib/json.hpp>
using json = nlohmann::json;

class Action
{
public:
	Action();
	static Action* parse(json j);
};