#pragma once
#include <iostream>
#include <boost/asio.hpp>
using boost::asio::ip::tcp;

#define MAX_REPLY_LEN 1024


class SockHandler
{
public:
	SockHandler();
	void connect(const int port, const std::string& ip);
	char* request(const std::vector<char>& req);

private:
	boost::asio::io_context _io_context;
	tcp::socket _s = tcp::socket(_io_context);
	tcp::resolver _resolver = tcp::resolver(_io_context);
};