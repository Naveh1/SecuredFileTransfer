#include "RequestProcessor.h"
#include "ResponseProcessor.h"
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

std::vector<char> RequestProcessor::serializeResponse(const bool nullTerminator) const
{
    std::string str = responseToString(nullTerminator);
    std::vector<char> bytes(str.begin(), str.end());
    bytes.push_back('\0');
    return bytes;
}


std::string RequestProcessor::responseToString(const bool nullTerminator) const
{
    int offset = nullTerminator ? 1 : 0;
    std::string res = padString(std::string(_clientID), CLIENT_ID_LEN) + numberToBytes<uint8_t>(_version, VERSION_LEN);
    res += padString(numberToBytes<uint16_t>(_code, CODE_LEN), CODE_LEN);
    res += padString(numberToBytes<uint32_t>(_payloadSize, PAYLOAD_SIZE), PAYLOAD_SIZE);
    return res + padString(std::string(_payload), _payloadSize - offset);
}


template <typename T>
std::string RequestProcessor::numberToBytes(const T number, const int len)
{
    char* resChar = new char[len + 1];
    resChar[len] = '\0';

    for (int i = 0; i < len; i++)
        resChar[i] = (uint8_t)(number >> i * BYTE);

    std::string res(resChar);
    delete[] resChar;

    return res;
}
