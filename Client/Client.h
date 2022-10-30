#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include "Helper.h"


#define PUBLICKEY_LEN 160

#define RESPONSE_HEAD_LEN 7

#define FILE_SENDING_TIMES 3

#define VERSION 3


class SockHandler;

enum codes { REGISTRATION = 1100, SEND_PUBLIC_KEY, SEND_FILE = 1103, CRC_OK = 1104, CRC_NOT_OK = 1105, CRC_ERROR = 1106 };

using boost::asio::ip::tcp;

/*
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
*/

bool existsTest(const std::string& name);
void createInfoFile(const std::string& name, const std::string& ID);
InfoFileData setupUserData();
UserData processInfoFile();

std::string decAESKey(const UserData& userData, const passedKey& key);
std::string decAESKey(const UserData& userData, const passedKey& key);
std::string encAES(const std::string& key, const std::string& content);

UserData registerUser(SockHandler& sock, const InfoFileData&);
passedKey sendKey(SockHandler& sock, const UserData& userData);
uint32_t sendFile(SockHandler& sock, const UserData& userData, const std::string& fileName, const std::string& encFileContent);
bool crcReq(SockHandler& sock, const UserData& userData, const std::string& fileName, const uint16_t code);
