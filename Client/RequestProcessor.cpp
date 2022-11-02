#include "RequestProcessor.h"
#include "ResponseProcessor.h"
#include <string>

#define REG_REQ_CODE 1100

//Padding string so it will be in the right length for the protocol
std::string padString(const std::string& str, const int len)
{
    if (len - str.size() <= 0)
        return str;

    try {
        return str + std::string(len - str.size(), '\0');
    }
    catch (const std::exception& e) {
        return str;
    }
}

//Processing request data and making it ready for serialization
RequestProcessor::RequestProcessor(const uint8_t version, const uint16_t code, const uint32_t payloadSize, const char* payload, const char clientID[CLIENT_ID_LEN]) : _version(version), _code(code), _payloadSize(payloadSize)
{
    memcpy_s(_clientID, CLIENT_ID_LEN, clientID, CLIENT_ID_LEN);

	_payload = new char[payloadSize];
    memcpy_s(_payload, payloadSize, payload, payloadSize);
}

//D'tor: Deleting dynamic memory
RequestProcessor::~RequestProcessor()
{
    delete[] _payload;
}

//Serializing reponse so it will be ready to get sent to the server
std::vector<char> RequestProcessor::serializeResponse() const
{
    std::string str = requestToString();
    std::vector<char> bytes(str.begin(), str.end());
    //bytes.push_back('\0');
    return bytes;
}

//Taking the request data and making it to on string 
std::string RequestProcessor::requestToString() const
{
    //int offset = nullTerminator ? 1 : 0;
    std::string res = padString(std::string(_clientID, CLIENT_ID_LEN), CLIENT_ID_LEN) + numberToBytes<uint8_t>(_version, VERSION_LEN);
    res += padString(numberToBytes<uint16_t>(_code, CODE_LEN), CODE_LEN);
    res += padString(numberToBytes<uint32_t>(_payloadSize, PAYLOAD_SIZE), PAYLOAD_SIZE);

    if(_code == REG_REQ_CODE)
        return res + padString(std::string(_payload), _payloadSize/* - offset*/);
    else
        return res + padString(std::string(_payload, _payloadSize), _payloadSize/* - offset*/);
}

//Moving a number into bytes string for sending in binary format
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

//Creating file sending request payload from data
char* RequestProcessor::getFilePayload(const char* ID, const std::string& fileName, const std::string& fileContent)
{
    std::string res = padString(std::string(ID, CLIENT_ID_LEN), CLIENT_ID_LEN) + padString(numberToBytes<uint32_t>(fileContent.size(), CONTENT_SIZE_SIZE), CONTENT_SIZE_SIZE);
    //res += padString(fileName, FILE_NAME_LEN) + std::string(fileContent, fileContent.size());
    res += padString(fileName, FILE_NAME_LEN) + fileContent;

    char* tmp = new char[res.size()];

    memcpy_s(tmp, res.size(), res.c_str(), res.size());

    return tmp;
}

//Creating crc request payload from data
char* RequestProcessor::getCrcPayload(const char* ID, const std::string& fileName)
{
    std::string res = padString(std::string(ID, CLIENT_ID_LEN), CLIENT_ID_LEN) + padString(fileName, FILE_NAME_LEN);

    char* tmp = new char[res.size()];

    memcpy_s(tmp, res.size(), res.c_str(), res.size());

    return tmp;
}