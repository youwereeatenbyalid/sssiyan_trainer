#include "SDKHelpers.hpp"

namespace srtc {
	static std::mutex g_mtx;

	::SystemString* create_managed_string(std::wstring_view str) {
		std::scoped_lock _{ g_mtx };
		return sdk::VM::create_managed_string(str);
	}

	SString::SString(const std::wstring_view& str)
		: m_str(create_managed_string(str))
	{
		add_ref();
	}

	SString::SString(const SystemString* str)
		: m_str(create_managed_string(str->data))
	{
		add_ref();
	}

	SString::SString(const SString& other)
		: m_str(other.m_str)
	{
		add_ref();
	}

	SString::SString(SString&& other) 
		: m_str(std::move(other.m_str))
	{
		other.m_str = nullptr;
	}

	SString::~SString()
	{
		release();
	}

	void SString::add_ref()
	{
		if (m_str == nullptr) {
			return;
		}

		utility::re_managed_object::add_ref(m_str);
	}

	void SString::release()
	{
		if (m_str == nullptr) {
			return;
		}

		utility::re_managed_object::release(m_str);
	}
}