#include <fstream>
#include <sstream>

#include "String.hpp"
#include "Config.hpp"

// clang-format off

using namespace std;

namespace utility {
    Config::Config(const string& filePath)
        : m_key_values{}, m_config_path{ filePath }
    {
        if (!filePath.empty()) {
            load(filePath);
        }
    }

    bool Config::load(const string& filePath) {
        if (filePath.empty()) {
            return false;
        }

        ifstream f(widen(filePath));

        if (!f) {
            return false;
        }

        for (string line{}; getline(f, line); ) {
            istringstream ss{ line };
            string key{};
            string value{};

            getline(ss, key, '=');
            getline(ss, value);

            set(key, value);
        }

        return true;
    }

    // Added by Darkness
    bool Config::load() { 
        if (!m_config_path.empty()) {
            return load(m_config_path);
        }

        return false;
    }

    bool Config::save(const string& filePath) {
        ofstream f(widen(filePath));

        if (!f) {
            return false;
        }

        for (auto& keyValue : m_key_values) {
            f << keyValue.first << "=" << keyValue.second << endl;
        }

        return true;
    }

    void Config::append(const Config& other)
    {
        for(const auto& entry : other.get_key_values())
        {
            m_key_values[entry.first] = entry.second;
        }
    }

    // Added by Darkness
    bool Config::save() {
        if (!m_config_path.empty()) {
            return save(m_config_path);
        }

        return false;
    }

    optional<string> Config::get(const string& key) const {
        auto search = m_key_values.find(key);

        if (search == m_key_values.end()) {
            return {};
        }

        return search->second;
    }

    void Config::set(const string& key, const string& value) {
        if (!key.empty() && !value.empty()) {
            m_key_values[key] = value;
        }
    }

}
