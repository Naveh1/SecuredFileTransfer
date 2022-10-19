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
std::string string_to_hex(const std::string& in, const int len);

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


std::string myHexify(const unsigned char* buffer, unsigned int length)
{
	std::stringstream ss;
	std::ios::fmtflags f(ss.flags());
	ss << std::hex;
	for (size_t i = 0; i < length; i++)
		ss << std::setfill('0') << std::setw(2) << (0xFF & buffer[i]);
		//ss << std::setfill('0') << std::setw(2) << (0xFF & buffer[i]) << (((i + 1) % 16 == 0) ? "\n" : " ");
	//ss << std::endl;
	ss.flags(f);

	return ss.str();
}

void createInfoFile(const std::string& name, const std::string& ID) 
{
	// 1. Create an RSA decryptor. this is done here to generate a new private/public key pair
	RSAPrivateWrapper rsapriv;

	// 2. get the public key
	//std::string pubkey = rsapriv.getPublicKey();	// you can get it as std::string ...
	
	//hexify(pubkeybuff, RSAPublicWrapper::KEYSIZE);
	std::ofstream infoFile(INFO_FILE);
	if (!infoFile)
	{
		std::cerr << "Couldn't open/create info file for writing at: " << INFO_FILE << std::endl;
		exit(0);
	}
	infoFile << name << std::endl;
	//infoFile << std::hex << ID;
	//std::cout << std::endl << string_to_hex(ID);
	infoFile << string_to_hex(ID, REGISTRATION_RESPONSE_PAYLOAD);
	//for (const char& a : ID)
	//	infoFile << std::hex << a;
	infoFile << std::endl << Base64Wrapper::encode(rsapriv.getPrivateKey());
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
	char* reply = new char[MAX_REPLY_LEN];
	size_t reply_len = 0;
	try {
		boost::asio::write(s, boost::asio::buffer(req));

		//reply_len = boost::asio::read(s, boost::asio::buffer(reply, MAX_REPLY_LEN));
		reply_len = s.read_some(boost::asio::buffer(reply, MAX_REPLY_LEN));

	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		exit(0);
	}

	return reply;
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

	//std::cout << std::string(reply) << std::endl;		//debug
	ResponseProcessor resp(reply);

	resp.processResponse();
	if (resp.getCode() == REGISTRATION_SUCCESS) {
		std::cout << "ID: " << resp.getPayload() << std::endl;		//debug

		for (const char& i : std::string(resp.getPayload()))
			std::cout << std::hex << (uint8_t)i;				//debug
	}
	else  if (resp.getCode() == REGISTRATION_FAIL) {
		std::cerr << "Registration failed" << std::endl;
		exit(0);
	}
	else
		std::cerr << "Unknown code" << std::endl;

	delete reply;

	return {infoData.name, resp.getPayload(), ""};
}

void sendKey(tcp::socket& s, const UserData& userData)
{
	RSAPrivateWrapper pKey(userData.privateKey);
	//std::cout << "public key size: " << pKey.getPublicKey().size() << std::endl;
	//std::string name = RequestProcessor::padName(userData.userName);
	char payload[NAME_LEN + PUBLICKEY_LEN - 1] = { 0 };
	memcpy(payload, userData.userName.c_str(), userData.userName.size());
	memcpy(payload + NAME_LEN - 1, pKey.getPublicKey().c_str(), PUBLICKEY_LEN);
	//std::cout << payload;
	//std::cout << pKey.getPublicKey();

	RequestProcessor req((uint8_t)VERSION, (uint16_t)SEND_PUBLIC_KEY, (uint32_t)(NAME_LEN + PUBLICKEY_LEN), payload, userData.userId.c_str());
	//RequestProcessor req((uint8_t)VERSION, (uint16_t)SEND_PUBLIC_KEY, (uint32_t)(NAME_LEN + PUBLICKEY_LEN), (name + pKey.getPublicKey()).c_str(), userData.userId.c_str());
	char* reply = request(s, req.serializeResponse());

	ResponseProcessor resp(reply);
	char aesKey[AES_KEY_LEN];
	resp.processResponse(aesKey);

	delete reply;
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


std::string string_to_hex(const std::string& in, const int len = -1) {
	//std::stringstream ss;
	size_t stop = len;
	if (len == -1)
		stop = in.length();
	//for (size_t i = 0; i < stop; ++i) {
	//	ss << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(in[i]));
	//}

	//return ss.str();

	return myHexify((const unsigned char*)in.c_str(), stop);
}

std::string hexToString(const std::string& in) {
	std::string output;

	if ((in.length() % 2) != 0) {
		throw std::runtime_error("String is not valid length ...");
	}

	size_t cnt = in.length() / 2;

	for (size_t i = 0; cnt > i; ++i) {
		uint32_t s = 0;
		std::stringstream ss;
		ss << std::hex << in.substr(i * 2, 2);
		ss >> s;

		output.push_back(static_cast<unsigned char>(s));
	}

	return output;
}

UserData processInfoFile() 
{

	std::string name, id, privateKey, line;
	std::ifstream infoFile = std::ifstream(INFO_FILE);
	if (!infoFile)
	{
		std::cerr << "Error opening information file" << std::endl;
		exit(0);
	}

	//infoFile >> name;
	std::getline(infoFile, name);
	infoFile >> id;

	while (std::getline(infoFile, line))
		privateKey += line;
	//infoFile >> privateKey;
	return { name, hexToString(id.substr(0, CLIENT_ID_LEN)), Base64Wrapper::decode(privateKey) };
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