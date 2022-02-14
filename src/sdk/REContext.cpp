#include <shared_mutex>
#include <spdlog/spdlog.h>

#include "utility/Scan.hpp"
#include "utility/Module.hpp"

#include "ModFramework.hpp"
#include "ReClass.hpp"
#include "REContext.hpp"

namespace sdk {
    VM** VM::s_global_context{ nullptr };
    VM::ThreadContextFn VM::s_get_thread_context{ nullptr };

    sdk::VM* VM::get() {
        update_pointers();
        return *s_global_context;
    }

    REThreadContext* VM::get_thread_context(int32_t unk /*= -1*/) {
        update_pointers();

        return s_get_thread_context(this, unk);
    }

    static std::shared_mutex s_mutex{};

    void VM::update_pointers() {
        {
            // Lock a shared lock for the s_mutex
            std::shared_lock lock(s_mutex);

            if (s_global_context != nullptr && s_get_thread_context != nullptr) {
                return;
            }
        }

        // Create a unique lock for the s_mutex as we get to the meat of the function
        std::unique_lock lock{ s_mutex };

        spdlog::info("[VM::update_pointers] Updating...");

        // Version 1
        //auto ref = utility::scan(g_framework->getModule().as<HMODULE>(), "48 8B 0D ? ? ? ? BA FF FF FF FF E8 ? ? ? ? 48 89 C3");

        auto mod = g_framework->get_module().as<HMODULE>();
        auto start = (uintptr_t)mod;
        auto end = (uintptr_t)start + *utility::get_module_size(mod);

        std::unordered_map<uintptr_t, uint32_t> references{};

        struct CtxPattern {
            int32_t ctx_offset;
            int32_t get_thread_context_offset;
            std::string pattern;
        };

        CtxPattern pattern = { 3, 13, "48 8B 0D ? ? ? ? BA FF FF FF FF E8 ? ? ? ?" }; // Version 2 Dec 17th, 2019, first ptr is at game.exe+0x7095E08

        std::optional<Address> ref{};
        const CtxPattern* context_pattern{nullptr};

        ref = {};
        references.clear();

        const auto& pat = pattern.pattern;

        for (auto i = utility::scan(start, end - start, pat); i.has_value(); i = utility::scan(*i + 1, end - (*i + 1), pat)) {
            auto potential_ctx_ref = utility::calculate_absolute(*i + 3);

            references[potential_ctx_ref]++;

            // this is for sure the right one
            if (references[potential_ctx_ref] > 10) {
                ref = *i;
                context_pattern = &pattern;
                break;
            }
        }

        if (!ref || *ref == nullptr) {
            spdlog::info("[VM::update_pointers] Unable to find ref.");
            return;
        }

        s_global_context = (decltype(s_global_context))utility::calculate_absolute(*ref + context_pattern->ctx_offset);
        s_get_thread_context = (decltype(s_get_thread_context))utility::calculate_absolute(*ref + context_pattern->get_thread_context_offset);

        spdlog::info("[VM::update_pointers] s_get_thread_context: {:x}", (uintptr_t)s_get_thread_context);
    }

    sdk::VMContext* get_thread_context(int32_t unk /*= -1*/) {
        auto global_context = VM::get();

        if (global_context == nullptr) {
            return nullptr;
        }

        return (sdk::VMContext*)global_context->get_thread_context(unk);
    }

    static std::shared_mutex s_pointers_mtx{};
    static void* (*s_context_unhandled_exception_fn)(::REThreadContext*) = nullptr;
    static void* (*s_context_local_frame_gc_fn)(::REThreadContext*) = nullptr;
    static void* (*s_context_end_global_frame_fn)(::REThreadContext*) = nullptr;

    void sdk::VMContext::update_pointers() {
        {
            std::shared_lock _{s_pointers_mtx};

            if (s_context_unhandled_exception_fn != nullptr && s_context_local_frame_gc_fn != nullptr && s_context_end_global_frame_fn != nullptr) {
                return;
            }
        }
        
        std::unique_lock _{s_pointers_mtx};

        spdlog::info("Locating funcs");
        
        // Version 1
        //auto ref = utility::scan(g_framework->getModule().as<HMODULE>(), "48 83 78 18 00 74 ? 48 89 D9 E8 ? ? ? ? 48 89 D9 E8 ? ? ? ?");

        // Version 2 Dec 17th, 2019 game.exe+0x20437C (works on old version too)
        auto ref = utility::scan(g_framework->get_module().as<HMODULE>(), "48 83 78 18 00 74 ? 48 ? ? E8 ? ? ? ? 48 ? ? E8 ? ? ? ? 48 ? ? E8 ? ? ? ?");

        if (!ref) {
            spdlog::error("We're going to crash");
            return;
        }

        s_context_unhandled_exception_fn = Address{ utility::calculate_absolute(*ref + 11) }.as<decltype(s_context_unhandled_exception_fn)>();
        s_context_local_frame_gc_fn = Address{ utility::calculate_absolute(*ref + 19) }.as<decltype(s_context_local_frame_gc_fn)>();
        s_context_end_global_frame_fn = Address{ utility::calculate_absolute(*ref + 27) }.as<decltype(s_context_end_global_frame_fn)>();

        spdlog::info("Context::UnhandledException {:x}", (uintptr_t)s_context_unhandled_exception_fn);
        spdlog::info("Context::LocalFrameGC {:x}", (uintptr_t)s_context_local_frame_gc_fn);
        spdlog::info("Context::EndGlobalFrame {:x}", (uintptr_t)s_context_end_global_frame_fn);
    }

    void* sdk::VMContext::unhandled_exception() {
        update_pointers();

        return s_context_unhandled_exception_fn(this);
    }

    void* sdk::VMContext::local_frame_gc() {
        update_pointers();

        return s_context_local_frame_gc_fn(this);
    }

    void* sdk::VMContext::end_global_frame() {
        update_pointers();

        return s_context_end_global_frame_fn(this);
    }

    void sdk::VMContext::cleanup_after_exception(int32_t old_reference_count) {
        auto& reference_count = this->referenceCount;
        auto count_delta = reference_count - old_reference_count;

        spdlog::error("{}", reference_count);
        if (count_delta >= 1) {
            --reference_count;

            // Perform object cleanup that was missed because an exception occurred.
            if (this->unkPtr != nullptr && this->unkPtr->unkPtr != nullptr) {
                this->unhandled_exception();
            }

            this->local_frame_gc();
            this->end_global_frame();
        } else if (count_delta == 0) {
            spdlog::info("No fix necessary");
        }
    }

    void sdk::VMContext::ScopedTranslator::translator(unsigned int code, struct ::_EXCEPTION_POINTERS* exc) {
        spdlog::info("VMContext: Caught exception code {:x}", code);

        switch (code) {
        case EXCEPTION_ACCESS_VIOLATION:
            spdlog::info("VMContext: Attempting to handle access violation.");

        default:
            break;
        }

        throw sdk::VMContext::Exception{};
    }
}


