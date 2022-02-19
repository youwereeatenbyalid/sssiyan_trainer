#pragma once
#include "utility/Config.hpp"
#include "utility/Scan.hpp"
#include <sstream>

class InitPatternsManager
{
private:
	utility::Config *patternsCfg = nullptr;
	bool isInitViaPatternsList = true;
	std::string listInitKey;
	bool isEmptyFile = false;
	bool IsChangesWasMade = false;

	uintptr_t to_uintptr_t(const std::string& address)
	{
		uintptr_t ret;
		std::stringstream stream(address);
		stream >> std::hex >> ret;
		return ret;
	}

	std::string to_hex_str(uintptr_t num)
	{
		std::stringstream stream;
		stream << std::hex << num;
		return stream.str();
	}

	void load()
	{
		if (patternsCfg == nullptr)
			return;
		patternsCfg->load();
		isInitViaPatternsList = patternsCfg->get<bool>(listInitKey).value_or(true);
	}

	void add(uintptr_t offs, const std::string& pattern)
	{
		if (patternsCfg == nullptr)
			return;
		patternsCfg->set(pattern, to_hex_str(offs));
		IsChangesWasMade = true;
	}

public:
	InitPatternsManager(const char* fileName, const char* initKey) : patternsCfg(new utility::Config(fileName)), listInitKey(initKey)
	{
		if(patternsCfg->get_key_values().empty())
			isEmptyFile = true;
		isInitViaPatternsList = patternsCfg->get<bool>(initKey).value_or(true);
	}

	~InitPatternsManager() { free(); };

	std::optional<uintptr_t> find_addr(HMODULE base, const std::string &pattern)
	{
		if (patternsCfg == nullptr)
			return utility::scan(base, pattern);
		auto uintptrBase = (uintptr_t)base;
		if (!isEmptyFile && isInitViaPatternsList)
		{
			auto data = patternsCfg->get(pattern);
			if (data.has_value())
				return std::make_optional<uintptr_t>(uintptrBase + to_uintptr_t( data.value()));
		}
		auto res = utility::scan(base, pattern);
		if (isInitViaPatternsList)
		{
			if (res.has_value())
				add(res.value() - uintptrBase, pattern);
		}
		return res;
	}

	void save()
	{
		if (patternsCfg == nullptr)
			return;
		patternsCfg->save();
	}

	bool is_init_by_patt_lst() const noexcept { return isInitViaPatternsList; }

	bool is_init() const noexcept { return patternsCfg != nullptr; }

	bool is_changed() const noexcept { return IsChangesWasMade; }

	void free()
	{
		if(patternsCfg != nullptr)
			delete patternsCfg;
		patternsCfg = nullptr;
	}
};
