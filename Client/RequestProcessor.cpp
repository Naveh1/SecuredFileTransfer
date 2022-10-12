#include "RequestProcessor.h"
#include <string>


RequestProcessor::RequestProcessor(const uint8_t version, const uint16_t code, const uint32_t payloadSize, const char* payload, const char clientID[CLIENT_ID_LEN]) : _version(version), _code(code), _payloadSize(payloadSize)
{
	strncpy_s(_clientID, clientID, CLIENT_ID_LEN);

	_payload = new char[payloadSize];
	strncpy_s(_payload, payloadSize, payload, payloadSize);
}

RequestProcessor::~RequestProcessor()
{
    delete[] _payload;
}

std::vector<char> RequestProcessor::serializeResponse() const
{
    std::string str = responseToString();
    std::vector<char> bytes(str.begin(), str.end());
    bytes.push_back('\0');
    return bytes;
}

std::string padString(const std::string& str, const int len)
{
    return std::string((len - str.length()), '0').append(str);
}

std::string RequestProcessor::responseToString() const
{
    /*
    std::string unpaddedCode = std::to_string(_code);
    std::string paddedCode = std::string( (CODE_LEN - unpaddedCode.length() ), '0').append(unpaddedCode);
    *//*
    std::string unpaddedSize = std::to_string(_payloadSize);
    std::string paddedPayloadSize = std::string((PAYLOAD_SIZE - unpaddedSize.length()), '0').append(unpaddedSize);*/

    return std::string(_clientID) + std::to_string(_version) + padString(std::to_string(_code), CODE_LEN)
        + padString(std::to_string(_payloadSize), PAYLOAD_SIZE) + padString(std::string(_payload), _payloadSize);
}
