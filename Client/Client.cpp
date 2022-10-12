#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <string>

#define INFO_FILE "me.info"
#define TRANSFER_INFO_FILE "transfer.info"

using boost::asio::ip::tcp;


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

		//getting port
		getline(infoFile, line);
		int port = std::atoi(line.c_str());

		//getting username
		getline(infoFile, name);

		//getting file name
		getline(infoFile, file);

		boost::asio::connect(s, resolver.resolve(ip, std::to_string(port)));
		//send registration request
	}
	
	

	return 0;
}