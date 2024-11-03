
#pragma once

#include <functional>
#include <string.h>

#include "./executable_allocator.hpp"
#include "./function.hpp"

namespace trampoline
{
	extern "C" void * _asm_get_this_pointer();

	////////////////////////////////////////////////////////////////////
	template<bool dynamic, typename Signature>
	struct c_function_ptr;

	template<bool dynamic, typename Signature>
	struct c_stdcall_function_ptr;

	class dynamic_function_base;

	struct dynamic_function_base
	{
		static constexpr long _jit_code_size = 64;

		unsigned char _jit_code[4000];
		int m_current_alloca = _jit_code_size;
		void generate_trampoline(const void* wrap_func_ptr);

		struct once_allocator
		{
			once_allocator();
			once_allocator(dynamic_function_base* parent);
			void* allocate(std::size_t size);
			void deallocate(void* ptr, int s);
		private:
			dynamic_function_base* _parent;
		};

		void* allocate_from_jit_code(int size)
		{
			// up cast to 64 bytes boundry
			size = (size / 64 + 1) * 64;
			auto old_pos = m_current_alloca;
			m_current_alloca += size;
			return _jit_code + old_pos;
		}

		once_allocator get_allocator();
	};

	template<typename ParentClass, bool hasParentClas, typename R, typename... Args>
	struct user_function_type_trait;

	template<typename ParentClass, typename R, typename... Args>
	struct user_function_type_trait<ParentClass, true, R, Args...>
	{
		using user_function_type = dr::function<R(ParentClass*, Args...), dynamic_function_base::once_allocator>;
	};

	template<typename ParentClass, typename R, typename... Args>
	struct user_function_type_trait<ParentClass, false, R, Args...>
	{
		using user_function_type = dr::function<R(Args...), dynamic_function_base::once_allocator>;
	};

	template<typename ParentClass, bool hasParentClassArg, bool is_stdcall, typename R, typename... Args>
	class dynamic_function : public dynamic_function_base
	{
		dynamic_function(dynamic_function&&) = delete;
		dynamic_function(dynamic_function&) = delete;

		using user_function_type = typename user_function_type_trait<ParentClass, hasParentClassArg, R, Args...>::user_function_type;

		void* operator new(std::size_t size)
		{
			return ExecutableAllocator{}.allocate(size);
		}

		void operator delete(void* ptr, std::size_t size)
		{
			return ExecutableAllocator{}.deallocate(ptr, size);
		}

		template<typename LambdaFunction>
		dynamic_function(ParentClass* parent, LambdaFunction&& lambda)
			: parent(parent)
			, user_function(std::forward<LambdaFunction>(lambda), get_allocator())
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

		ParentClass* parent;
		user_function_type user_function;
	};

	template<bool dynamic, typename R, typename... Args>
	struct c_function_ptr<dynamic, R(Args...)>
	{
		typedef R (*function_ptr_t)(Args...);

		using wrapper_class = dynamic_function<c_function_ptr, dynamic, false, R, Args...>;
		using user_function_type = typename wrapper_class::user_function_type;

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

	template<bool dynamic, typename R, typename... Args>
	struct c_function_ptr<dynamic, R (*)(Args...)> : public c_function_ptr<dynamic, R(Args...)>
	{
		using c_function_ptr<dynamic, R(Args...)>::c_function_ptr;
	};

#ifdef _M_IX86
	template<bool dynamic, typename R, typename... Args>
	struct c_stdcall_function_ptr<dynamic, R(Args...)>
	{
		typedef R ( __stdcall *function_ptr_t)(Args...);

		using wrapper_class = dynamic_function<c_stdcall_function_ptr, dynamic, true, R, Args...>;

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

	template<bool dynamic, typename R, typename... Args>
	struct c_function_ptr<dynamic, R (__stdcall *)(Args...)> : public c_stdcall_function_ptr<dynamic, R(Args...)>
	{
		using c_stdcall_function_ptr<R(Args...)>::c_stdcall_function_ptr;
	};
#endif

	template<typename  CallbackSignature, typename RealCallable>
	auto make_once_function(RealCallable&& callable)
	{
		struct function_wrapper
		{
			c_function_ptr<true, CallbackSignature>* wrappered_function;

			using function_ptr_t = typename c_function_ptr<true, CallbackSignature>::function_ptr_t;

			operator function_ptr_t()
			{
				return static_cast<function_ptr_t>(*wrappered_function);
			}
		};

		auto dynamic_allocated_function = new c_function_ptr<true, CallbackSignature>(std::forward<RealCallable>(callable));

		return function_wrapper{dynamic_allocated_function};
	}

	template<typename  CallbackSignature, typename RealCallable>
		requires std::convertible_to<RealCallable, typename c_function_ptr<false, CallbackSignature>::user_function_type>
	auto make_function(RealCallable&& callable)
	{
		return c_function_ptr<false, CallbackSignature>(std::forward<RealCallable>(callable));
	}

	template<typename  CallbackSignature, typename RealCallable>
		requires std::convertible_to<RealCallable, typename c_function_ptr<true, CallbackSignature>::user_function_type>
	auto make_function(RealCallable&& callable)
	{
		return make_once_function<CallbackSignature>(std::forward<RealCallable>(callable));
	}


} // namespace trampoline
