#include "utils.h"
#include <sstream>
#include <fstream>
#include <random>

#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
#include <Windows.h>
#include <process.h>
#include <direct.h>
#include <io.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <sys/stat.h>
#include <unistd.h>
#endif

#if defined(_WIN32) || defined(WIN32)|| defined(_WINDOWS)
#include <Wbemidl.h>
#include <comdef.h>
#include <intrin.h>
#include <array>
#pragma comment(lib, "wbemuuid.lib")
#else
#include <string.h>
#include <sstream>
#include <unistd.h>
#include <cpuid.h>
#include <sys/ioctl.h>
#include <linux/hdreg.h>
#include <sys/fcntl.h>
#endif

float getLinePixelDistanse(float start_x, float start_y, float end_x, float end_y)
{
	return sqrt(pow((end_x - start_x), 2) + pow((end_y - start_y), 2));
}

float getEllipsePixelCircle(float minor, float major)
{
	auto h = pow((major - minor) / (major + minor), 2);
	auto Q = (major + minor) / 2;
	auto L = Q * 3.1415926 * (1 + h / 4 + pow(h, 2) / pow(4.0, 3) + pow(h, 3) / pow(4.0, 4) + 25 * pow(h, 4) / 16384 + 49 * pow(h, 5) / 65536);
	return L;
}

std::string getFormatTimeStr(const std::time_t timestamp, const TimeFormat format, const std::string custom_format)
{
	std::stringstream ss;
	std::tm time_info; 
#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
	gmtime_s(&time_info, &timestamp); // 获取 UTC 时间
#elif defined(__linux__) || defined(__APPLE__)
	gmtime_r(&timestamp, &time_info);
#endif

	switch (format) {
	case TimeFormat::ISO8601:
		ss << std::put_time(&time_info, "%Y-%m-%dT%H:%M:%S");
		break;
	case TimeFormat::RFC2822:
		ss << std::put_time(&time_info, "%a, %d %b %Y %H:%M:%S +0000");
		break;
	case TimeFormat::Custom:
		ss << std::put_time(&time_info, custom_format.c_str());
		break;
	default:
		throw std::invalid_argument("Invalid time format");
	}

	return ss.str();
}

uint64_t getCurrentMicroSecTime()
{
	auto duration_since_epoch = std::chrono::system_clock::now().time_since_epoch(); // 从1970-01-01 00:00:00到当前时间点的时长
	auto microseconds_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(duration_since_epoch).count(); // 将时长转换为微秒数
	//microSecToStr(microseconds_since_epoch);

	//// 将微秒数转换为时间点
	//std::chrono::microseconds dur(microseconds_since_epoch);
	//auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds>(dur);

	//// 将时间点转换为可读的时间字符串
	//std::time_t t = std::chrono::system_clock::to_time_t(tp);
	//std::string time_str = std::ctime(&t);
	//std::cout << "时间：" << time_str;
	return microseconds_since_epoch;
}

std::string getCurrentUtcDate()
{
	std::time_t t = std::time(nullptr);

	// 将时间转换为 UTC 时间
	std::tm now ;
	#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
	char date[64] = { 0 };
	gmtime_s(&now, &t);
	sprintf_s(date, "%04d%02d%02d", now.tm_year + 1900 , now.tm_mon +1 , now.tm_mday);
#elif defined(__linux__) || defined(__APPLE__)
	char *date = nullptr;
	gmtime_r(&t, &now);
	asprintf(&date, "%04d%02d%02d", now.tm_year + 1900 , now.tm_mon +1 , now.tm_mday);
#endif		
	return date;
}

std::string getCurrentLocalDate()
{
	auto now = std::chrono::system_clock::now();

	std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);

	std::tm now_tm;
	#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
	localtime_s(&now_tm ,&now_time_t);
#elif defined(__linux__) || defined(__APPLE__)
	localtime_r(&now_time_t, &now_tm);
