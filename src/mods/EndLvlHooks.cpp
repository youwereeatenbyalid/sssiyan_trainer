#include "EndLvlHooks.hpp"

EndLvlHooks::EndLvlReset::EndLvlReset()
{
	EndLvlHooks::subscribe_reset_func(this, &EndLvlReset::reset);
}