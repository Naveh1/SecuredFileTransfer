#include "ResponseProcessor.h"
#include "RequestProcessor.h"

//Turning bytes string into a readable data
template <typename T>
T ResponseProcessor::deserialize(const char* buffer, const unsigned int len)
{
    T res = 0;

    for (int i = 0; i < len; i++)
        res |= (uint8_t)buffer[i] << i * BYTE;

    return res;
}

//bytes string C'tor
ResponseProcessor::ResponseProcessor(const char* resp) : ResponseProcessor(deserialize<uint8_t>(resp, VERSION_LEN), 
    deserialize<uint16_t>(resp + VERSION_LEN, CODE_LEN), deserialize<uint32_t>(resp + VERSION_LEN + CODE_LEN, PAYLOAD_SIZE), 
    resp + VERSION_LEN + CODE_LEN + PAYLOAD_SIZE)
{
}


//C'tor
ResponseProcessor::ResponseProcessor(const uint8_t version, const uint16_t code, const uint32_t payloadSize, const char* payload) : _version(version), _code(code), _payloadSize(payloadSize)
{
    if (!payloadSize) {
        _payload = nullptr;
        return;
    }
	_payload = new char[payloadSize];
    //_payload = std::string(payloadSize, '\0').c_str();
	//strncpy_s(_payload, payloadSize, payload, payloadSize);

    memcpy(_payload, payload, payloadSize);
}

uint16_t ResponseProcessor::getCode() const
{
    return _code;
}

const char* ResponseProcessor::getPayload() const
{
    return _payload;
}

uint32_t ResponseProcessor::getPayloadSize() const
{
    return _payloadSize;
}

//Processing response from data according to code
void ResponseProcessor::processResponse(void* res) const
{
    switch (_code) 
    {
    case REGISTRATION_SUCCESS:
        std::cout << "Registration succeeded!" << std::endl;
        break;
    case REGISTRATION_FAIL:
        std::cerr << "Registration failed!" << std::endl;
        break;
    case SENT_AES:
        memcpy_s(res, ENC_AES_KEY_LEN, getPayload() + CLIENT_ID_LEN, ENC_AES_KEY_LEN);
        break;
    case GOT_FILE_WITH_CRC:
        *((uint32_t*)res) = deserialize<uint32_t>(getPayload() + CLIENT_ID_LEN + CONTENT_SIZE_SIZE + FILE_NAME_LEN);
        break;
    default:
        std::cerr << "Unsupported code: " << _code << std::endl;
    }
}

//D'tor: deleting dynamic data
ResponseProcessor::~ResponseProcessor()
{
    delete[] _payload;
}