#endif

	int year = now_tm.tm_year + 1900; // 年份从 1900 年开始计算
	int month = now_tm.tm_mon + 1; // 月份从 0 开始计算
	int day = now_tm.tm_mday; // 当月的第几天
	int hour = now_tm.tm_hour; // 小时
	int minute = now_tm.tm_min; // 分钟
	int second = now_tm.tm_sec; // 秒

	// 使用字符串流进行格式化
	std::ostringstream oss;
	oss << setfill('0') << setw(4) << year << "-" << setw(2) << month << "-" << setw(2) << day 
		<< " " << setw(2) << hour << ":" << setw(2) << minute << ":" << setw(2) << second;

	// 获取格式化后的字符串
	return oss.str();
}

bool isLeapYear(int year)
{
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int countLeapYears(int start_year, int end_year)
{
	// 计算起始年份之前的闰年数量
	int count_before_start_year = (start_year - 1) / 4 - (start_year - 1) / 100 + (start_year - 1) / 400;
	// 计算结束年份的闰年数量
	int count_end_year = end_year / 4 - end_year / 100 + end_year / 400;
	// 计算总的闰年数量
	int leap_years = count_end_year - count_before_start_year;

	return leap_years;
}

bool fileExist(const std::string& file_path)
{
#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
	if (_access(file_path.c_str(), 0) == 0) {
		//std::cout << "File exists." << std::endl;
		return true;
	}
	else {
		//std::cout << "File does not exist." << std::endl;
		return false;
	}
#elif defined(__linux__) || defined(__APPLE__)
	if (access(file_path.c_str(), F_OK) == 0) {
		//std::cout << "File exists." << std::endl;
		return true;
	}
	else {
		if (errno == ENOENT) {
            std::cout <<"File does not exist"<<std::endl;
        } else {
            std::cout <<"access failed: "<<strerror(errno)<<std::endl;
        }
		return false;
	}
#endif
}

std::string getFileNameByPath(const std::string& file_path)
{
	if (file_path.empty())
		return file_path;

	if(!fileExist(file_path))
		return "";
	size_t pos = file_path.find_last_of("/\\"); // 查找最后一个斜杠或反斜杠的位置
	if (pos != std::string::npos) { // 如果找到了斜杠或反斜杠
		return file_path.substr(pos + 1); // 返回从斜杠或反斜杠后面的字符开始的子串，即文件名
	}
	else {
		return file_path; // 如果未找到斜杠或反斜杠，则整个路径就是文件名
	}
}

bool isValidDate(const std::string& dateStr)
{
	std::regex dateRegex("\\d{4}\\d{2}\\d{2}");
	if (!std::regex_match(dateStr, dateRegex)) {
		return false; // 日期格式不正确
	}
	
	int year = std::stoi(dateStr.substr(0, 4));
	int month = std::stoi(dateStr.substr(4, 2));
	int day = std::stoi(dateStr.substr(6, 2));

	if (month < 1 || month > 12) {
		return false; // 月份无效
	}

	int maxDay = 31;
	if (month == 4 || month == 6 || month == 9 || month == 11) {
		maxDay = 30;
	}
	else if (month == 2) {
		if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
			maxDay = 29;
		}
		else {
			maxDay = 28;
		}
	}

	if (day < 1 || day > maxDay) {
		return false; // 日期无效
	}

	return true;
}

int getWeeksBetweenDates(const std::string& start_date_str, const std::string& end_date_str)
{
	std::tm start_tm = {}, end_tm = {};

	// 解析起始日期
	std::istringstream start_date_stream(start_date_str);
	start_date_stream >> std::get_time(&start_tm, "%Y%m%d");
	std::time_t start_t = std::mktime(&start_tm);
	std::cout << "start year: " << start_tm.tm_year << "month: " << start_tm.tm_mon << "day:" << start_tm.tm_mday << ", start_t: " << start_t << std::endl;;
	// 解析结束日期
	std::istringstream end_date_stream(end_date_str);
	end_date_stream >> std::get_time(&end_tm, "%Y%m%d");
	std::time_t end_t = std::mktime(&end_tm);
	std::cout << "end year: " << end_tm.tm_year << "month: " << end_tm.tm_mon << "day:" << end_tm.tm_mday << ", end_t: " << end_t;

	std::cout << "start data: " << start_date_stream.str() << "    end data: "<< end_date_stream .str() << std::endl;

	if (start_t == -1 || end_t == -1) {
		std::cerr << "Invalid date format." << std::endl;
		return 0;
	}

	std::chrono::duration<int, std::ratio<86400>> days_between = std::chrono::duration_cast<std::chrono::duration<int, std::ratio<86400>>>(std::chrono::seconds(end_t - start_t));
	int num_days = days_between.count();
	int num_weeks = num_days / 7;
	int days_left = num_days % 7;
	if (days_left > 4 )
		num_weeks++;	

	return num_weeks;
}

