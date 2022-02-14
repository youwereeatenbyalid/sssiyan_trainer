class REThreadContext;
class SystemString;

namespace sdk {
typedef void (*InvokeMethod)(void* stack_frame, void* context);

class VM;
class VMContext;

VMContext* get_thread_context(int32_t unk = -1);
InvokeMethod* get_invoke_table();
}

#pragma once

#include <cstdint>
#include <string_view>
#include <exception>

namespace sdk {
// AKA via.clr.VM
class VM {
public:
    static VM* get();

public:
    REThreadContext* get_thread_context(int32_t unk = -1);

private:
    using ThreadContextFn = REThreadContext* (*)(VM*, int32_t);
    static void update_pointers();

    static VM** s_global_context;
    static ThreadContextFn s_get_thread_context;
};

class VMContext : public ::REThreadContext {
public:
    void* unhandled_exception();
    void* local_frame_gc();
    void* end_global_frame();

    void cleanup_after_exception(int32_t old_reference_count);

    class Exception : public std::exception {
    public:
        const char* what() const override {
            return "VMContext::Exception";
        }
    };

    class ScopedTranslator {
    public:
        ScopedTranslator(VMContext* context)
            : m_context{context},
            m_prev_reference_count{context->referenceCount},
            m_old_translator{_set_se_translator(ScopedTranslator::translator)}
        {
        }
        ~ScopedTranslator() {
            _set_se_translator(m_old_translator);
        }

        auto get_prev_reference_count() const {
            return m_prev_reference_count;
        }

        auto get_context() const {
            return m_context;
        }

    private:
        static void translator(unsigned int, struct ::_EXCEPTION_POINTERS*);

        const ::_se_translator_function m_old_translator;
        VMContext* m_context{};
        int32_t m_prev_reference_count{};
    };

private:
    void update_pointers();
};
}