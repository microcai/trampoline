
#pragma once

#include <memory>
#include <functional>
#include <iostream>
#include <string.h>

#include "./executable_allocator.hpp"
#include "./scoped_exit.hpp"

#ifdef __MSC_VER
#define __NO_STACK_PROTECT __declspec(safebuffers)
#elif defined(__GNUC__) || defined(__clang__)
#define __NO_STACK_PROTECT __attribute__((no_stack_protector))
#else
#define __NO_STACK_PROTECT
#endif

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

		using ParentClass = c_function_ptr<R(Args...)>;

		using user_function_no_this_type = std::function<R(Args...)>;
		using user_function_with_this_type = std::function<R(ParentClass*, Args...)>;

		typedef R (*function_ptr)(Args...);

		__NO_STACK_PROTECT static R do_invoke(Args... args) noexcept
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

		template<typename LambdaFunction> requires std::convertible_to<LambdaFunction, user_function_no_this_type>
		dynamic_function(ParentClass* parent, LambdaFunction&& lambda)
			: parent(parent)
			, user_function_no_this(std::forward<LambdaFunction>(lambda))
		{
			void* wrap_func_ptr = reinterpret_cast<void*>(&do_invoke);

			auto code_len = trampoline_entry_code_length();

			memcpy(_jit_code, _machine_code_template(), code_len);
			memcpy(_jit_code + code_len, &wrap_func_ptr, sizeof(wrap_func_ptr));
			ExecutableAllocator{}.protect(this, sizeof (*this));
		}

		template<typename LambdaFunction> requires std::convertible_to<LambdaFunction, user_function_with_this_type>
		dynamic_function(ParentClass* parent, LambdaFunction&& lambda)
			: parent(parent)
			, user_function(std::forward<LambdaFunction>(lambda))
		{
			void* wrap_func_ptr = reinterpret_cast<void*>(&do_invoke);

			auto code_len = trampoline_entry_code_length();

			memcpy(_jit_code, _machine_code_template(), code_len);
			memcpy(_jit_code + code_len, &wrap_func_ptr, sizeof(wrap_func_ptr));
			ExecutableAllocator{}.protect(this, sizeof (*this));
		}

		operator function_ptr()
		{
			return reinterpret_cast<function_ptr>(reinterpret_cast<void*>(this->_jit_code));
		}

		__NO_STACK_PROTECT R operator()(Args... args) noexcept
		{
			if (user_function)
				return user_function(parent, args...);
			return user_function_no_this(args...);
		}

		friend class c_function_ptr<R(Args...)>;

		unsigned char _jit_code[32];
		ParentClass* parent;
		user_function_no_this_type user_function_no_this;
		user_function_with_this_type user_function;
	};

	template<typename R, typename... Args>
	struct c_function_ptr<R(Args...)>
	{
		using wrapper_class = dynamic_function<R, Args...>;

		wrapper_class* _impl;

		template<typename LambdaFunction>
		explicit c_function_ptr(LambdaFunction&& lambda)
			: _impl(new wrapper_class(this, std::forward<LambdaFunction>(lambda)))
		{
		}

		c_function_ptr(c_function_ptr&&) = delete;
		c_function_ptr(const c_function_ptr&) = delete;

		typedef R (*function_ptr)(Args...);

		operator function_ptr()
		{
			return static_cast<function_ptr>(*_impl);
		}

		~c_function_ptr()
		{
			delete _impl;
		}
	};

	template<typename R, typename... Args>
	struct c_function_ptr<R (*)(Args...)> : public c_function_ptr<R(Args...)>
	{
		using c_function_ptr<R(Args...)>::c_function_ptr;
	};

} // namespace trampoline
