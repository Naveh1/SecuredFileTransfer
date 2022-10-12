#pragma once
#include <iostream>
#include <Vector>

class ResponseProcessor
{
public:
	ResponseProcessor(const char* resp);			//deserialization d'tor
	ResponseProcessor(const uint8_t _version, const uint16_t _code, const uint32_t _payloadSize, const char* _payload);

private:
	uint8_t _version;
	uint16_t _code;
	uint32_t _payloadSize;

	char* _payload;
};