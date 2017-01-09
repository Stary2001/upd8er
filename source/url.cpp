#include <3ds.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "http.h"

using namespace util::http;

URL::URL()
{}

URL::URL(std::string url)
{
	int start = 0;
	int end = url.find("://");

	if(end != std::string::npos)
	{
		host_relative = false;
		path_relative = false;
		schema = url.substr(start, end-start);
		end += 3;
		start = end;

		end = url.find("/", end);
		if(end != std::string::npos)
		{
			host = url.substr(start, end-start);
		}
	}
	else
	{
		host_relative = true;
	}
			
	if(end != std::string::npos)
	{
		port = url.substr(start, end-start);
		end++;
		start = end;

		/*if(port == "")
		{
			port = schema == "http" ? "80" : "443";
		}*/
	}
	else
	{
		end = 0;
	}

	path = url.substr(end);
}

std::string URL::to_str()
{
	std::string s;
	if(!host_relative)
	{
		s += schema;
		s += "://";
		s += host;
	}

	if(!path_relative)
	{
		s += "/";
	}
	s += path;

	return s;
}