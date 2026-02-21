#pragma once
#include <memory>
#include <string>

namespace NetLibResources
{
	void global_init();
	void global_free();

	template<typename T> class CfgValue
	{
		std::unique_ptr<T> data;
	public:
		CfgValue() { }
		CfgValue(T* value) : data(value) { }
		CfgValue(const CfgValue& src) : data(src.IsEmpty() ? nullptr : new T(src.Get())) { }
		CfgValue<T>& operator=(const CfgValue<T>& src) { data.reset(src.IsEmpty() ? nullptr : new T(src.Get())); return *this; }
		bool IsEmpty() const { return data == nullptr; }
		const T& Get() const { return *data.get(); }
		void Set(T* value) { data.reset(value); }
	};

	struct NetworkSettings
	{
		CfgValue<std::string> CertificateAuthorityBundle;
		CfgValue<bool> SslVerifyPeer, SslVerifyHost;
		CfgValue<std::string> HttpUserAgent;

		NetworkSettings();
		NetworkSettings(const NetworkSettings& src);
		NetworkSettings& operator=(const NetworkSettings& src);
	};

	NetworkSettings global_cfg_get();
	void global_cfg_set(NetworkSettings cfg);
}
