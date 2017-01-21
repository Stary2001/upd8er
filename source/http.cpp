#include <3ds.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include "http.h"
#include "util.h"

using namespace util::http;

HTTPContext::HTTPContext()
{
	bad = true;
}

HTTPContext::HTTPContext(HTTPC_RequestMethod meth, std::string url)
{
	bad = false;
	requesting = false;

	Result r = httpcOpenContext(&_ctx, meth, url.c_str(), 1 /* use default proxy */);
	if(R_FAILED(r))
	{
		printf("OpenContext failed with %08x!!\n", r);
		bad = true;
	}
}

Result HTTPContext::begin_request()
{
	requesting = true;
	return httpcBeginRequest(&_ctx);
}

Result HTTPContext::cancel()
{
	requesting = false;
	return httpcCancelConnection(&_ctx);
}

Result HTTPContext::close()
{
	bad = true;
	return httpcCloseContext(&_ctx);
}

Result HTTPContext::get_status_code(u32 *status_code)
{
	return httpcGetResponseStatusCode(&_ctx, status_code);
}

Result HTTPContext::get_file_size(u32 *content_size)
{
	return httpcGetDownloadSizeState(&_ctx, nullptr, content_size);
}

Result HTTPContext::download_data(u8 *buffer, u32 size, u32 *downloaded_size)
{
	return httpcDownloadData(&_ctx, buffer, size, downloaded_size);
}

Result HTTPContext::get_header(std::string name, std::string &str)
{
	static char header_buf[512];
	Result r = httpcGetResponseHeader(&_ctx, name.c_str(), header_buf, 512);
	if(R_FAILED(r))
	{
		return r;
	}
	else
	{
		str = std::string(header_buf);
		return 0;
	}
}

Result HTTPContext::add_header(std::string name, std::string value)
{
	return httpcAddRequestHeaderField(&_ctx, name.c_str(), value.c_str());
}

Result HTTPContext::set_ssl_options(u32 options)
{
	return httpcSetSSLOpt(&_ctx, options);
}

Result HTTPContext::set_keepalive(HTTPC_KeepAlive keepalive)
{
	return httpcSetKeepAlive(&_ctx, keepalive);
}

Result do_redirect(HTTPContext &ctx, std::string &old_url, URL &new_url)
{
	std::string new_url_str;
	Result res;
	if(R_FAILED(res = ctx.get_header("Location", new_url_str)))
	{
		return res;
	}

	ctx.cancel();

	new_url = URL(old_url);
	URL loc_url(new_url_str);

	if(loc_url.host_relative) // Is it relative?
	{
		if(loc_url.path_relative)
		{
			return MAKERESULT(RL_FATAL, RS_NOTSUPPORTED, RM_HTTP, RD_NOT_IMPLEMENTED);
		}
		else  // Absolute as a path. (/something)
		{
			new_url.path = new_url_str;
		}
	}
	else
	{
		new_url = loc_url;
	}
	return 0;
}

Result util::http::start_download(std::string url, HTTPContext &ctx, size_t limit, int recursion)
{
	if(recursion == 10) // 10 redirects is too far...
	{
		return MAKERESULT(RL_FATAL, RS_NOTSUPPORTED, RM_HTTP, RD_OUT_OF_RANGE);
	}

	Result res;

	ctx = HTTPContext(HTTPC_METHOD_GET, url);
	
	CHECKED(ctx.set_ssl_options(SSLCOPT_DisableVerify));
	CHECKED(ctx.set_keepalive(HTTPC_KEEPALIVE_ENABLED));

	if(limit != 0)
	{
		char itoa_buffer[33];
		itoa(limit - 1, itoa_buffer, 10);
		std::string s = "bytes=0-" + std::string(itoa_buffer);

		CHECKED(ctx.add_header("Range", s));
	}

	CHECKED(ctx.add_header("User-Agent", "upd8er/3ds - v1.0"));

	CHECKED(ctx.begin_request());

	u32 resp_status = 0;
	
	CHECKED(ctx.get_status_code(&resp_status))

	if(resp_status == 200 || resp_status == 206)
	{
		//printf("Got %i!\n", resp_status);
	}
	else if((resp_status >= 301 && resp_status <= 303) || (resp_status >= 307 && resp_status <= 308))
	{
		URL new_url;
		CHECKED(do_redirect(ctx, url, new_url));
		ctx.cancel();
		ctx.close();

		return start_download(new_url.to_str(), ctx, limit, recursion + 1);
	}
	else
	{
		//printf("HTTP *not* OK for %s, got %i instead!\n", url.c_str(), resp_status);
		ctx.cancel();
		ctx.close();

		return MAKERESULT(RL_FATAL, RS_NOTSUPPORTED, RM_HTTP, RD_NOT_IMPLEMENTED);
	}

	return 0;
}

