#pragma once
#include <string>

// 文字コードゆーてぃりてぃ
namespace StringUtility {
std::wstring ConvertString(const std::string& str);
std::string ConvertString(const std::wstring& str);
};