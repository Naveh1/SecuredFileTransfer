#include "StringHelper.h"

#include <string>
#include <iomanip>
#include <osrng.h>


std::string StringHelper::string_to_hex(const std::string& in, const int len ) {
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


std::string StringHelper::myHexify(const unsigned char* buffer, unsigned int length)
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


std::string StringHelper::hexToString(const std::string& in) {
	std::string output;

	if ((in.length() % 2) != 0 || !isHex(in)) {
		//throw std::runtime_error("String is not valid length ...");
		std::cerr << "Invalid hex string" << std::endl;
		exit(0);
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

bool StringHelper::isHex(const std::string& str)
{
	for (auto& a : str)
		if (!std::isxdigit(a))
			return false;
	return true;
}