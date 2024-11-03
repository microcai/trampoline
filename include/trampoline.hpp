
#pragma once

#include <type_traits>
#include <utility>
#include <memory>
#include <cstring>

#include "./executable_allocator.hpp"

namespace trampoline
{
	extern "C" void * _asm_get_this_pointer();

	////////////////////////////////////////////////////////////////////
	struct c_function_ptr
	{
		virtual ~c_function_ptr(){}
	};

	template<typename Signature, typename UserFunction>
	struct c_function_ptr_impl;

	template<typename Signature, typename UserFunction>
	struct c_stdcall_function_ptr;

	class dynamic_function_base;

	struct dynamic_function_base
	{
		static constexpr long _jit_code_size = 64;

		unsigned char _jit_code[_jit_code_size];
		int m_current_alloca = _jit_code_size;
		void generate_trampoline(const void* wrap_func_ptr);

		void* operator new(std::size_t size)
		{
			return ExecutableAllocator{}.allocate(size);
		}

		void operator delete(void* ptr, std::size_t size)
		{
			return ExecutableAllocator{}.deallocate(ptr, size);
		}
	};

	template<typename ParentClass, typename UserFunction, typename Signature>
	struct user_function_type_trait_has_parent_class;

	template<typename ParentClass, typename UserFunction, typename R, typename... Args>
	struct user_function_type_trait_has_parent_class<ParentClass, UserFunction, R(Args...)>
	{
		static bool constexpr value = std::is_invocable_r_v<R, UserFunction, ParentClass*, Args...>;
	};

	template<typename ParentClass, typename UserFunction, typename R, typename... Args>
	struct user_function_type_trait_has_parent_class<ParentClass, UserFunction, R(*)(Args...)>
	{
		static bool constexpr value = std::is_invocable_r_v<R, UserFunction, ParentClass*, Args...>;
	};

	template<typename UserFunction, typename ParentClass , bool is_stdcall, typename R, typename... Args>
	class dynamic_function : public dynamic_function_base
	{
		dynamic_function(dynamic_function&&) = delete;
		dynamic_function(dynamic_function&) = delete;

		static bool constexpr hasParentClassArg = user_function_type_trait_has_parent_class<ParentClass, UserFunction, R(Args...)>::value;

		dynamic_function(ParentClass* parent, UserFunction&& lambda)
			: parent(parent)
			, user_function(std::forward<UserFunction>(lambda))
		{
			attach_trampoline();
		}

		void attach_trampoline()
		{
			if constexpr (is_stdcall)
			{
				/*
				 * c++ 对象使用 __thiscall, 如果 C 函数要求 __stdcall
				 * 则 所有的参数，已经都在栈上。但是没有 this 。
				 * __thiscall 和 __stdcall 的唯一区别就是，this 要在 ecx 寄存器
				 * 里传递。剩下的都一样。包括栈清理也是 被调用者清理
				 * 于是这里 汇编代码只要利用 trampoline 技术，找到 this 送入
				 * ecx 寄存器，就可以直接调用签名和 C 函数一样的 call_user_function()
				 * 成员函数了。
				 * 这样就可以不需要 _callback_trunk_cdecl 的包装
				 */
				auto call_op_func = &dynamic_function::call_user_function;
				void* raw;
				static_assert(sizeof(call_op_func) == sizeof(raw), "member function pointer size assumption failed");
				memcpy(&raw, &call_op_func, sizeof(raw));
				generate_trampoline(raw);
			}
			else
			{
#if defined (__i386__)
				generate_trampoline(reinterpret_cast<void*>(&dynamic_function::_callback_trunk_cdecl_x86));
#else
				/*
				 * 调用方使用 cdecl 调用，而 call_user_function() 的调用约定是 thiscall
				 * 调用约定不同，因此需要使用 _callback_trunk_cdecl 来“接收”调用方的参数
				 * 然后再调用 call_user_function()
				 */
				generate_trampoline(reinterpret_cast<void*>(&dynamic_function::_callback_trunk_cdecl));
#endif
			}
		}

		static R _callback_trunk_cdecl_x86(void* _this, void* ret, Args... args)
		{
			return (*reinterpret_cast<dynamic_function*>(_this))(args...);
		}

		static R _callback_trunk_cdecl(Args... args) noexcept
		{
			dynamic_function* _this = reinterpret_cast<dynamic_function*>(_asm_get_this_pointer());

			return _this->call_user_function(args...);
		}

		~dynamic_function()
		{
			ExecutableAllocator{}.unprotect(this, sizeof (*this));
		}