std::string createUuid()
{
	//std::srand(std::time(0));
	//static std::random_device rd;
	//static std::uniform_int_distribution<uint64_t> dist(0ULL, 0xFFFFFFFFFFFFFFFFULL);
	//uint64_t ab = dist(rd);
	//uint64_t cd = dist(rd);
	//uint32_t a, b, c, d;
	//std::stringstream ss;
	//ab = (ab & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
	//cd = (cd & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
	//a = (ab >> 32U);
	//b = (ab & 0xFFFFFFFFU);
	//c = (cd >> 32U);
	//d = (cd & 0xFFFFFFFFU);
	//ss << std::hex << std::nouppercase << std::setfill('0');
	//ss << std::setw(8) << (a) << '-';
	//ss << std::setw(4) << (b >> 16U) << '-';
	//ss << std::setw(4) << (b & 0xFFFFU) << '-';
	//ss << std::setw(4) << (c >> 16U) << '-';
	//ss << std::setw(4) << (c & 0xFFFFU);
	//ss << std::setw(8) << d;
	//return ss.str();
	std::srand(std::time(0));
	std::stringstream ss;
	std::random_device rd;
	std::mt19937_64 gen(rd());
	std::uniform_int_distribution<uint64_t> dis(0, 0xFFFFFFFFFFFFFFFF);

	// 生成8位16进制数，表示UUID的时间戳低位部分
	ss << std::hex << std::setw(8) << std::setfill('0') << dis(gen);

	ss << "-";

	// 生成4位16进制数，表示UUID的时间戳中位部分
	ss << std::hex << std::setw(4) << std::setfill('0') << dis(gen);

	ss << "-";

	// 生成4位16进制数，表示UUID的版本和变体信息
	ss << "4";  // 表示UUID的版本为4，即基于随机数的UUID
	ss << std::hex << std::setw(3) << std::setfill('0') << (0x8000 | (dis(gen) >> 48));  // 设置UUID的变体信息

	ss << "-";

	// 生成4位16进制数，表示UUID的时间戳高位部分
	ss << "a";  // 表示UUID的variant为10，即基于RFC 4122规范定义的变体
	ss << std::hex << std::setw(3) << std::setfill('0') << (0x8000 | (dis(gen) >> 60));  // 设置UUID的变体信息

	ss << "-";

	// 生成12位16进制数，表示UUID的随机数部分
	for (int i = 0; i < 12; ++i) {
		ss << std::hex << std::setw(1) << std::setfill('0') << (dis(gen) & 0xF);
	}

	return ss.str();
}

