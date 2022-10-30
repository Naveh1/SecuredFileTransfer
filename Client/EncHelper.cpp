#include "EncHelper.h"

#include "Base64Wrapper.h"
#include "RSAWrapper.h"
#include "AESWrapper.h"


std::string EncHelper::decAESKey(const UserData& userData, const passedKey& key)
{
	RSAPrivateWrapper pKey(userData.privateKey);

	try {
		return pKey.decrypt(key.key, key.len);
	}
	catch (const std::exception& e) {
		std::cout << "Decryption error: " << e.what() << std::endl;
		return "";
	}
}


std::string EncHelper::encAES(const std::string& key, const std::string& content)
{
	AESWrapper eKey((unsigned char*)key.c_str(), key.size());

	return eKey.encrypt(content.c_str(), content.size());
}
