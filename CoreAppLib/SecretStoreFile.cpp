#include "SecretStoreFile.h"
#include <cstring>
#include <fstream>

namespace SecretStoreFile_Imp
{
#define FileSignatureStr "LSSD"
#define FileSignatureLen 4

	static bool stream_write_u16(std::ofstream& stm, uint16_t val);
	static bool stream_write_u32(std::ofstream& stm, uint32_t val);
	static bool stream_read_u16(std::ifstream& stm, uint16_t& val);
	static bool stream_read_u32(std::ifstream& stm, uint32_t& val);
}
using namespace SecretStoreFile_Imp;

SecretStoreFile::SecretStoreFile(const FILE_PATH_CHAR* file_path)
	: filePath(file_path)
{ }

void SecretStoreFile::SetHeader(uint8_t ver, uint32_t attr24)
{
	version = ver;
	hdrAttributes = attr24 & 0x00FFFFFF;
}

uint8_t SecretStoreFile::GetVersion() const
{
	return version;
}

uint32_t SecretStoreFile::GetAttributes() const
{
	return hdrAttributes;
}

bool SecretStoreFile::Load()
{
	data.clear();

	std::ifstream stm(filePath, std::ios::binary);
	if (!stm) return true; // empty file is OK

	char sig[FileSignatureLen];
	stm.read(sig, FileSignatureLen);
	if (!stm.good() || std::memcmp(sig, FileSignatureStr, FileSignatureLen) != 0)
		return false;

	stm.read(reinterpret_cast<char*>(&version), 1);
	if (!stm.good()) return false;

	unsigned char hdr_attr[3] = { 0 };
	stm.read(reinterpret_cast<char*>(hdr_attr), 3);
	if (!stm.good()) return false;

	hdrAttributes =
		(uint32_t(hdr_attr[0])) | (uint32_t(hdr_attr[1]) << 8) | (uint32_t(hdr_attr[2]) << 16);

	while (true) {
		uint16_t rec_attr = 0, key_len = 0;
		uint32_t val_len = 0;

		if (!stream_read_u16(stm, rec_attr)) return false;
		if (!stream_read_u16(stm, key_len)) return false;
		if (!stream_read_u32(stm, val_len)) return false;

		if (rec_attr == 0 && key_len == 0 && val_len == 0)
			break; // Sentinel - the end of the data
		if ((key_len > KeyMaxLength) || (val_len > ValueMaxLength))
			return false; // Probbaly broken data

		std::string key(key_len, '\0'), value(val_len, '\0');

		stm.read(&key[0], key_len);
		if (!stm.good()) return false;

		stm.read(&value[0], val_len);
		if (!stm.good()) return false;

		data[key] = { rec_attr, value };
	}

	return true;
}

bool SecretStoreFile::Save()
{
	std::ofstream stm(filePath, std::ios::binary | std::ios::trunc);
	if (!stm) return false;

	stm.write(FileSignatureStr, FileSignatureLen);
	stm.write(reinterpret_cast<const char*>(&version), 1);

	unsigned char attr_bytes[3] = {
		(unsigned char)(hdrAttributes & 0xFF),
		(unsigned char)((hdrAttributes >> 8) & 0xFF),
		(unsigned char)((hdrAttributes >> 16) & 0xFF)
	};
	stm.write(reinterpret_cast<const char*>(attr_bytes), 3);

	for (const auto& kv : data) {
		const std::string& key = kv.first;
		const Record& rec = kv.second;

		if (!stream_write_u16(stm, rec.Attributes)) return false;
		if (!stream_write_u16(stm, (uint16_t)key.size())) return false;
		if (!stream_write_u32(stm, (uint32_t)rec.Value.size())) return false;

		stm.write(key.data(), key.size());
		stm.write(rec.Value.data(), rec.Value.size());
		if (!stm.good()) return false;
	}

	// Sentinel (the data end)
	if (!stream_write_u16(stm, 0)) return false;
	if (!stream_write_u16(stm, 0)) return false;
	if (!stream_write_u32(stm, 0)) return false;

	return stm.good();
}

bool SecretStoreFile::Set(const char* key, uint16_t attributes, const char* value, size_t size)
{
	if (!key || (size && ((size > ValueMaxLength) || !value))) return false;
	std::string rec_key(key);
	if (rec_key.size() > KeyMaxLength) return false;
	data[rec_key] = { attributes, std::string(value, size) };
	return true;
}

bool SecretStoreFile::Get(const char* key, uint16_t& attributes, std::string& value) const
{
	auto it = data.find(key);
	if (it == data.end())
		return false;
	attributes = it->second.Attributes;
	value = it->second.Value;
	return true;
}

bool SecretStoreFile::Remove(const char* key)
{
	auto it = data.find(key);
	if (it == data.end())
		return false;
	data.erase(it);
	return true;
}

std::vector<std::string> SecretStoreFile::ListKeys() const
{
	std::vector<std::string> keys;
	keys.reserve(data.size());
	for (const auto& kv : data)
		keys.push_back(kv.first);
	return keys;
}

// ************************************** SecretStoreFile_Imp **************************************

bool SecretStoreFile_Imp::stream_write_u16(std::ofstream& stm, uint16_t val)
{
	stm.write(reinterpret_cast<const char*>(&val), sizeof(val));
	return stm.good();
}

bool SecretStoreFile_Imp::stream_write_u32(std::ofstream& stm, uint32_t val)
{
	stm.write(reinterpret_cast<const char*>(&val), sizeof(val));
	return stm.good();
}

bool SecretStoreFile_Imp::stream_read_u16(std::ifstream& stm, uint16_t& val)
{
	stm.read(reinterpret_cast<char*>(&val), sizeof(val));
	return stm.gcount() == sizeof(val);
}

bool SecretStoreFile_Imp::stream_read_u32(std::ifstream& stm, uint32_t& val)
{
	stm.read(reinterpret_cast<char*>(&val), sizeof(val));
	return stm.gcount() == sizeof(val);
}
