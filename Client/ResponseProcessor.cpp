#include "ResponseProcessor.h"
#include "RequestProcessor.h"


template <typename T>
T ResponseProcessor::deserialize(const char* buffer, const unsigned int len)
{
    unsigned char toCast = 0;
    for (int i = 0; i < len; i++)
        toCast |= buffer[i] << (len - 1 - i) * BYTE;
    return static_cast<T>(toCast);
}

ResponseProcessor::ResponseProcessor(const char* resp) : ResponseProcessor(deserialize<uint8_t>(resp, VERSION_LEN), 
    deserialize<uint16_t>(resp + VERSION_LEN, CODE_LEN), deserialize<uint32_t>(resp + VERSION_LEN + CODE_LEN, PAYLOAD_SIZE), 
    resp + VERSION_LEN + CODE_LEN + PAYLOAD_SIZE)
{
}



ResponseProcessor::ResponseProcessor(const uint8_t version, const uint16_t code, const uint32_t payloadSize, const char* payload) : _version(version), _code(code), _payloadSize(payloadSize)
{
	_payload = new char[payloadSize];
	strncpy_s(_payload, payloadSize, payload, payloadSize);
}

uint16_t ResponseProcessor::getCode() const
{
    return _code;
}

void ResponseProcessor::processResponse(char* res) const
{
    switch (_code) 
    {
    case REGISTRATION_SUCCESS:
        std::cout << "Registration succeeded!" << std::endl;
        res = _payload;
        break;
    case REGISTRATION_FAIL:
        std::cerr << "Registration failed!" << std::endl;
        break;
    default:
        std::cerr << "Unsupported code: " << _code << std::endl;
    }
}

ResponseProcessor::~ResponseProcessor()
{
    delete[] _payload;
}