bool createDirectory(const std::string dir_path)
{
#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
	if (CreateDirectoryA(dir_path.c_str(), NULL))
		return true;
	else
		return false;
#elif defined(__linux__) || defined(__APPLE__)
	if (mkdir(folderPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
		return true;
	else
		return false;
#endif
}

bool getCurrentWorkDir(std::string& pwd)
{
#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
	char buffer[256];
	if (_getcwd(buffer, sizeof(buffer)) != NULL) {
		pwd = buffer;
		return true;
	}
	else {
		return false;
	}
#elif defined(__linux__) || defined(__APPLE__)
	char buffer[256];
	if (getcwd(buffer, sizeof(buffer)) != NULL) {
		pwd = buffer;
		return true;
	}
	else {
		return false;
	}
#endif
}

bool dirExist(const std::string& dir_path)
{
#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
	DWORD dwAttrib = GetFileAttributes(dir_path.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#elif defined(__linux__) || defined(__APPLE__)
	struct stat st;
	return (stat(folderPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
#endif
}

std::string UTF8ToGB(const char* str)
{
#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
	std::string result;
	WCHAR* strSrc;
	LPSTR szRes;

	//获得临时变量的大小
	int i = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	strSrc = new WCHAR[i + 1];
	MultiByteToWideChar(CP_UTF8, 0, str, -1, strSrc, i);

	//获得临时变量的大小
	i = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
	szRes = new CHAR[i + 1];
	WideCharToMultiByte(CP_ACP, 0, strSrc, -1, szRes, i, NULL, NULL);

	result = szRes;
	delete[]strSrc;
	delete[]szRes;

	return result;
#elif defined(__linux__) || defined(__APPLE__)
return "";
#endif
}

std::string GBToUTF8(const char* str)
{
#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
	std::string outUtf8 = "";
	int n = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	WCHAR* str1 = new WCHAR[n];
	MultiByteToWideChar(CP_ACP, 0, str, -1, str1, n);
	n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
	char* str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
	outUtf8 = str2;
	delete[]str1;
	str1 = NULL;
	delete[]str2;
	str2 = NULL;
	return outUtf8;
#elif defined(__linux__) || defined(__APPLE__)
	return "";
#endif
}

std::list<std::string> splitStr(char ch, const std::string& str)
{
	std::list<std::string> tokens;

	size_t start = 0;
	size_t end = str.find(ch);

	while (end != std::string::npos) {
		tokens.push_back(str.substr(start, end - start));
		start = end + 1;
		end = str.find(ch, start);
	}

	tokens.push_back(str.substr(start));
	return tokens;
}

int getTimeDifferenceOfUtc()
{
	time_t rawtime;
	struct tm local_tm;
	time(&rawtime);
	struct tm utc_tm;	
#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
	localtime_s(&local_tm, &rawtime);
	time_t utc_t = mktime(&local_tm);
	gmtime_s(&utc_tm, &utc_t);
#elif defined(__linux__) || defined(__APPLE__)
	localtime_r(&rawtime, &local_tm);
	time_t utc_t = mktime(&local_tm);
	gmtime_r(&utc_t, &utc_tm);
#endif	
	return local_tm.tm_hour - utc_tm.tm_hour;
}

long long datetimeToMicroSec(const std::string& dateStr, const std::string& timeStr)
{
	struct tm tmValue;
	memset(&tmValue, 0, sizeof(tmValue));
	int time_ms = 0;
#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
	sscanf_s(dateStr.c_str(), "%04d%02d%02d", &tmValue.tm_year, &tmValue.tm_mon, &tmValue.tm_mday);
	sscanf_s(timeStr.c_str(), "%02d%02d%02d.%d", &tmValue.tm_hour, &tmValue.tm_min, &tmValue.tm_sec, &time_ms);
#elif defined(__linux__) || defined(__APPLE__)
	sscanf(dateStr.c_str(), "%04d%02d%02d", &tmValue.tm_year, &tmValue.tm_mon, &tmValue.tm_mday);
	sscanf(timeStr.c_str(), "%02d%02d%02d.%d", &tmValue.tm_hour, &tmValue.tm_min, &tmValue.tm_sec, &time_ms);
#endif
	tmValue.tm_year -= 1900;
	tmValue.tm_mon -= 1;
	//tmValue.tm_mday -= 1;
	
	int tdu = getTimeDifferenceOfUtc();
	//tmValue.tm_hour += tdu;
	tmValue.tm_isdst = -1;	
	time_t timeValue = mktime(&tmValue);
	long long micro_second = static_cast<long long>(timeValue + tdu * 60 * 60) * 1000000LL + time_ms;
	//auto dt = microSecToDt(micro_second);
	return micro_second;	
}

DateTime microSecToDt(long long micro_time)
{
	struct tm time_struct;
	long long ms = micro_time / 1000000;	
#if defined(_WIN32) || defined(WIN32) || defined(_WINDOWS)
	gmtime_s(&time_struct, &ms); // 将秒转换为 UTC 时间
#elif defined(__linux__) || defined(__APPLE__)
	time_t t = ms;
	gmtime_r(&t, &time_struct);
#endif
	DateTime dt;
	dt.m_tm_year = time_struct.tm_year + 1900;
	dt.m_tm_mon = time_struct.tm_mon + 1;
	dt.m_tm_mday = time_struct.tm_mday;
	dt.m_tm_hour = time_struct.tm_hour;
	dt.m_tm_min = time_struct.tm_min;
	dt.m_tm_sec = time_struct.tm_sec;
	dt.m_tm_ms = micro_time % 1000000 / 1000;
	dt.m_tm_us = micro_time % 1000;
	return dt;
}

#if defined(_WIN32) || defined(WIN32)|| defined(_WINDOWS)
#else
bool parseSerial(const char* line, int line_size, const char* match_words, std::string& serial_no)
{
	const char* serial_s = strstr(line, match_words);
	if (NULL == serial_s) {
		return (false);
	}
	serial_s += strlen(match_words);
	while (isspace(serial_s[0])) {
		++serial_s;
	}

	const char* serial_e = line + line_size;
	const char* comma = strchr(serial_s, ',');
	if (NULL != comma) {
		serial_e = comma;
	}

	while (serial_e > serial_s && isspace(serial_e[-1])) {
		--serial_e;
	}

	if (serial_e <= serial_s) {
		return (false);
	}

	std::string(serial_s, serial_e).swap(serial_no);

	return (true);
}

void getSerial(const char* file_name, const char* match_words, std::string& serial_no)
{
	serial_no.c_str();

	std::ifstream ifs(file_name, std::ios::binary);
	if (!ifs.is_open())
	{
		return;
	}

	char line[4096] = { 0 };
	while (!ifs.eof()) {
		ifs.getline(line, sizeof(line));
		if (!ifs.good())
		{
			break;
		}

		if (0 == ifs.gcount())
		{
			continue;
		}

		if (parseSerial(line, ifs.gcount() - 1, match_words, serial_no))
		{
			break;
		}
	}

	ifs.close();
}
#endif

std::string getCpu()
{
#if defined(_WIN32) || defined(WIN32)|| defined(_WINDOWS)
	std::array<int, 4> cpu_id;
	__cpuid(cpu_id.data(), 1);
	char buf[17];
	sprintf_s(buf, "%08X%08X", cpu_id[3], cpu_id[0]);
	return buf;
#else
	std::string strCPUId;
	unsigned int level = 1;
	unsigned eax = 3 /* processor serial number */, ebx = 0, ecx = 0, edx = 0;
	__get_cpuid(level, &eax, &ebx, &ecx, &edx);
	// byte swap
	int first = ((eax >> 24) & 0xff) | ((eax << 8) & 0xff0000) | ((eax >> 8) & 0xff00) | ((eax << 24) & 0xff000000);
	int last = ((edx >> 24) & 0xff) | ((edx << 8) & 0xff0000) | ((edx >> 8) & 0xff00) | ((edx << 24) & 0xff000000);
	// tranfer to string
	std::stringstream ss;
	ss << std::hex << first;
	ss << std::hex << last;
	ss >> strCPUId;
	return strCPUId;
#endif
}

std::string getDisk()
{
#if defined(_WIN32) || defined(WIN32)|| defined(_WINDOWS)
	std::string str = "";
	HRESULT hres;
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	//if (FAILED(hres))
	//{
	//	return str;
	//}

	hres = CoInitializeSecurity(
		NULL,
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities
		NULL                         // Reserved
	);
	//if (FAILED(hres))
	//{
	//	CoUninitialize();
	//	return str;
	//}

	IWbemLocator* pLoc = NULL;
	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hres))
	{
		CoUninitialize();
		return str;
	}

	IWbemServices* pSvc = NULL;
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (e.g. Kerberos)
		0,                       // Context object
		&pSvc                    // pointer to IWbemServices proxy
	);
	if (FAILED(hres))
	{
		pLoc->Release();
		CoUninitialize();
		return str;
	}

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities
	);
	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return str;
	}
	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_PhysicalMedia"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);
	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return str;
	}

	IWbemClassObject* pclsObj = nullptr;
	ULONG uReturn = 0;
	if (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
		if (0 != uReturn)
		{
			VARIANT tmp;
			hr = pclsObj->Get(L"SerialNumber", 0, &tmp, 0, 0);
			if (::SysStringLen(tmp.bstrVal) > 0)
			{
				int index = 0;
				str = (_bstr_t)tmp.bstrVal;
				while ((index = (int)str.find(' ', index)) != std::string::npos)
				{
					str.erase(index, 1);
				}
			}
			VariantClear(&tmp);
		}
	}

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	pclsObj->Release();
	CoUninitialize();
	return str;
