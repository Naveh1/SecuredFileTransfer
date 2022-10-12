#pragma once

#include <iostream>
#include <vector>

#define CLIENT_ID_LEN 16
enum lengths { VERSION_LEN = 1, CODE_LEN, PAYLOAD_SIZE = 4 };

class RequestProcessor
{
public:
	RequestProcessor(const uint8_t version, const uint16_t code, const uint32_t payloadSize, const char* payload, const char clientID[CLIENT_ID_LEN] = "0");
	~RequestProcessor();

	std::vector<char> serializeResponse() const;
	std::string responseToString() const;

private:
	char _clientID[CLIENT_ID_LEN];
	uint8_t _version;
	uint16_t _code;
	uint32_t _payloadSize;

	char* _payload;

};