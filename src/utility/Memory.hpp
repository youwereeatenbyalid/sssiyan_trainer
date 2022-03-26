#pragma once

#include <cstdint>
#include <reframework/shared/utility/Memory.hpp>

namespace utility {
    // Template argument is the type of the pointer that is going to be returned
    template<typename T, typename... Args, class Enabled = std::enable_if_t<(... && std::is_convertible_v<Args, int64_t>)>>
    T* followPtrChain(void* base, int64_t offset, Args... offsetList) noexcept
    {
        if (!isGoodReadPtr(reinterpret_cast<uintptr_t>(base), sizeof(base)))
        {
            return nullptr;
        }

        auto addr = static_cast<uint8_t*>(base) + offset;

        if constexpr (!sizeof...(offsetList)) {
            if (!isGoodReadPtr(reinterpret_cast<uintptr_t>(addr), sizeof(addr)))
            {
                return nullptr;
            }

            return reinterpret_cast<T*>(addr);
        }
        else {
            return FollowPtrChain<T>(*reinterpret_cast<void**>(addr), offsetList...);
        }
    }
}
