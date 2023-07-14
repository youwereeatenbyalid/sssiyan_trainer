#pragma once

#include <mutex>
#include <windows.h>
#include <cstdint>
#include <functional>

#include <utility/Address.hpp>

class Detour_t {
public:
	Detour_t(Address target, Address destination, int64_t return_offset = 0);

	~Detour_t();

	/*
	* Returns the address of where the detour should be placed
	*/
	const auto& get_original() const { return m_original; }

	/*
	* Returns the address of the code the detour wouldl jump to
	*/
	const auto& get_destination() const { return m_destination; }

	/*
	* Returns the address of a copy of the original code that has been replaced by the detour jump
	*/
	const auto& get_trampoline() const { return m_trampoline; }

	/// <summary>
	/// Returns get trampoline as a void function to call for original behavior (I think)
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template <typename T>
	T* get_trampoline() const {
		return reinterpret_cast<T*>(m_trampoline);
	}

	/*
	* Returns the return address which the detour is supposed to jump to after execution
	*/
	Address get_return_address() const { return m_return_offset == 0 ? Address(m_trampoline) : Address(m_original.as<uintptr_t>() + m_return_offset); }

	/*
	* Returns the return address offset from the original code
	*/
	const auto& get_return_offset() const { return m_return_offset; }

	/*
	* Sets the return offset used to calculate the return address
	*/
	void set_return_offset(int64_t offset) { m_return_offset = offset; }

	/*
	* Returns a bool that indicates the state of the detour whether if the target and destination are set (true) or not (false)
	*/
	auto is_valid() const { return m_trampoline != nullptr; }

	/*
	* Returns a bool that indicates the state of the detour whether if the detour is in place (true) or not (false)
	*/
	const auto& is_enabled() const { return m_is_enabled; }

	/*
	* Places the detour in place, returns true if succeeded and fale if failed
	*/
	bool enable();

	/*
	* Adds the detour to the list but doesn't place the detour in place, returns true if succeeded and false if failed
	*/
	bool create();

	/*
	* Disables the detour but doesn't remove it from minhook's table, returns true if succeeded and false if failed
	*/
	bool disable();

	/*
	* Checks if the detour is valid automatically internally then disables the detour and returns a bool
	* which indicates whether the remove operation was successful or not, if the detour
	* was never created using the Create() method, the return value would be true
	*/
	bool remove();

	/*
	* Calls Create() if the detour is not in place (Is disabled) or
	* calls Remove() if the detour is already in place (Is enabled)
	* returns the current state of the hook after enabling (true) or disabling (false) it
	*/
	bool toggle();

	/*
	* Takes a bool and enables (true) or disables (false) the detour based on that
	* returns the current state of the hook after enabling (true) or disabling (false) it
	*/
	bool toggle(bool state);

	/*
	* Set a callback that will be called before the detour is placed
	* The argument of the callback is the current Detour_t object pointer
	*/
	void set_pre_hook_enable_callback(std::function<void(Detour_t*)> cb) { m_pre_hook_enable_callback = cb; }

	/*
	* Set a callback that will be called after the detour is placed
	* The argument of the callback is the current Detour_t object pointer
	*/
	void set_post_hook_enable_callback(std::function<void(Detour_t*)> cb) { m_post_hook_enable_callback = cb; }

	/*
	* Set a callback that will be called before the detour is removed
	* The argument of the callback is the current Detour_t object pointer
	*/
	void set_pre_hook_disable_callback(std::function<void(Detour_t*)> cb) { m_pre_hook_disable_callback = cb; }

	/*
	* Set a callback that will be called after the detour is removed
	* The argument of the callback is the current Detour_t object pointer
	*/
	void set_post_hook_disable_callback(std::function<void(Detour_t*)> cb) { m_post_hook_disable_callback = cb;  }

private:
	std::recursive_mutex m_hook_mutex{};
	bool m_is_enabled{ false };

	std::function<void(Detour_t*)> m_pre_hook_enable_callback{}, m_post_hook_enable_callback{},
		m_pre_hook_disable_callback{}, m_post_hook_disable_callback{};

	int64_t m_return_offset = 0;
	Address m_original{ nullptr };
	Address m_destination{ nullptr };
	void* m_trampoline{ nullptr };
};