Result util::http::download_buffer(std::string url, u8 *& buff, size_t &len, size_t limit)
{
	Result res;

	HTTPContext ctx;
	CHECKED(start_download(url, ctx, limit));

	u32 file_size = 0;
	CHECKED(ctx.get_file_size(&file_size))
	u32 chunk = 0x10000;

	if(limit != 0)
	{
		if(limit < chunk) chunk = limit;
		if(file_size > limit) file_size = limit;
	}

	if(file_size != 0)
	{
		buff = (u8*)malloc(file_size);
		len = file_size;
	}
	else
	{
		buff = (u8*)malloc(chunk);
		len = chunk;
	}

	u32 readsize = 0;
	u32 downloaded_total = 0;

	do
	{
		if((len - downloaded_total) < chunk)
		{
			//printf("Resizing buffer to %i bytes!\n", len + (chunk - (len % chunk)));
			u8 *oldbuff = buff;
			buff = (u8*)realloc(buff, len + (chunk - (len % chunk)));
			if(buff == nullptr)
			{
				//printf("REALLOC FAILED!!\n");
				free(oldbuff);
				return MAKERESULT(RL_FATAL, RS_OUTOFRESOURCE, RM_HTTP, RD_TOO_LARGE);
			}
		}

		res = ctx.download_data(buff + downloaded_total, chunk, &readsize);

		if(res != HTTPC_RESULTCODE_DOWNLOADPENDING && R_FAILED(res))
		{
			free(buff);
			buff = nullptr;
			return res;
		}

		downloaded_total += readsize;
		if(limit != 0 && downloaded_total >= limit)
		{
			ctx.cancel();
			break;
		}
	}
	while(res == ((Result)HTTPC_RESULTCODE_DOWNLOADPENDING));
	ctx.close();

	len = downloaded_total;

	return 0;
}

Result util::http::download_string(std::string url, std::string &s)
{
	Result res;

	HTTPContext ctx;
	CHECKED(start_download(url, ctx));

	u32 file_size = 0;
	CHECKED(ctx.get_file_size(&file_size))
	if(file_size != 0)
	{
		s.resize(file_size);
	}

	u8 *buff = (u8*)s.data();

	u32 downloaded = 0;
	u32 downloaded_total = 0;
	u32 chunk = 0x1000;

	do
	{
		if((s.capacity() - downloaded_total) < chunk)
		{
			//printf("Resizing buffer to %i bytes!\n", s.capacity() + (chunk - (s.capacity() % chunk)));
			s.resize(s.capacity() + (chunk - (s.capacity() % chunk)));
			buff = (u8*)s.data();
		}

		res = ctx.download_data(buff + downloaded_total, chunk, &downloaded);
		if(res != HTTPC_RESULTCODE_DOWNLOADPENDING && R_FAILED(res))
		{
			delete buff;
			buff = nullptr;
			return res;
		}
		downloaded_total += downloaded;
	}
	while(res == ((Result)HTTPC_RESULTCODE_DOWNLOADPENDING));

	s.resize(downloaded_total);

	ctx.close();

	return 0;
}

Result util::http::download_file(std::string url, std::string filename)
{
	Result res;
	size_t len;
	u8 *buffer;

	if((res = download_buffer(url, buffer, len)) == 0)
	{
		std::ofstream f(filename, std::ios::out | std::ios::binary);
		f.write((char*)buffer, len);
		f.close();

		free(buffer);
		return 0;
	}
	else
	{
		return res;
	}
}