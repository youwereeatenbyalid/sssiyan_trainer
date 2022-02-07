#pragma once

#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace utility {
    class Config {
    public:
        Config(const std::string& file_path = "");

        bool load(const std::string& file_path);
        bool save(const std::string& file_path);

        // Added by Darkness
        void append(const Config& other);

        // Added by Darkness
        std::string get_path() const {
            return m_config_path;
        }

        bool load();
        bool save();

        Config& operator=(Config& other) { return *this; }
        const Config& operator=(const Config& other) const { return *this; }

        // Added by Darkness
        Config operator+(const Config& other) const { auto tmp = *this; tmp.append(other); return tmp; }

    	// Added by Darkness
        Config& operator+=(Config& other) { this->append(other); return *this; }
        const Config& operator+=(const Config& other) { this->append(other); return *this; }

        // Helper for differentiating between boolean and arithmetic values.
        template <typename T>
        using is_arithmetic_not_bool = std::bool_constant<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>;

        template <typename T>
        static constexpr bool IS_ARITHMETIC_NOT_BOOL_V = is_arithmetic_not_bool<T>::value;

        // Added by Darkness
        // Classic SFINAE...
		template<typename T>
		struct IsVector
		{
			static constexpr bool value = false;
		};

		template<typename T>
		struct IsVector<std::vector<T>>
		{
			static constexpr bool value = true;
		};

        // get method for arithmetic types.
        template <typename T>
        std::optional<typename std::enable_if_t<IS_ARITHMETIC_NOT_BOOL_V<T>, T>> get(const std::string& key) const {
            auto value = get(key);

            if (!value) {
                return {};
            }

            // Use the correct conversion function based on the type.
            if constexpr (std::is_integral_v<T>) {
                if constexpr (std::is_unsigned_v<T>) {
                    return (T)std::stoul(*value);
                }
                else {
                    return (T)std::stol(*value);
                }
            }
            else {
                return (T)std::stod(*value);
            }
        }

        // get method for boolean types.
        template <typename T>
        std::optional<typename std::enable_if_t<std::is_same_v<T, bool>, T>> get(const std::string& key) const {
            auto value = get(key);

            if (!value) {
                return {};
            }

            if (*value == "true") {
                return true;
            }

            if (*value == "false") {
                return false;
            }

            return {};
        }

        // get method for strings.
        std::optional<std::string> get(const std::string& key) const;

        // Added by Darkness
		// Get method for arrays.
		template<typename T>
        std::optional<typename std::enable_if_t<IsVector<T>::value, T>> get(const std::string& key)
		{
            using El_T = typename T::value_type;

			auto valuesStr = get(key);

			if (!valuesStr) {
				return {};
			}
            
            std::vector<std::string> valuesParsed;
            std::string tempBuffer;

            // Extract elements.
            for (size_t i = 0; i < (*valuesStr).size(); i++) {
                if (i < (*valuesStr).size() - 1 && (*valuesStr).substr(i, 2) != std::string(", ")) {
                    tempBuffer += (*valuesStr)[i];
                }
                else if ((*valuesStr).substr(i, 2) == std::string(", ") || i == (*valuesStr).size() - 1) {
                    if (i == (*valuesStr).size() - 1) tempBuffer += (*valuesStr)[i];
                    valuesParsed.push_back(tempBuffer);
                    tempBuffer.clear();
                    i++;
				}
            }

            T ret;

            for (auto& value : valuesParsed) {
                // Use the correct conversion function based on the type.
                if constexpr (std::is_integral_v<El_T>) {
					if constexpr (std::is_same_v<El_T, bool>) {
						if (value == "true") {
							ret.push_back(true);
						}

						if (value == "false") {
							ret.push_back(false);
						}
					}
					else if constexpr (std::is_unsigned_v<El_T>) {
                        ret.push_back((El_T)std::stoul(value));
                    }
                    else {
                        ret.push_back((El_T)std::stol(value));
                    }
                }
                else if constexpr (std::is_floating_point_v<El_T> || std::is_same_v<El_T, double>) {
                    ret.push_back((El_T)std::stod(value));
                }
                else if constexpr (std::is_same_v<El_T, std::string>) {
                    ret.push_back(value);
                }
                else {
                    // Not handled
                }
			}

            return std::optional<T>(ret);
		}

        // set method for arithmetic types.
        template <typename T>
        void set(const std::string& key, typename std::enable_if_t<IS_ARITHMETIC_NOT_BOOL_V<T>, T> value) {
            set(key, std::to_string(value));
        }

        // set method for boolean types.
        template <typename T>
        void set(const std::string& key, typename std::enable_if_t<std::is_same_v<T, bool>, T> value) {
            if (value) {
                set(key, "true");
            }
            else {
                set(key, "false");
            }
        }

        // Added by darkness
        // set method for arrays.
		template<typename T>
		void set(const std::string& key, typename std::enable_if_t<IsVector<T>::value, const T&> values)
		{
            using El_T = typename T::value_type;

            std::vector<std::string> tempValues;
            tempValues.reserve(values.size());
            if constexpr (std::is_same_v<El_T, std::string>) {
				for (auto& value : values) {
					tempValues.push_back(value);
				}
            }
            else if constexpr (std::is_same_v<El_T, bool>) {
				for (const auto& value : values) {
                    if (value == true) {
                        tempValues.push_back("true");
                    }
					
                    if (value == false) {
                        tempValues.push_back("false");
                    }
				}
            }
            else {
				for (auto& value : values) {
					tempValues.push_back(std::to_string(value));
				}
            }

            set(key, tempValues);
		}

        // set method for strings.
        void set(const std::string& key, const std::string& value);

        // Added by Darkness
        // Set method for string arrays.
		void Config::set(const std::string& key, const std::vector<std::string>& values)
		{
			std::string valuesTmp;
			bool first{ true };
			for (auto& value : values) {
				if (first) {
					valuesTmp += value;
					first = false;
				}
				else {
					valuesTmp += ", " + value;
				}
			}

			set(key, valuesTmp);
		}

        auto& get_key_values() {
            return m_key_values;
        }

        const auto& get_key_values() const {
            return m_key_values;
        }

    private:
        std::map<std::string, std::string> m_key_values;
        
        // Added by Darkness
        const std::string m_config_path;
    };
}
