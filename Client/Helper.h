#pragma once

#include <iostream>

#define REGISTRATION_RESPONSE_PAYLOAD 16

#define INFO_FILE "me.info"
#define TRANSFER_INFO_FILE "transfer.info"

#define MIN_PORT 1024
#define MAX_PORT 65536

#define NAME_LEN_IN_FILE 100

#define HEX_FACTOR 2


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


class Helper
{
public:
	static bool existsTest(const std::string& name);
	static void createInfoFile(const std::string& name, const std::string& ID);
	static InfoFileData setupUserData();
	static UserData processInfoFile();
};