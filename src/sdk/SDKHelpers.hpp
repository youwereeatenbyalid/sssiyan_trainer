#pragma once
#include "sdk/ReClass.hpp"
#include "sdk/REContext.hpp"

namespace srtc {
	::SystemString* create_managed_string(std::wstring_view str); // System.String

	class SString {
	public:
		SString(const std::wstring_view& str);
		SString(const SystemString* str);
		SString(const SString& other);
		SString(SString&& other);

		~SString();

		void add_ref();
		void release();

		auto get() {
			return m_str;
		}

		auto get_copy() {
			add_ref();
			return get();
		}

		inline operator bool() { return m_str != nullptr; }

	private:
		SystemString* m_str = nullptr;
	};
}