		void* raw_function_ptr()
		{
			return reinterpret_cast<void*>(this->_jit_code);
		}

		R call_user_function(Args... args) noexcept
		{
			if constexpr (hasParentClassArg)
			{
				return user_function(parent, args...);
			}
			else
			{
				return user_function(args...);
			}
		}

		friend ParentClass;
		friend std::unique_ptr<dynamic_function>::deleter_type;

		ParentClass* parent;
		UserFunction user_function;
	};

	template<typename UserFunction, typename R, typename... Args>
	struct c_function_ptr_impl<R(Args...), UserFunction> : public c_function_ptr
	{
		typedef R (*function_ptr_t)(Args...);

		using wrapper_class = dynamic_function<UserFunction, c_function_ptr_impl, false, R, Args...>;

		std::unique_ptr<wrapper_class> _impl;

		c_function_ptr_impl(c_function_ptr_impl&&) = default;
		c_function_ptr_impl(const c_function_ptr_impl&) = delete;

		explicit c_function_ptr_impl(UserFunction&& lambda)
			: _impl(new wrapper_class(this, std::forward<UserFunction>(lambda)))
		{
		}

		function_ptr_t get_function_pointer()
		{
			return reinterpret_cast<function_ptr_t>(_impl->raw_function_ptr());
		}

		operator function_ptr_t()
		{
			return get_function_pointer();
		}
	};

	template<typename UserFunction, typename R, typename... Args>
	struct c_function_ptr_impl<R (*)(Args...), UserFunction> : public c_function_ptr_impl<R(Args...), UserFunction>
	{
		using c_function_ptr_impl<R(Args...), UserFunction>::c_function_ptr_impl;
	};

#ifdef _M_IX86
	template<typename UserFunction, typename R, typename... Args>
	struct c_stdcall_function_ptr<R(Args...), UserFunction> : public c_function_ptr_base
	{
		typedef R ( __stdcall *function_ptr_t)(Args...);

		using wrapper_class = dynamic_function<UserFunction, c_stdcall_function_ptr, true, R, Args...>;

		std::unique_ptr<wrapper_class> _impl;

		c_function_ptr_impl(c_stdcall_function_ptr&&) = default;
		c_function_ptr_impl(const c_stdcall_function_ptr&) = delete;

		explicit c_stdcall_function_ptr(UserFunction&& lambda)
			: _impl(new wrapper_class(this, std::forward<UserFunction>(lambda)))
		{
		}

		function_ptr_t get_function_pointer()
		{
			return reinterpret_cast<function_ptr_t>(_impl->raw_function_ptr());
		}

		operator function_ptr_t()
		{
			return get_function_pointer();
		}
	};

	template<typename UserFunction, typename R, typename... Args>
	struct c_function_ptr<UserFunction, R (__stdcall *)(Args...)> : public c_stdcall_function_ptr<UserFunction, R(Args...)>
	{
		using c_stdcall_function_ptr<R(Args...)>::c_stdcall_function_ptr;
	};
#endif

	template<typename  CallbackSignature, typename RealCallable>
	auto new_function(RealCallable&& callable)
	{
		return new c_function_ptr_impl<CallbackSignature, RealCallable>(std::forward<RealCallable>(callable));
	}

	template<typename  CallbackSignature, typename RealCallable>
	concept has_self_as_first_arg = user_function_type_trait_has_parent_class<
			c_function_ptr_impl<CallbackSignature, RealCallable>, RealCallable, CallbackSignature
		>::value;

	template<typename  CallbackSignature, typename RealCallable>
		requires (!has_self_as_first_arg<CallbackSignature, RealCallable>)
	auto make_function(RealCallable&& callable)
	{
		return c_function_ptr_impl<CallbackSignature, RealCallable>(std::forward<RealCallable>(callable));
	}

	template<typename  CallbackSignature, typename RealCallable>
		requires (has_self_as_first_arg<CallbackSignature, RealCallable>)
	auto make_function(RealCallable&& callable)
	{
		struct function_wrapper
		{
			c_function_ptr_impl<CallbackSignature, RealCallable>* wrappered_function;

			using function_ptr_t = typename c_function_ptr_impl<CallbackSignature, RealCallable>::function_ptr_t;

			operator function_ptr_t()
			{
				return static_cast<function_ptr_t>(*wrappered_function);
			}
		};
		return function_wrapper{ new_function<CallbackSignature>(std::forward<RealCallable>(callable)) };
	}


} // namespace trampoline
