﻿#include "Client.h"

#include <string>
#include <fstream>
#include "RequestProcessor.h"
#include "ResponseProcessor.h"
#include "crc.h"
//#include <cryptopp/rsa.h>

#include <iomanip>

#include "Base64Wrapper.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"

#include "SockHandler.h"
#include "StringHelper.h"
#include "EncHelper.h"



UserData registerUser(SockHandler& sock, const InfoFileData& infoData)
{

	//send registration request
	RequestProcessor req(VERSION, REGISTRATION, NAME_LEN, infoData.name.c_str());
	/*auto requ = req.serializeResponse(true);
	std::cout << "len: "  << requ.size() << std::endl;
	for (char r : requ)
		std::cout << r;
	std::cout << std::endl;*/

	//std::string reply(MAX_REPLY_LEN, '\0');

	char* reply = sock.request(req.serializeResponse());

	//std::cout << std::string(reply) << std::endl;		//debug
	ResponseProcessor resp(reply);

	resp.processResponse();
	if (resp.getCode() == REGISTRATION_SUCCESS) {
		//std::cout << "ID: " << resp.getPayload() << std::endl;		//debug
		std::cout << "";
		//for (const char& i : std::string(resp.getPayload()))
		//	std::cout << std::hex << (uint8_t)i;				//debug
	}
	else  if (resp.getCode() == REGISTRATION_FAIL) {
		std::cerr << "Registration failed" << std::endl;
		exit(0);
	}
	else
		std::cerr << "Unknown code" << std::endl;

	delete reply;

	return {infoData.name, resp.getPayload(), ""};
}


passedKey sendKey(SockHandler& sock, const UserData& userData)
{
	RSAPrivateWrapper pKey(userData.privateKey);
	//std::cout << string_to_hex(pKey.getPublicKey(), -1) << std::endl;
	//std::cout << "public key size: " << pKey.getPublicKey().size() << std::endl;
	//std::string name = RequestProcessor::padName(userData.userName);
	char payload[NAME_LEN + PUBLICKEY_LEN] = { '\0' };
	memcpy(payload, userData.userName.c_str(), userData.userName.size());
	memcpy(payload + NAME_LEN, pKey.getPublicKey().c_str(), PUBLICKEY_LEN);
	//std::cout << payload;
	//std::cout << pKey.getPublicKey();
	//std::cout << "Public key: " << string_to_hex(pKey.getPublicKey(), -1) << std::endl;

	RequestProcessor req((uint8_t)VERSION, (uint16_t)SEND_PUBLIC_KEY, (uint32_t)(NAME_LEN + PUBLICKEY_LEN), payload, userData.userId.c_str());
	//std::cout << string_to_hex(req.getPayload(), 415);
	//auto a = req.serializeResponse();
	//std::cout << std::endl << std::endl << string_to_hex(std::string(a.begin(), a.end()).c_str(), a.size()) << '\t' << a.size() << std::endl;
	//RequestProcessor req((uint8_t)VERSION, (uint16_t)SEND_PUBLIC_KEY, (uint32_t)(NAME_LEN + PUBLICKEY_LEN), (name + pKey.getPublicKey()).c_str(), userData.userId.c_str());
	char* reply = sock.request(req.serializeResponse());


	ResponseProcessor resp(reply);
	char aesKey[ENC_AES_KEY_LEN] = { '\0' };
	resp.processResponse(aesKey);

	delete reply;

	//std::cout << "Enc aesKey: " << string_to_hex(std::string(aesKey, 128), 128) << std::endl;
	//std::cout << "Enc aesKey len: " << aesKey << '\t' << strlen(aesKey) << std::endl;
	char* aesDynamic = new char[ENC_AES_KEY_LEN];
	memcpy_s(aesDynamic, ENC_AES_KEY_LEN, aesKey, ENC_AES_KEY_LEN);
	return { aesDynamic,  ENC_AES_KEY_LEN };
}


std::string getFileContent(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);

	if (!file) {
		std::cerr << "Error reading file" << std::endl;
		exit(0);
	}

	//std::string data;
	//  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	//  std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	return data;
}


uint32_t sendFile(SockHandler& sock, const UserData& userData, const std::string& fileName, const std::string& encFileContent)
{
	//SEND_FILE
	char* payload = RequestProcessor::getFilePayload(userData.userId.c_str(), fileName, encFileContent);
	RequestProcessor req((uint8_t)VERSION, (uint16_t)SEND_FILE, (uint32_t)(CLIENT_ID_LEN + CONTENT_SIZE_SIZE
		+ FILE_NAME_LEN + encFileContent.size()), payload, userData.userId.c_str());

	char* reply = sock.request(req.serializeResponse());

	delete payload;

	ResponseProcessor resp(reply);
	uint32_t crc = 0;

	resp.processResponse(&crc);

	return crc;
}

bool crcReq(SockHandler& sock, const UserData& userData, const std::string& fileName, const uint16_t code)
{
	char* payload = RequestProcessor::getCrcPayload(userData.userId.c_str(), fileName);
	RequestProcessor req((uint8_t)VERSION, (uint16_t)code, (uint32_t)(CLIENT_ID_LEN + FILE_NAME_LEN), payload, userData.userId.c_str());

	char* reply = sock.request(req.serializeResponse());

	delete payload;

	ResponseProcessor resp(reply);
	if (resp.getCode() != RECEIVED_APPROVAL) {
		std::cerr << "Wrong response" << std::endl;
		return false;
	}

	return true;
}


int main() 
{
	Sleep(5000); //Giving the server time to turn on - debug
	UserData userData;
	InfoFileData infoData = Helper::setupUserData();
	SockHandler sockHandler(infoData.port, infoData.ip);
	
	//file = setupServer();

	//connect(s, resolver, infoData);

	//std::ifstream infoFile(INFO_FILE);
	//if (!infoFile)
	if(!Helper::existsTest(INFO_FILE))
	{
		userData = registerUser(sockHandler, infoData);

		Helper::createInfoFile(userData.userName, userData.userId);
	}

	userData = Helper::processInfoFile();

	passedKey aesKey = sendKey(sockHandler, userData);

	std::string AESkey = EncHelper::decAESKey(userData, aesKey);

	//std::cout << string_to_hex(AESkey, AESkey.size()) << std::endl;

	std::string content = getFileContent(infoData.file);

	CRC crcCalc = CRC();
	crcCalc.update((unsigned char*)content.c_str(), content.size());
	uint32_t crc = crcCalc.digest();

	std::string encContent = EncHelper::encAES(AESkey, content);
	//encContent = content; // debug
	uint32_t resCrc = 0, times = 0;

	//std::cout << string_to_hex(encContent) << std::endl;
	
	resCrc = sendFile(sockHandler, userData, infoData.file, encContent);

	while (resCrc != crc && ++times <= FILE_SENDING_TIMES) 
	{
		//std::cout << crc << "\t---\t" << resCrc << std::endl;
		if (!crcReq(sockHandler, userData, infoData.file, CRC_NOT_OK))
		{
			delete aesKey.key;
			return 0;
		}
		std::cout << "Sending file for the " << times + 1 << " time." << std::endl;
		resCrc = sendFile(sockHandler, userData, infoData.file, encContent);
	}

	uint16_t code = CRC_OK;
	if (resCrc == crc)
		std::cout << "CRC_OK packet" << std::endl;
	else
	{
		std::cerr << "crc was wrong too many times. ABORT message packet" << std::endl;
		code = CRC_ERROR;
	}

	crcReq(sockHandler, userData, infoData.file, code);
	delete aesKey.key;

	return 0;
}