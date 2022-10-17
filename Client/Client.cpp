#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <string>
#include "RequestProcessor.h"
#include "ResponseProcessor.h"
//#include <cryptopp/rsa.h>

#include <iomanip>

#include "Base64Wrapper.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"

#define PUBLICKEY_LEN 160

#define RESPONSE_HEAD_LEN 7
#define REGISTRATION_RESPONSE_PAYLOAD 16

struct InfoFileData 
{
	std::string ip;
	int port;
	std::string name;
	std::string file;
};


struct UserData 
{
	std::string userName;
	std::string userId;
	std::string privateKey;
};



#define INFO_FILE "me.info"
#define TRANSFER_INFO_FILE "transfer.info"

using boost::asio::ip::tcp;

#define NAME_LEN_IN_FILE 100
#define NAME_LEN 255
#define VERSION 3

#define MIN_PORT 1024
#define MAX_PORT 65536

#define MAX_REPLY_LEN 1024

enum codes {REGISTRATION = 1100, SEND_PUBLIC_KEY};


bool existsTest(const std::string& name);
void hexify(const unsigned char* buffer, unsigned int length);
void createInfoFile(const std::string& name, const std::string& ID);
void connect(tcp::socket& s, tcp::resolver& resolver, const InfoFileData& data);
UserData registerUser(tcp::socket& s, const InfoFileData&);
InfoFileData setupUserData();
UserData processInfoFile();
char* request(tcp::socket& s, const std::vector<char>& req);

bool existsTest(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}

void hexify(const unsigned char* buffer, unsigned int length)
{
	std::ios::fmtflags f(std::cout.flags());
	std::cout << std::hex;
	for (size_t i = 0; i < length; i++)
		std::cout << std::setfill('0') << std::setw(2) << (0xFF & buffer[i]) << (((i + 1) % 16 == 0) ? "\n" : " ");
	std::cout << std::endl;
	std::cout.flags(f);
}

void createInfoFile(const std::string& name, const std::string& ID) 
{
	// 1. Create an RSA decryptor. this is done here to generate a new private/public key pair
	RSAPrivateWrapper rsapriv;

	// 2. get the public key
	std::string pubkey = rsapriv.getPublicKey();	// you can get it as std::string ...
	
	//hexify(pubkeybuff, RSAPublicWrapper::KEYSIZE);
	std::ofstream infoFile(INFO_FILE);
	if (!infoFile)
	{
		std::cerr << "Couldn't open/create info file for writing at: " << INFO_FILE << std::endl;
		exit(0);
	}
	infoFile << name << std::endl << ID << std::endl << Base64Wrapper::encode(rsapriv.getPrivateKey());
}

void connect(tcp::socket& s, tcp::resolver& resolver, const InfoFileData& data)
{
	Sleep(5000);

	std::cout << "Connecting to " << data.ip << " on port " << std::to_string(data.port) << std::endl;

	try {
		boost::asio::connect(s, resolver.resolve(data.ip, std::to_string(data.port)));
	}
	catch (std::exception& e) {
		std::cerr << "Error connecting: " << e.what() << std::endl;
		exit(0);
	}
	std::cout << "Name: " << data.name << std::endl;		//debug
}

char* request(tcp::socket& s, const std::vector<char>& req)
{
	char reply[MAX_REPLY_LEN];
	size_t reply_len = 0;
	try {
		boost::asio::write(s, boost::asio::buffer(req));

		//reply_len = boost::asio::read(s, boost::asio::buffer(reply, MAX_REPLY_LEN));
		s.read_some(boost::asio::buffer(reply, MAX_REPLY_LEN));

	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		exit(0);
	}
}

