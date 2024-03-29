#pragma once
#include <iostream>
#include <Vector>


#define BYTE 8
#define CONTENT_SIZE_SIZE 4

//#define REGISTRATION_SUCCESS 2100
//#define REGISTRATION_FAIL 2101
//#define SENT_AES 2102
//#define GOT_FILE_WITH_CRC 2103
//#define RECEIVED_APPROVAL 2104

#define AES_KEY_LEN 16
#define ENC_AES_KEY_LEN 128

enum responseProcessor { REGISTRATION_SUCCESS = 2100 , REGISTRATION_FAIL = 2101, SENT_AES = 2102, GOT_FILE_WITH_CRC = 2103, RECEIVED_APPROVAL = 2104};


class ResponseProcessor
{
public:
	ResponseProcessor(const char* resp);			//deserialization c'tor
	ResponseProcessor(const uint8_t _version, const uint16_t _code, const uint32_t _payloadSize, const char* _payload);

	~ResponseProcessor();

	uint16_t getCode() const;
	const char* getPayload() const;
	uint32_t getPayloadSize() const;
	
	void processResponse(void* res = NULL) const;

private:
	uint8_t _version;
	uint16_t _code;
	uint32_t _payloadSize;

	char* _payload;

	template <typename T>
	static T deserialize(const char* buffer, const unsigned int len = sizeof(T));
};