#else
	std::string serial_no;
	const char* lshw_result = ".lshw_result.txt";
	char command[512] = { 0 };
	snprintf(command, sizeof(command), "lsblk -o serial /dev/nvme0n1 | awk NR==2 > %s", lshw_result);

	if (0 == system(command)) {
		getSerial(lshw_result, "", serial_no);
	}
	unlink(lshw_result);
	return serial_no;
#endif
}

std::list<std::string> getDisks()
{
#if defined(_WIN32) || defined(WIN32)|| defined(_WINDOWS)
	std::list<std::string> list;
	HRESULT hres;
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	//if (FAILED(hres))
	//{
	//	return vec;
	//}

	hres = CoInitializeSecurity(
		NULL,
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities
		NULL                         // Reserved
	);
	//if (FAILED(hres))
	//{
	//	CoUninitialize();
	//	return vec;
	//}

	IWbemLocator* pLoc = NULL;
	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID*)&pLoc);
	if (FAILED(hres))
	{
		CoUninitialize();
		return list;
	}

	IWbemServices* pSvc = NULL;
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (e.g. Kerberos)
		0,                       // Context object
		&pSvc                    // pointer to IWbemServices proxy
	);
	if (FAILED(hres))
	{
		pLoc->Release();
		CoUninitialize();
		return list;
	}

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities
	);
	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return list;
	}
	IEnumWbemClassObject* pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_PhysicalMedia"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);
	if (FAILED(hres))
	{
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return list;
	}

	IWbemClassObject* pclsObj = nullptr;
	ULONG uReturn = 0;
	std::string buf;
	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
		if (0 == uReturn)
		{
			break;
		}
		VARIANT tmp;
		hr = pclsObj->Get(L"SerialNumber", 0, &tmp, 0, 0);
		if (::SysStringLen(tmp.bstrVal) > 0)
		{
			int index = 0;
			buf = (_bstr_t)tmp.bstrVal;
			while ((index = (int)buf.find(' ', index)) != std::string::npos)
			{
				buf.erase(index, 1);
			}
			list.push_back(buf);
		}
		VariantClear(&tmp);
	}

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	pclsObj->Release();
	CoUninitialize();

	return list;
