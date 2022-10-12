﻿#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <string>
#include "RequestProcessor.h"

#define INFO_FILE "me.info"
#define TRANSFER_INFO_FILE "transfer.info"

using boost::asio::ip::tcp;

#define NAME_LEN_IN_FILE 100
#define NAME_LEN 255
#define VERSION 3

#define MIN_PORT 1024
#define MAX_PORT 65536

enum codes {REGISTRAYION = 1100};

bool existsTest(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}

int main() 
{
	std::ifstream infoFile(INFO_FILE);
	std::string line, ip, name, file;
	int port;

	boost::asio::io_context io_context;
	tcp::socket s(io_context);
	tcp::resolver resolver(io_context);

	if (!infoFile)
	{
		infoFile = std::ifstream(TRANSFER_INFO_FILE);

		std::cout << "Read file and register";
		//getting ip
		getline(infoFile, ip, ':');

		boost::system::error_code ec;
		boost::asio::ip::address::from_string(ip, ec);
		if (ec)
		{
			std::cerr << "Invalid ip: " << ec.message() << std::endl;
			return 0;
		}

		//getting port
		getline(infoFile, line);
		int port = std::atoi(line.c_str());

		if (port < MIN_PORT || port > MAX_PORT)
		{
			std::cerr << "Invalid port numner" << std::endl;
			return 0;
		}

		//getting username
		getline(infoFile, name);

		if (name.size() > NAME_LEN_IN_FILE)
		{
			std::cerr << "User name is too long" << std::endl;
			return 0;
		}

		//getting file name
		getline(infoFile, file);

		if (!existsTest(file))
		{
			std::cerr << "Error opening file" << std::endl;
			return 0;
		}

		boost::asio::connect(s, resolver.resolve(ip, std::to_string(port)));
		//send registration request
		RequestProcessor req(VERSION, REGISTRAYION, NAME_LEN, name.c_str());
		boost::asio::write(s, boost::asio::buffer(req.serializeResponse()));
	
		std::string reply;
		boost::asio::read(s, boost::asio::buffer(reply));
	}

	return 0;
}