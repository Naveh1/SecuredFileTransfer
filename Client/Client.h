#pragma once

#include <iostream>
#include <boost/asio.hpp>


#define PUBLICKEY_LEN 160

#define RESPONSE_HEAD_LEN 7
#define REGISTRATION_RESPONSE_PAYLOAD 16

#define FILE_SENDING_TIMES 3

#define INFO_FILE "me.info"
#define TRANSFER_INFO_FILE "transfer.info"

#define NAME_LEN_IN_FILE 100
#define VERSION 3

#define MIN_PORT 1024
#define MAX_PORT 65536

#define HEX_FACTOR 2

enum codes { REGISTRATION = 1100, SEND_PUBLIC_KEY, SEND_FILE = 1103, CRC_OK = 1104, CRC_NOT_OK = 1105, CRC_ERROR = 1106 };

using boost::asio::ip::tcp;

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


struct passedKey
{
	char* key;
	unsigned int len;
};


bool existsTest(const std::string& name);
void createInfoFile(const std::string& name, const std::string& ID);
void connect(tcp::socket& s, tcp::resolver& resolver, const InfoFileData& data);
UserData registerUser(tcp::socket& s, const InfoFileData&);
InfoFileData setupUserData();
UserData processInfoFile();
char* request(tcp::socket& s, const std::vector<char>& req);
std::string string_to_hex(const std::string& in, const int len);
std::string myHexify(const unsigned char* buffer, unsigned int length);
passedKey sendKey(tcp::socket& s, const UserData& userData);
std::string hexToString(const std::string& in);
std::string decAESKey(const UserData& userData, const passedKey& key);
std::string decAESKey(const UserData& userData, const passedKey& key);
std::string encAES(const std::string& key, const std::string& content);
uint32_t sendFile(tcp::socket& s, const UserData& userData, const std::string& fileName, const std::string& encFileContent);
bool crcReq(tcp::socket& s, const UserData& userData, const std::string& fileName, const uint16_t code);
bool isHex(const std::string& str);