#else
	std::list<std::string> list;
	return list;
#endif
}

int encryptAES(const std::string& str_source, const std::string& str_key, std::string& result)
{
	result = "";
	AES aes(128);
	unsigned char source[128] = { 0 };

	strncpy((char*)source, str_source.c_str(), str_source.length());
	unsigned int inLen = str_source.length();
	if (inLen % 16 != 0)
	{
		inLen = (inLen + 16) / 16 * 16;
	}
	unsigned char key[64] = { 0 };
	strncpy((char*)key, str_key.c_str(), str_key.length());

	int ret = 0;
	unsigned int outlen = 0;
	unsigned char* out = aes.EncryptECB(source, inLen, key, outlen);
	if (out != NULL && outlen > 0)
	{
		for (int i = 0; i < outlen && out[i] != '\0'; i++)
		{
			char szBuf[10] = { 0 };
			sprintf(szBuf, "%02x", out[i]);
			result += szBuf;
		}
		delete[] out;
		out = NULL;
	}
	else
	{
		ret = -1;
	}
	if (out != NULL) delete[] out;
	return ret;
}

int decryptAES(const std::string& str_source, const std::string& str_key, std::string& result)
{
	result = "";
	AES aes(128);
	unsigned char source[1024] = { 0 };

	char* source_str = const_cast<char *>(str_source.c_str());
	for (int i = 0; i < strlen(source_str) / 2; i++)
	{
		sscanf(source_str + 2 * i, "%02x", &source[i]);
	}
	unsigned char key[64] = { 0 };
	strncpy((char*)key, str_key.c_str(), str_key.length());

	int ret = 0;
	unsigned int outlen = 0;
	unsigned char* out = aes.DecryptECB(source, strlen(source_str) / 2, key, outlen);
	if (out != NULL && outlen > 0)
	{
		for (int i = 0; i < outlen && out[i] != '\0'; i++)
		{
			if (!isgraph(out[i]))
			{		
				ret = -1;
				break;
			}
		}
		if (0 == ret)
			result = (char *)out;
	}
	if (out == NULL)
		ret = -1;
	if (out != NULL) delete[] out;
	return ret;
}

