#pragma once
#include <string>
#include <3ds.h>

namespace util
{
	namespace http
	{
		class URL
		{	
		public:
			URL();
			URL(std::string url);
			std::string schema;
			std::string host;
			std::string port;
			std::string path;
			bool host_relative;
			bool path_relative;

			std::string to_str();
		};	

		class HTTPContext
		{
		public:
			HTTPContext();
			HTTPContext(HTTPC_RequestMethod meth, std::string url);
			~HTTPContext();
			Result begin_request();
			Result cancel();

			Result get_status_code(u32 *status_code);
			Result get_file_size(u32 *content_size);
			Result download_data(u8 *buffer, u32 size, u32 *downloaded_size);
			Result get_header(std::string name, std::string &str);
			Result set_ssl_options(u32 options);
			Result set_keepalive(HTTPC_KeepAlive keepalive);

			bool bad;
			bool requesting;
		private:
			httpcContext _ctx;
		};

		Result start_download(std::string url, HTTPContext &c, int recursion = 0);
		Result download_buffer(std::string url, u8 *& buff, size_t &len);
		Result download_string(std::string url, std::string &s);
		Result download_file(std::string url, std::string filename);
	}
}