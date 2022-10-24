#include "Client.h"

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


bool existsTest(const std::string& name) {
	std::ifstream f(name.c_str());
	return f.good();
}


std::string myHexify(const unsigned char* buffer, unsigned int length)
{
	std::stringstream ss;
	std::ios::fmtflags f(ss.flags());
	ss << std::hex;
	for (size_t i = 0; i < length; i++)
		ss << std::setfill('0') << std::setw(2) << (0xFF & buffer[i]);
		//ss << std::setfill('0') << std::setw(2) << (0xFF & buffer[i]) << (((i + 1) % 16 == 0) ? "\n" : " ");
	//ss << std::endl;
	ss.flags(f);

	return ss.str();
}


void createInfoFile(const std::string& name, const std::string& ID) 
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
	infoFile << string_to_hex(ID, REGISTRATION_RESPONSE_PAYLOAD);
	//for (const char& a : ID)
	//	infoFile << std::hex << a;
	infoFile << std::endl << Base64Wrapper::encode(rsapriv.getPrivateKey());
}


void connect(tcp::socket& s, tcp::resolver& resolver, const InfoFileData& data)
{
	Sleep(5000);

	std::cout << "Connecting to " << data.ip << " on port " << std::to_string(data.port) << std::endl;

	try {
		boost::asio::connect(s, resolver.resolve(data.ip, std::to_string(data.port)));
	}
	catch (std::exception& e) {
		std::cerr << "Error connecting: " << e.what() << std::endl;
		exit(0);
	}
}


char* request(tcp::socket& s, const std::vector<char>& req)
{
	char* reply = new char[MAX_REPLY_LEN];
	size_t reply_len = 0;
	try {
		boost::asio::write(s, boost::asio::buffer(req));

		//reply_len = boost::asio::read(s, boost::asio::buffer(reply, MAX_REPLY_LEN));
		reply_len = s.read_some(boost::asio::buffer(reply, MAX_REPLY_LEN));

	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		exit(0);
	}

	return reply;
}


UserData registerUser(tcp::socket & s, const InfoFileData& infoData)
{

	//send registration request
	RequestProcessor req(VERSION, REGISTRATION, NAME_LEN, infoData.name.c_str());
	/*auto requ = req.serializeResponse(true);
	std::cout << "len: "  << requ.size() << std::endl;
	for (char r : requ)
		std::cout << r;
	std::cout << std::endl;*/

	//std::string reply(MAX_REPLY_LEN, '\0');

	char* reply = request(s, req.serializeResponse());

	//std::cout << std::string(reply) << std::endl;		//debug
	ResponseProcessor resp(reply);

	resp.processResponse();
	if (resp.getCode() == REGISTRATION_SUCCESS) {
		std::cout << "ID: " << resp.getPayload() << std::endl;		//debug

		for (const char& i : std::string(resp.getPayload()))
			std::cout << std::hex << (uint8_t)i;				//debug
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


passedKey sendKey(tcp::socket& s, const UserData& userData)
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
	char* reply = request(s, req.serializeResponse());


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


InfoFileData setupUserData()
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

	/*if (!existsTest(file))
	{
		std::cerr << "Error opening file" << std::endl;
		exit(0);
	}*/ //deleting for debug

	return {ip, port, name, file};
}


std::string string_to_hex(const std::string& in, const int len = -1) {
	//std::stringstream ss;
	size_t stop = len;
	if (len == -1)
		stop = in.length();
	//for (size_t i = 0; i < stop; ++i) {
	//	ss << std::setw(2) << static_cast<unsigned int>(static_cast<unsigned char>(in[i]));
	//}

	//return ss.str();

	return myHexify((const unsigned char*)in.c_str(), stop);
}



std::string hexToString(const std::string& in) {
	std::string output;

	if ((in.length() % 2) != 0) {
		throw std::runtime_error("String is not valid length ...");
	}

	size_t cnt = in.length() / 2;

	for (size_t i = 0; cnt > i; ++i) {
		uint32_t s = 0;
		std::stringstream ss;
		ss << std::hex << in.substr(i * 2, 2);
		ss >> s;

		output.push_back(static_cast<unsigned char>(s));
	}

	return output;
}


UserData processInfoFile() 
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

	while (std::getline(infoFile, line))
		privateKey += line;
	//infoFile >> privateKey;
	return { name, hexToString(id), Base64Wrapper::decode(privateKey) };
	//return { name, hexToString(id.substr(0, CLIENT_ID_LEN * 2)), Base64Wrapper::decode(privateKey) };
}


std::string decAESKey(const UserData& userData, const passedKey& key)
{
	RSAPrivateWrapper pKey(userData.privateKey);
	
	try {
		return pKey.decrypt(key.key, key.len);
	}
	catch (const std::exception& e){
		std::cout << "Decryption error: " << e.what() << std::endl;
		return "";
	}
}


std::string getFileContent(const std::string& path)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);

	if (!file) {
		std::cerr << "Error reading file" << std::endl;
		exit(0);
	}

	std::string data;

	std::getline(file, data, '\0');

	return data;
}


std::string encAES(const std::string& key, const std::string& content)
{
	AESWrapper eKey((unsigned char*)key.c_str(), key.size());

	return eKey.encrypt(content.c_str(), content.size());
}


uint32_t sendFile(tcp::socket& s, const UserData& userData, const std::string& fileName, const std::string& encFileContent)
{
	//SEND_FILE
	char* payload = RequestProcessor::getFilePayload(userData.userId.c_str(), fileName, encFileContent);
	RequestProcessor req((uint8_t)VERSION, (uint16_t)SEND_FILE, (uint32_t)(CLIENT_ID_LEN + CONTENT_SIZE_SIZE
		+ FILE_NAME_LEN + encFileContent.size()), payload, userData.userId.c_str());

	char* reply = request(s, req.serializeResponse());

	delete payload;

	ResponseProcessor resp(reply);
	uint32_t crc = 0;

	resp.processResponse(&crc);

	return crc;
}


int main() 
{
	boost::asio::io_context io_context;
	tcp::socket s(io_context);
	tcp::resolver resolver(io_context);
	UserData userData;
	InfoFileData infoData = setupUserData();
	
	//file = setupServer();

	connect(s, resolver, infoData);

	//std::ifstream infoFile(INFO_FILE);
	//if (!infoFile)
	if(!existsTest(INFO_FILE))
	{
		userData = registerUser(s, infoData);

		createInfoFile(userData.userName, userData.userId);
	}

	userData = processInfoFile();

	passedKey aesKey = sendKey(s, userData);

	std::string AESkey = decAESKey(userData, aesKey);

	std::cout << string_to_hex(AESkey, AESkey.size()) << std::endl;

	std::string content = getFileContent(infoData.file);

	CRC crcCalc = CRC();
	crcCalc.update((unsigned char*)content.c_str(), content.size());
	uint32_t crc = crcCalc.digest();

	std::string encContent = encAES(AESkey, content);

	uint32_t resCrc = 0, times = 0;
	
	do {
		std::cout << "Sending file for the " << times + 1 << " time." << std::endl;
		resCrc = sendFile(s, userData, infoData.file, encContent);
	} while (resCrc != crc && ++times < FILE_SENDING_TIMES);

	if (resCrc == crc)
		std::cout << "CRC_OK packet" << std::endl;
	else
		std::cout << "ABORT message packet" << std::endl;
	
	delete aesKey.key;

	return 0;
}