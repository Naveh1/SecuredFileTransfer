#include "SockHandler.h"

//C'tor
SockHandler::SockHandler(const int port, const std::string& ip) 
{
	connect(port, ip);
}

//Connecting to server by port and ip
void SockHandler::connect(const int port, const std::string& ip)
{
	std::cout << "Connecting to " << ip << " on port " << std::to_string(port) << std::endl;

	try {
		boost::asio::connect(_s, _resolver.resolve(ip, std::to_string(port)));
	}
	catch (std::exception& e) {
		std::cerr << "Error connecting: " << e.what() << std::endl;
		exit(0);
	}
}


//Sending request to server
char* SockHandler::request(const std::vector<char>& req)
{
	char* reply = new char[MAX_REPLY_LEN];
	size_t reply_len = 0;
	try {
		boost::asio::write(_s, boost::asio::buffer(req));

		//reply_len = boost::asio::read(s, boost::asio::buffer(reply, MAX_REPLY_LEN));
		reply_len = _s.read_some(boost::asio::buffer(reply, MAX_REPLY_LEN));

	}
	catch (const std::exception& e) {
		std::cerr << "Server closed the connection." << std::endl;
		//std::cout << e.what() << std::endl;
		exit(0);
	}

	return reply;
}
