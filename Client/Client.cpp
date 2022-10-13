#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <string>
#include "RequestProcessor.h"
#include "ResponseProcessor.h"

#define INFO_FILE "me.info"
#define TRANSFER_INFO_FILE "transfer.info"

using boost::asio::ip::tcp;

#define NAME_LEN_IN_FILE 100
#define NAME_LEN 255
#define VERSION 3

#define MIN_PORT 1024
#define MAX_PORT 65536

#define MAX_REPLY_LEN 1024

enum codes {REGISTRATION = 1100};

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
		if (!infoFile)
		{
			std::cout << "Couldn't file transfer info file! aborting" << std::endl;
			return 0;
		}
		std::cout << "Read file and register" << std::endl;
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

		/*if (!existsTest(file))
		{
			std::cerr << "Error opening file" << std::endl;
			return 0;
		}*/

		Sleep(5000);

		std::cout << "Connecting to " << ip << " on port " << std::to_string(port) << std::endl;

		try {
			boost::asio::connect(s, resolver.resolve(ip, std::to_string(port)));
		}
		catch (std::exception& e) {
			std::cerr << "Error connecting: " << e.what() << std::endl;
			return 0;
		}
		std::cout << "Name: " << name << std::endl;
		//send registration request
		RequestProcessor req(VERSION, REGISTRATION, NAME_LEN, name.c_str());
		auto requ = req.serializeResponse(true);
		std::cout << "len: "  << requ.size() << std::endl;
		for (char r : requ)
			std::cout << r;
		std::cout << std::endl;
		boost::asio::write(s, boost::asio::buffer(req.serializeResponse(true)));
	
		//std::string reply(MAX_REPLY_LEN, '\0');
		char reply[MAX_REPLY_LEN] = { 0 };

		boost::asio::streambuf response;
	
		try {
			do {
				boost::asio::read(s, response, ec);
				std::cout << ".";
				Sleep(100);
			} while (ec == boost::asio::error::eof);
			if (ec)
				throw std::runtime_error(ec.message());
			//boost::asio::read(s, boost::asio::buffer(reply, MAX_REPLY_LEN));
		}
		catch (std::exception& e) {
			std::cerr << "Error: " << e.what();
			exit(0);
		}

		ResponseProcessor resp(std::string((std::istreambuf_iterator<char>(&response)), std::istreambuf_iterator<char>()).c_str());
		//ResponseProcessor resp(reply);

		resp.processResponse();

		if (resp.getCode() == REGISTRATION_FAIL)
			return 0;
	}

	return 0;
}