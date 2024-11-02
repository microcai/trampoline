
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
	extern "C" void * _asm_get_rax();

	////////////////////////////////////////////////////////////////////
	template<typename Signature>
	struct c_function_ptr;

	template<typename Signature>
	struct c_stdcall_function_ptr;

	class dynamic_function_base_trampoline
	{
	protected:
		unsigned char _jit_code[64];
		void  setup_trampoline(const void* wrap_func_ptr);
	};

	template<typename ParentClass, bool use_stdcall, typename R, typename... Args>
	class dynamic_function : public dynamic_function_base_trampoline
	{
		dynamic_function(dynamic_function&&) = delete;
		dynamic_function(dynamic_function&) = delete;

		using user_function_no_this_type = std::function<R(Args...)>;
		using user_function_with_this_type = std::function<R(ParentClass*, Args...)>;

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

		template<typename LambdaFunction> requires std::convertible_to<LambdaFunction, user_function_no_this_type>
		dynamic_function(ParentClass* parent, LambdaFunction&& lambda)
			: parent(parent)
			, user_function_no_this(std::forward<LambdaFunction>(lambda))
		{
			attach_trampoline();
		}

		template<typename LambdaFunction> requires std::convertible_to<LambdaFunction, user_function_with_this_type>
		dynamic_function(ParentClass* parent, LambdaFunction&& lambda)
			: parent(parent)
			, user_function(std::forward<LambdaFunction>(lambda))
		{
			attach_trampoline();
		}

		void attach_trampoline()
		{
			if constexpr (use_stdcall)
			{
				/*
				 * c++ 对象使用 __thiscall, 如果 C 函数要求 __stdcall
				 * 则 所有的参数，已经都在栈上。但是没有 this 。
				 * __thiscall 和 __stdcall 的唯一区别就是，this 要在 ecx 寄存器
				 * 里传递。剩下的都一样。包括栈清理也是 被调用者清理
				 * 于是这里 汇编代码只要利用 trampoline 技术，找到 this 送入
				 * ecx 寄存器，就可以直接调用签名和 C 函数一样的 operator()
				 * 成员函数了。
				 * 这样就可以不需要 do_invoke 的包装
				 */
				auto call_op_func = &dynamic_function::operator();
				void* raw;
				memcpy(&raw, &call_op_func, sizeof(raw));
				setup_trampoline(raw);
			}
			else
			{
#if defined (__i386__)
				setup_trampoline(reinterpret_cast<void*>(&dynamic_function::do_call));
#else
				/*
				 * 调用方使用 cdecl 调用，而 operator() 的调用约定是 thiscall
				 * 由于 cdecl 是调用方清栈，而 __thiscall 是被调用方清栈
				 * 因此只能使用 do_invoke 中转。do_invoke 利用 trampoline
				 * 技术获取 this 指针，然后调用 this->operator()
				 * 避免 this->operator() 直接被调用而引起 调用约定不匹配
				 */
				setup_trampoline(reinterpret_cast<void*>(&dynamic_function::do_invoke));
#endif
			}
		}

		~dynamic_function()
		{
			ExecutableAllocator{}.unprotect(this, sizeof (*this));
		}

		static R do_call(void* _this, void* ret, Args... args)
		{
			return (*reinterpret_cast<dynamic_function*>(_this))(args...);
		}

		void* raw_function_ptr()
		{
			return reinterpret_cast<void*>(this->_jit_code);
		}

		R operator()(Args... args) noexcept
		{
			if (user_function)
				return user_function(parent, args...);
			return user_function_no_this(args...);
		}

		friend ParentClass;

		ParentClass* parent;
		user_function_no_this_type user_function_no_this;
		user_function_with_this_type user_function;
	};

	template<typename R, typename... Args>
	struct c_function_ptr<R(Args...)>
	{
		typedef R (*function_ptr_t)(Args...);

		using wrapper_class = dynamic_function<c_function_ptr, false, R, Args...>;

		wrapper_class* _impl;

		template<typename LambdaFunction>
		explicit c_function_ptr(LambdaFunction&& lambda)
			: _impl(new wrapper_class(this, std::forward<LambdaFunction>(lambda)))
		{
		}

		c_function_ptr(c_function_ptr&&) = delete;
		c_function_ptr(const c_function_ptr&) = delete;

		operator function_ptr_t()
		{
			return reinterpret_cast<function_ptr_t>(_impl->raw_function_ptr());
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

#ifdef _M_IX86
	template<typename R, typename... Args>
	struct c_stdcall_function_ptr<R(Args...)>
	{
		typedef R ( __stdcall *function_ptr_t)(Args...);

		using wrapper_class = dynamic_function<c_stdcall_function_ptr, true, R, Args...>;

		wrapper_class* _impl;

		template<typename LambdaFunction>
		explicit c_stdcall_function_ptr(LambdaFunction&& lambda)
			: _impl(new wrapper_class(this, std::forward<LambdaFunction>(lambda)))
		{
		}

		c_stdcall_function_ptr(c_stdcall_function_ptr&&) = delete;
		c_stdcall_function_ptr(const c_stdcall_function_ptr&) = delete;


		operator function_ptr_t()
		{
			return reinterpret_cast<function_ptr_t>(_impl->raw_function_ptr());
		}

		~c_stdcall_function_ptr()
		{
			delete _impl;
		}
	};

	template<typename R, typename... Args>
	struct c_function_ptr<R (__stdcall *)(Args...)> : public c_stdcall_function_ptr<R(Args...)>
	{
		using c_stdcall_function_ptr<R(Args...)>::c_stdcall_function_ptr;
	};
#endif
} // namespace trampoline
