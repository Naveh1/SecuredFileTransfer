#include "Helper.h"
#include "StringHelper.h"

#include <fstream>

#include "Base64Wrapper.h"
#include "RSAWrapper.h"
//#include "AESWrapper.h"

#include "RequestProcessor.h"

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

bool Helper::existsTest(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}


void Helper::createInfoFile(const std::string& name, const std::string& ID)
{
	// 1. Create an RSA decryptor. this is done here to generate a new private/public key pair
	RSAPrivateWrapper rsapriv;

	// 2. get the public key
	//std::string pubkey = rsapriv.getPublicKey();	// you can get it as std::string ...

	//hexify(pubkeybuff, RSAPublicWrapper::KEYSIZE);
	std::ofstream infoFile(INFO_FILE);
	if (!infoFile)
	{
		std::cerr << "Couldn't open/create info file for writing at: " << INFO_FILE << std::endl;
		exit(0);
	}
	infoFile << name << std::endl;
	//infoFile << std::hex << ID;
	//std::cout << std::endl << string_to_hex(ID);
	infoFile << StringHelper::string_to_hex(ID, REGISTRATION_RESPONSE_PAYLOAD);
	//for (const char& a : ID)
	//	infoFile << std::hex << a;
	infoFile << std::endl << Base64Wrapper::encode(rsapriv.getPrivateKey());
}


InfoFileData Helper::setupUserData()
{
	std::string line, ip, name, file;

	std::ifstream infoFile = std::ifstream(TRANSFER_INFO_FILE);
	if (!infoFile)
	{
		std::cerr << "Couldn't file transfer info file! aborting" << std::endl;
		exit(0);
	}
	std::cout << "Read file and register" << std::endl;
	//getting ip
	getline(infoFile, ip, ':');

	boost::system::error_code ec;
	boost::asio::ip::address::from_string(ip, ec);
	if (ec)
	{
		std::cerr << "Invalid ip: " << ec.message() << std::endl;
		exit(0);
	}

	//getting port
	getline(infoFile, line);
	int port = std::atoi(line.c_str());

	if (port < MIN_PORT || port > MAX_PORT)
	{
		std::cerr << "Invalid port numner" << std::endl;
		exit(0);
	}

	//getting username
	getline(infoFile, name);

	if (name.size() > NAME_LEN_IN_FILE)
	{
		std::cerr << "User name is too long" << std::endl;
		exit(0);
	}

	//getting file name
	getline(infoFile, file);

	if (!existsTest(file))
	{
		std::cerr << "Error opening file" << std::endl;
		exit(0);
	}

	return { ip, port, name, file };
}


UserData Helper::processInfoFile()
{

	std::string name, id, privateKey, line;
	std::ifstream infoFile = std::ifstream(INFO_FILE);
	if (!infoFile)
	{
		std::cerr << "Error opening information file" << std::endl;
		exit(0);
	}

	//infoFile >> name;
	std::getline(infoFile, name);
	infoFile >> id;

	if (id.size() != CLIENT_ID_LEN * HEX_FACTOR || !StringHelper::isHex(id))
	{
		std::cerr << "Illegal id in info file" << std::endl;
		exit(0);
	}
	infoFile >> line;
	//while (std::getline(infoFile, line))
	//	privateKey += line;
	//infoFile >> privateKey;
	return { name, StringHelper::hexToString(id), Base64Wrapper::decode(privateKey) };
	//return { name, hexToString(id.substr(0, CLIENT_ID_LEN * 2)), Base64Wrapper::decode(privateKey) };
}