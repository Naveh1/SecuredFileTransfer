#include "RequestProcessor.h"
#include <string>

RequestProcessor::RequestProcessor(const uint8_t version, const uint16_t code, const uint32_t payloadSize, const char* payload, const char clientID[CLIENT_ID_LEN]) : _version(version), _code(code), _payloadSize(payloadSize)
{
	strncpy(_clientID, clientID, CLIENT_ID_LEN);

	_payload = new char[payloadSize];
	strcpy(_payload, payload);
}

std::vector<char> RequestProcessor::serializeResponse() const
{
    std::string str = responseToString();
    std::vector<char> bytes(str.begin(), str.end());
    bytes.push_back('\0');
    return bytes;
}

std::string RequestProcessor::responseToString() const
{
    return std::string(_clientID) + std::to_string(_version) + std::to_string(_code) + std::to_string(_payloadSize) + std::string(_payload);
}
