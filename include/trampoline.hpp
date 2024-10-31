
#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string.h>

#include "executable_allocator.hpp"
#include "scoped_exit.hpp"

namespace trampoline
{
	const unsigned char * _machine_code_template();
	extern "C" int trampoline_entry_code_length();
	extern "C" void * _asm_get_rax();

	////////////////////////////////////////////////////////////////////
	template<typename Signature>
	struct c_function_ptr;

	template<typename R, typename... Args>
	class dynamic_function
	{
		dynamic_function(dynamic_function&&) = delete;
		dynamic_function(dynamic_function&) = delete;

		typedef R (*function_ptr)(Args...);

		static R do_invoke(Args... args) noexcept
		{
			#if defined (__linux__) && defined (__GNUC__) && defined (__x86_64__)
			void* _rax;
			asm("mov %%rax, %0": "=r"(_rax));
			#elif defined (__aarch64__)
			void* _rax;
			asm("mov %[out], x10": [out]"=r"(_rax));
			#else
			void* _rax = _asm_get_rax();
			#endif

			dynamic_function* _this = reinterpret_cast<dynamic_function*>(_rax);

			return (*_this)(args...);
		}

		void* operator new(std::size_t size)
		{
			return ExecutableAllocator{}.allocate(size);
		}

		void operator delete(void* ptr, std::size_t size)
		{
			return ExecutableAllocator{}.deallocate(ptr, size);
		}

		template<typename LambdaFunction>
		explicit dynamic_function(LambdaFunction&& lambda)
			: ref_count(2)
			, user_function(std::forward<LambdaFunction>(lambda))
		{
			void* wrap_func_ptr = reinterpret_cast<void*>(&do_invoke);

			auto code_len = trampoline_entry_code_length();

			memcpy(_jit_code, _machine_code_template(), code_len);
			memcpy(_jit_code + code_len, &wrap_func_ptr, sizeof(wrap_func_ptr));
		}

		operator function_ptr()
		{
			return reinterpret_cast<function_ptr>(reinterpret_cast<void*>(this->_jit_code));
		}

		R operator()(Args... args) noexcept
		{
			auto a = make_scoped_exit([this]() { unref(); });

			return user_function(args...);
		}

		void unref()
		{
			if (ref_count.fetch_sub(1) == 1)
			{
				delete this;
			}
		}

		friend class c_function_ptr<R(Args...)>;

		unsigned char _jit_code[32];
		std::atomic_int ref_count;
		std::function<R(Args...)> user_function;
	};

	template<typename R, typename... Args>
	struct c_function_ptr<R(Args...)>
	{
		using wrapper_class = dynamic_function<R, Args...>;

		wrapper_class* _impl;

		template<typename LambdaFunction>
		explicit c_function_ptr(LambdaFunction&& lambda)
		{
			_impl = new wrapper_class(std::forward<LambdaFunction>(lambda));
		}

		c_function_ptr(c_function_ptr&&) = delete;
		c_function_ptr(c_function_ptr&) = delete;

		typedef R (*function_ptr)(Args...);

		operator function_ptr()
		{
			return static_cast<function_ptr>(*_impl);
		}

		void no_auto_destory()
		{
			_impl->ref_count = 0;
		}

		void destory()
		{
			delete _impl;
			_impl = nullptr;
		}

		~c_function_ptr()
		{
			if (_impl)
			{
				_impl->unref();
			}
		}
	};

	template<typename R, typename... Args>
	struct c_function_ptr<R (*)(Args...)> : public c_function_ptr<R(Args...)>
	{
		using c_function_ptr<R(Args...)>::c_function_ptr;
	};

} // namespace trampoline