UserData registerUser(tcp::socket & s, const InfoFileData& infoData)
{

	//send registration request
	RequestProcessor req(VERSION, REGISTRATION, NAME_LEN, infoData.name.c_str());
	/*auto requ = req.serializeResponse(true);
	std::cout << "len: "  << requ.size() << std::endl;
	for (char r : requ)
		std::cout << r;
	std::cout << std::endl;*/

	//std::string reply(MAX_REPLY_LEN, '\0');

	char* reply = request(s, req.serializeResponse(true));

	std::cout << reply;		//debug
	ResponseProcessor resp(reply);

	resp.processResponse();
	if (resp.getCode() == REGISTRATION_SUCCESS) {
		std::cout << "ID: " << resp.getPayload() << std::endl;		//debug

		for (const char& i : std::string(resp.getPayload()))
			std::cout << std::hex << resp.getPayload();				//debug
	}
	else  if (resp.getCode() == REGISTRATION_FAIL) {
		std::cerr << "Registration failed" << std::endl;
		exit(0);
	}
	else
		std::cerr << "Unknown code" << std::endl;


	return {infoData.name, resp.getPayload(), ""};
}

void sendKey(tcp::socket& s, const UserData& userData)
{
	RSAPrivateWrapper pKey(userData.privateKey);

	std::string name = padString(userData.userName, NAME_LEN);

	RequestProcessor req((uint8_t)VERSION, (uint16_t)SEND_PUBLIC_KEY, (uint32_t)(NAME_LEN + PUBLICKEY_LEN), (name + pKey.getPublicKey()).c_str(), userData.userId.c_str());
	char* reply = request(s, req.serializeResponse());

	ResponseProcessor resp(reply);
	char aesKey[AES_KEY_LEN];
	resp.processResponse(aesKey);
}


InfoFileData setupUserData()
{
	std::string line, ip, name, file;

	std::ifstream infoFile = std::ifstream(TRANSFER_INFO_FILE);
	if (!infoFile)
	{
		std::cerr << "Couldn't file transfer info file! aborting" << std::endl;
		exit(0);
	}
	std::cout << "Read file and register" << std::endl;
	//getting ip
	getline(infoFile, ip, ':');

	boost::system::error_code ec;
	boost::asio::ip::address::from_string(ip, ec);
	if (ec)
	{
		std::cerr << "Invalid ip: " << ec.message() << std::endl;
		exit(0);
	}

	//getting port
	getline(infoFile, line);
	int port = std::atoi(line.c_str());

	if (port < MIN_PORT || port > MAX_PORT)
	{
		std::cerr << "Invalid port numner" << std::endl;
		exit(0);
	}

	//getting username
	getline(infoFile, name);

	if (name.size() > NAME_LEN_IN_FILE)
	{
		std::cerr << "User name is too long" << std::endl;
		exit(0);
	}

	//getting file name
	getline(infoFile, file);

	/*if (!existsTest(file))
	{
		std::cerr << "Error opening file" << std::endl;
		exit(0);
	}*/ //deleting for debug

	return {ip, port, name, file};
}


UserData processInfoFile() 
{
	std::string name, id, privateKey;
	std::ifstream infoFile = std::ifstream(INFO_FILE);
	if (!infoFile)
	{
		std::cerr << "Error opening information file" << std::endl;
		exit(0);
	}

	infoFile >> name;
	infoFile >> std::hex >> id;
	infoFile >> privateKey;

	return { name, id, Base64Wrapper::decode(privateKey) };
}

int main() 
{
	boost::asio::io_context io_context;
	tcp::socket s(io_context);
	tcp::resolver resolver(io_context);
	UserData userData;
	InfoFileData infoData = setupUserData();
	
	//file = setupServer();

	connect(s, resolver, infoData);

	//std::ifstream infoFile(INFO_FILE);
	//if (!infoFile)
	if(!existsTest(INFO_FILE))
	{
		//userData = 
		userData = registerUser(s, infoData);

		createInfoFile(userData.userName, userData.userId);
	}

	userData = processInfoFile();

	sendKey(s, userData);

	return 0;
}