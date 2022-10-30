#pragma once

#include "Helper.h"

class EncHelper
{
public:
	static std::string decAESKey(const UserData& userData, const passedKey& key);
	static std::string encAES(const std::string& key, const std::string& content);
};