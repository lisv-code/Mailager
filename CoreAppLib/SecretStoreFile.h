#pragma once
#ifndef _LIS_SECRET_STORE_FILE_H_
#define _LIS_SECRET_STORE_FILE_H_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <LisCommon/FileSystem.h>

class SecretStoreFile
{
public:
	static const size_t KeyMaxLength = 0xFFFF; // physical limit of uint16_t - 65535
	static const size_t ValueMaxLength = 0x7FFFFFFF; // 2 GB logical limit

	struct Record {
		uint16_t Attributes = 0;
		std::string Value; // raw binary
	};

	explicit SecretStoreFile(const FILE_PATH_CHAR* file_path);

	bool Load();
	bool Save();

	// Key length is limited to 65535 bytes (16-bit), Value - 2 GB (32-bit) size
	bool Set(const char* key, uint16_t attributes, const char* value, size_t size);
	bool Get(const char* key, uint16_t& attributes, std::string& value) const;
	bool Remove(const char* key);

	std::vector<std::string> ListKeys() const;

	void SetHeader(uint8_t ver, uint32_t attr24);
	uint8_t GetVersion() const;
	uint32_t GetAttributes() const;

private:
	std::basic_string<FILE_PATH_CHAR> filePath;

	uint8_t version = 1;
	uint32_t hdrAttributes = 0; // 24-bit stored in 3 bytes

	std::unordered_map<std::string, Record> data;
};

#endif // _LIS_SECRET_STORE_FILE_H_
