#pragma once
#include <string>
#include <vector>

namespace GameListEditor
{
	template <typename T>
	class ListManager
	{
	private:
		T indx;
		char* fixedName;
	public:
		const char* get_fixed_name() const
		{
			return fixedName;
		}
		void set_fixed_name(const char* name)
		{
			fixedName = name;
		}
	};

	template <typename T>

}


