#ifndef MYSTR_H
#define MYSTR_H

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <set>

// 判断字符串是不是数字
bool isNum(std::string str)
{
	std::stringstream sin(str);
	double d;
	char c;
	if (!(sin >> d))
		return false;
	if (sin >> c)
		return false;
	return true;
}

//模板函数：将string类型变量转换为常用的数值类型（此方法具有普遍适用性）  
template <class Type>
Type stringToNum(const std::string& str)
{
	std::istringstream iss(str);
	Type num;
	iss >> num;
	return num;
}

std::vector<std::string> split2(const  std::string& s, const std::string& delim)
{
	std::vector<std::string> elems;
	size_t pos = 0;
	size_t len = s.length();
	size_t delim_len = delim.length();
	if (delim_len == 0) return elems;
	while (pos < len)
	{
		int find_pos = s.find(delim, pos);
		if (find_pos < 0)
		{
			elems.push_back(s.substr(pos, len - pos));
			break;
		}
		elems.push_back(s.substr(pos, find_pos - pos));
		pos = find_pos + delim_len;
	}
	return elems;
}

std::vector<std::string> split(const  std::string& s)
{
	std::string strTest = s;
	std::string::iterator p = strTest.begin();
	std::vector<std::string> elems;
	std::string sepword;

	bool bTrue = false;
	while (p != strTest.end())
	{
		if (*p == ' ' || *p == '\t' || *p == ';')
		{
			if (bTrue)
			{
				elems.push_back(sepword);
				sepword.clear();
			}
			bTrue = false;
		}
		else
		{
			bTrue = true;
			sepword += *p;
		}

		p++;

		if (p == strTest.end() || *p == '\0' || *p == '\n')
		{
			if (bTrue)
			{
				elems.push_back(sepword);
				sepword.clear();
			}
			bTrue = false;
			break;
		}
	}
	return elems;
}

void trim(std::string &str)
{
	int s = str.find_first_not_of(" \t");
	int e = str.find_last_not_of(" \t");
	str = str.substr(s, e - s + 1);
}

int strvec_to_map(std::vector<std::string> strvec, std::map<std::string, int> *to_map){
	if (to_map == NULL || strvec.size() == 0)
	{
		return 1;
	}
	to_map->clear();
	std::set<std::string> myset = std::set<std::string>(strvec.begin(), strvec.end());
	int i = 0;
	for each (std::string var in myset)
	{
		(*to_map).insert(std::pair<std::string, int>(var, i));
		i++;
	}
	return 0;
}

int str_to_map(std::string str, std::map<std::string, int> *to_map){
	if (to_map == NULL)
	{
		return 1;
	}
	std::vector<std::string> strvec;
	// 去除两端的空格
	trim(str);
	// 分割字符串
	strvec = split(str);
	// 将字符串列表转换成map
	strvec_to_map(strvec, to_map);
	return 0;
}
#endif


