#pragma once
#include <iostream>

class StringHelper
{
public:
	static std::string string_to_hex(const std::string& in, const int len = -1);
	static std::string myHexify(const unsigned char* buffer, unsigned int length);
	static std::string hexToString(const std::string& in);
	static bool isHex(const std::string& str);
};