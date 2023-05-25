#ifndef UTILS_INC
#define UTILS_INC
#pragma once

#include <string>
#include <list>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <cstring>
#include <iostream>
#include <regex>

enum class TimeFormat {
	ISO8601, // 格式为 YYYY-MM-DDTHH:mm:ss
	RFC2822, // 格式为 DDD, DD MMM YYYY HH:mm:ss +0000
	Custom // 自定义格式
};

struct DateTime
{
    int m_tm_us;
    int m_tm_ms;
    int m_tm_sec;   // seconds after the minute - [0, 60] including leap second
    int m_tm_min;   // minutes after the hour - [0, 59]
    int m_tm_hour;  // hours since midnight - [0, 23]
    int m_tm_mday;  // day of the month - [1, 31]
    int m_tm_mon;   // months since January - [0, 11]
    int m_tm_year;  // years since 1900
};      

template<typename T>
int compareValue(T a, T b, T epsilon = static_cast<T>(1e-6))
{
	if (std::fabs(a - b) < epsilon)
		return 0;
	else if (a > b)
		return 1;
	else
		return -1;
}

float getLinePixelDistanse(float start_x, float start_y, float end_x, float end_y);

float getEllipsePixelCircle(float minor, float major);

std::string UTF8ToGB(const char *str);
std::string GBToUTF8(const char *str);

std::list<std::string> splitStr(char ch, const std::string& str);

//获取本地时间与UTC时间的时差
int getTimeDifferenceOfUtc();
// 将DICOM日期和时间转换为long long类型的微妙时间
long long datetimeToMicroSec(const std::string &dateStr, const std::string &timeStr);
DateTime microSecToDt(long long micro_time);
// 将时间戳转换为字符串
std::string getFormatTimeStr(const std::time_t timestamp, const TimeFormat format = TimeFormat::ISO8601, const std::string custom_format = "");
uint64_t getCurrentMicroSecTime();

std::string getCurrentUtcDate();
std::string getCurrentLocalDate();
bool isLeapYear(int year);
int countLeapYears(int start_year, int end_year);

bool fileExist(const std::string &file_path);
bool createDirectory(const std::string dir_path);
bool getCurrentWorkDir(std::string& pwd);
bool dirExist(const std::string& dir_path);
bool isFileInUse(const wchar_t* filename);
bool deleteFile(const std::string &file_path);
bool cancelFileRdOnlyAttibute(const std::string &file_path);

std::string getFileNameByPath(const std::string& file_path);

//暂时支持日期格式为20230403的字符串判断
bool isValidDate(const std::string& dateStr);

//不能整除的天数，只要大于4天，周数就+1
int getWeeksBetweenDates(const std::string& start_date_str, const std::string& end_date_str);

std::string createUuid();

// get cpu
std::string getCpu();
// get disk
std::string getDisk();
// get disks
std::list<std::string> getDisks();

int encryptAES(const std::string &str_source, const std::string &str_key, std::string &result);

int decryptAES(const std::string& str_source, const std::string& str_key, std::string& result);

int encryptCBC(const std::string& str_source, const std::string& str_key, const std::string &vi, std::string& result);

int decryptCBC(const std::string& str_source, const std::string& str_key, const std::string& vi, std::string& result);

std::string wstring2String(const std::wstring& ws);
std::wstring string2WString(const std::string& s);
#endif

