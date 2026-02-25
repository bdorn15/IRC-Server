#include "Global.hpp"

/**
 * @brief Split string by delim into vector of things.
 */
std::vector<std::string> xsvSplit(const std::string string, const char delim)
{
	std::vector<std::string> tmpVec;
	size_t start = 0;
	size_t end = string.find(delim);

	while (end != std::string::npos)
	{
		tmpVec.emplace_back(string.substr(start, end - start));
		start = end + 1;
		end = string.find(delim, start);
	}
	tmpVec.emplace_back(string.substr(start));

	return (tmpVec);
}

/**
 * @brief Is a particular string in a CSV file.
 */
bool isInXSV(const std::string needle, const std::string string, const char delim)
{
	std::vector<std::string> tmpVec;
	tmpVec = xsvSplit(string, delim);

	if (std::find(tmpVec.begin(), tmpVec.end(), needle) != tmpVec.end())
		return (true);
	
	return (false);
}