int encryptCBC(const std::string& str_source, const std::string& str_key, const std::string& str_vi, std::string& result)
{
	result = "";
	AES aes(128);
	unsigned char source[128] = { 0 };

	strncpy((char*)source, str_source.c_str(), str_source.length());
	unsigned int inLen = str_source.length();
	if (inLen % 16 != 0)
	{
		inLen = (inLen + 16) / 16 * 16;
	}
	unsigned char key[64] = { 0 };
	strncpy((char*)key, str_key.c_str(), str_key.length());

	unsigned char vi[64] = { 0 };
	strncpy((char*)vi, str_vi.c_str(), str_vi.length());

	int ret = 0;
	unsigned int outlen = 0;
	unsigned char* out = aes.EncryptCBC(source, inLen, key, vi, outlen);
	if (out != NULL && outlen > 0)
	{
		for (int i = 0; i < outlen && out[i] != '\0'; i++)
		{
			char szBuf[10] = { 0 };
			sprintf(szBuf, "%02x", out[i]);
			result += szBuf;
		}
		delete[] out;
		out = NULL;
	}
	else
	{
		ret = -1;
	}
	if (out != NULL) delete[] out;
	return ret;
}

int decryptCBC(const std::string& str_source, const std::string& str_key, const std::string& str_vi, std::string& result)
{
	result = "";
	AES aes(128);
	unsigned char source[1024] = { 0 };

	char* source_str = const_cast<char*>(str_source.c_str());
	for (int i = 0; i < strlen(source_str) / 2; i++)
	{
		sscanf(source_str + 2 * i, "%02x", &source[i]);
	}
	unsigned char key[64] = { 0 };
	strncpy((char*)key, str_key.c_str(), str_key.length());

	unsigned char vi[64] = { 0 };
	strncpy((char*)vi, str_vi.c_str(), str_vi.length());

	int ret = 0;
	unsigned int outlen = 0;
	unsigned char* out = aes.DecryptCBC(source, strlen(source_str) / 2, key, vi, outlen);
	if (out != NULL && outlen > 0)
	{
		for (int i = 0; i < outlen && out[i] != '\0'; i++)
		{
			if (!isgraph(out[i]))
			{
				ret = -1;
				break;
			}
		}
		if (0 == ret)
			result = (char*)out;
	}
	if (out == NULL)
		ret = -1;
	if (out != NULL) delete[] out;
	return ret;
}

