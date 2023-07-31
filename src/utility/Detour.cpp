#include "Detour.hpp"
#include <windows.h>
#include <MinHook.h>
#include <spdlog/spdlog.h>

bool g_is_minhook_initialized{ false };

Detour_t::Detour_t(Address target, Address destination, int64_t return_offset)
	: m_original(target), m_destination(destination), m_return_offset(return_offset)
{
	std::scoped_lock _{ m_hook_mutex };

	// Initialize MinHook if it hasn't been already.
	if (!g_is_minhook_initialized && MH_Initialize() == MH_OK) {
		g_is_minhook_initialized = true;
	}

	// Create the hook. Call create afterwards to prevent race conditions accessing FunctionHook before it leaves its constructor.
	if (const auto status = MH_CreateHook(m_original.as<LPVOID>(), m_destination.as<LPVOID>(), reinterpret_cast<LPVOID*>(&m_trampoline)); status == MH_OK) {
		spdlog::info("Detour init successful {:p}->{:p}", m_original.ptr(), m_destination.ptr());
	}
	else {
		spdlog::error("Failed to detour {:p}: {}", m_original.ptr(), MH_StatusToString(status));
		return;
	}

	create();
}

Detour_t::~Detour_t()
{
	remove();
}

bool Detour_t::enable()
{
	std::scoped_lock _{ m_hook_mutex };

	if (m_pre_hook_enable_callback)
		m_pre_hook_enable_callback(this);

	if (auto status = MH_EnableHook(m_original.as<LPVOID>()); status != MH_OK) {

		spdlog::error("Failed to place the detour {:x}: {}", m_original.ptr(), MH_StatusToString(status));

		return false;
	}

	m_is_enabled = true;
	
	if (m_post_hook_enable_callback)
		m_post_hook_enable_callback(this);
	
	return true;
}

bool Detour_t::create()
{
	std::scoped_lock _{ m_hook_mutex };

	if (m_trampoline != nullptr) {
		return m_trampoline;
	}

	if (m_original == nullptr || m_destination == nullptr || m_trampoline == nullptr) {
		spdlog::error("Detour not initialized");
		return false;
	}

	spdlog::info("Detour added {:x}->{:x}", m_original.ptr(), m_destination.ptr());

	return true;
}

bool Detour_t::disable() 
{
	std::scoped_lock _{ m_hook_mutex };

	if (m_pre_hook_disable_callback)
		m_pre_hook_disable_callback(this);

	if (auto status = MH_DisableHook(m_original.as<LPVOID>()); status != MH_OK) {
		spdlog::error("Failed to remove the detour {:x}: {}", m_original.ptr(), MH_StatusToString(status));
		return false;
	}

	m_is_enabled = false;
	
	if (m_post_hook_disable_callback)
		m_post_hook_disable_callback(this);
	
	return true;
}

bool Detour_t::remove()
{
	std::scoped_lock _{ m_hook_mutex };
	// Don't try to remove invalid detours.
	if (!is_valid()) {
		return true;
	}

	if (disable() == false || MH_RemoveHook(m_original) != MH_OK) {
		return false;
	}

	m_trampoline = nullptr;
	return true;
}

bool Detour_t::toggle()
{
	return toggle(!m_is_enabled);
}

bool Detour_t::toggle(bool state)
{
	if (state)
		enable();
	else
		disable();

	return m_is_enabled;
}