//template<typename T>
//int compareValue(T a, T b, T epsilon)
//{
//	if (std::fabs(a - b) < epsilon)
//		return 0;
//	else if (a > b)
//		return 1;
//	else
//		return -1;
//}
bool isFileInUse(const wchar_t* filename)
{
#if defined(_WIN32) || defined(WIN32)|| defined(_WINDOWS)
	HANDLE hFile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return true;

	CloseHandle(hFile);
	return false;
#else
	std::setlocale(LC_ALL, "");
	int len = std::wcslen(filename) + 1;
	char* fname = new char[len];
	std::wcstombs(fname, filename, len);
	// 打开文件
	int fd = open(fname, O_RDONLY);
	if (fd == -1)
	{
		// 文件打开失败，可能是因为文件正在被使用
		return true;
	}

	// 判断文件是否被占用
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;

	bool in_use = (fcntl(fd, F_GETLK, &fl) == -1);

	// 关闭文件
	close(fd);

	return in_use;
#endif
}

bool deleteFile(const std::string& file_path)
{
	if (file_path.empty())
		return false;
#if defined(_WIN32) || defined(WIN32)|| defined(_WINDOWS)
	std::wstring wstr = string2WString(file_path);
	const wchar_t* wchars = wstr.c_str();

	LPCWSTR file = wchars;
	BOOL result2 = DeleteFileW(file);
	if (result2)
	{
		return true;
	}
	else
	{
		// 文件删除失败，处理错误
		DWORD error = GetLastError();
		std::cout <<  "delete failed, errno：" << error << std::endl;
		return false;
	}
#else
	int result = std::remove(file_path.data());

	if (result == 0) {
		return true;
	}
	else {
		return false;
	}
#endif
}

bool cancelFileRdOnlyAttibute(const std::string& file_path)
{
	if (file_path.empty())
		return false;
#if defined(_WIN32) || defined(WIN32)|| defined(_WINDOWS)	
	std::wstring wstr = string2WString(file_path);
	const wchar_t* wchars = wstr.c_str();	

	LPCWSTR file = wchars;
	DWORD attributes = GetFileAttributesW(file);

	// 检查文件是否存在
	if (attributes == INVALID_FILE_ATTRIBUTES)
	{
		std::cerr << "文件不存在\n";
		return false;
	}

	// 将文件属性设置为非只读模式
	attributes &= ~FILE_ATTRIBUTE_READONLY;
	if (!SetFileAttributesW(file, attributes))
	{
		std::cerr << "无法修改文件属性\n";
		return false;
	}
	return true;
#else
	// 获取文件的当前权限位
	struct stat st;
	if (stat(file_path.c_str(), &st) != 0)
	{
		// 获取文件状态失败
		return false;
	}

	// 判断文件是否只读
	if ((st.st_mode & S_IWUSR) == 0)
	{
		// 文件已经是可写的，不需要修改权限位
		return true;
	}

	// 移除文件只读权限
	int new_mode = st.st_mode & ~S_IWUSR;
	if (chmod(file_path.c_str(), new_mode) != 0)
	{
		// 修改文件权限失败
		return false;
	}

	return true;
#endif
	
}

// wstring=>string
std::string wstring2String(const std::wstring& ws)
{
	std::string strLocale = setlocale(LC_ALL, "");
	const wchar_t* wchSrc = ws.c_str();
	size_t nDestSize = wcstombs(NULL, wchSrc, 0) + 1;
	char* chDest = new char[nDestSize];
	memset(chDest, 0, nDestSize);
	wcstombs(chDest, wchSrc, nDestSize);
	std::string strResult = chDest;
	delete[] chDest;
	setlocale(LC_ALL, strLocale.c_str());
	return strResult;
}

// string => wstring
std::wstring string2WString(const std::string& s)
{
	std::string strLocale = setlocale(LC_ALL, "");
	const char* chSrc = s.c_str();
	size_t nDestSize = mbstowcs(NULL, chSrc, 0) + 1;
	wchar_t* wchDest = new wchar_t[nDestSize];
	wmemset(wchDest, 0, nDestSize);
	mbstowcs(wchDest, chSrc, nDestSize);
	std::wstring wstrResult = wchDest;
	delete[] wchDest;
	setlocale(LC_ALL, strLocale.c_str());
	return wstrResult;
}
