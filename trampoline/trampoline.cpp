module;

#include <assert.h>
#include <type_traits>
#include <utility>
#include <memory>
#include <cstring>

#ifdef _MSC_VER
#define __attribute__(x)
#else
#define __declspec(x)
#endif

export module trampoline;

import ExecutableAllocator;

extern "C" // from assembly code
{
	extern std::size_t generate_trampoline(void* jit_code_address, const void* call_target);
#ifdef _M_IX86
	extern std::size_t generate_trampoline2(void* jit_code_address, const void* call_target);
#endif
}

namespace trampoline
{
	extern "C" void * _asm_get_this_pointer()  __attribute__((no_caller_saved_registers));

	////////////////////////////////////////////////////////////////////
	export struct c_function_ptr
	{
		virtual void* raw_function_ptr(){ return nullptr; }
		virtual ~c_function_ptr(){}
	};

	template<typename Signature, typename UserFunction>
	struct c_function_ptr_impl;

	template<typename Signature, typename UserFunction>
	struct c_stdcall_function_ptr;

	struct dynamic_function_base;

	enum calling_convertion
	{
		x86_cdecl,
		x86_stdcall,
		x64_msabi,
		x64_sysvabi,
		arm64_apcs,
	};

#if defined(_M_IX86) || defined(__i386__)
	inline static constexpr auto default_calling_convertion = x86_cdecl;
#elif defined(_MSC_VER) && defined(_M_AMD64)
	inline static constexpr auto default_calling_convertion = x64_msabi;
#elif defined(__x86_64__) || (_M_AMD64)
	inline static constexpr auto default_calling_convertion = x64_sysvabi;
#elif defined(__aarch64__)
	inline static constexpr auto default_calling_convertion = arm64_apcs;
#endif

	struct dynamic_function_base
	{
		static constexpr long _jit_code_size = 64;

		unsigned char _jit_code[_jit_code_size];
		void generate_trampoline(const void* wrap_func_ptr)
		{
			auto code_size = ::generate_trampoline(_jit_code, wrap_func_ptr);
			assert(code_size <= _jit_code_size);
			ExecutableAllocator{}.protect(this, sizeof (*this));
		}
#ifdef _M_IX86
		void generate_trampoline2(const void* wrap_func_ptr)
		{
			auto code_size = ::generate_trampoline2(_jit_code, wrap_func_ptr);
			assert(code_size <= _jit_code_size);
			ExecutableAllocator{}.protect(this, sizeof(*this));
		}
#endif
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

	template<typename UserFunction, typename ParentClass , calling_convertion callabi, typename R, typename... Args>
	struct dynamic_function : public dynamic_function_base
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
#ifdef _M_IX86
			if constexpr (callabi == x86_stdcall)
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
				generate_trampoline2(raw);
			}
			else
#endif
			if constexpr (callabi == x86_cdecl)
			{
				generate_trampoline(reinterpret_cast<void*>(&dynamic_function::_callback_trunk_cdecl_x86));
			}
			else
			{
				/*
				 * 调用方使用 cdecl 调用，而 call_user_function() 的调用约定是 thiscall
				 * 调用约定不同，因此需要使用 _callback_trunk_cdecl 来“接收”调用方的参数
				 * 然后再调用 call_user_function()
				 */
				generate_trampoline(reinterpret_cast<void*>(&dynamic_function::_callback_trunk_cdecl));
			}
		}
		

#if defined (__i386__)
		__attribute__((regparm(2)))
#endif
		static R _callback_trunk_cdecl_x86(void* _this, const void* const ret_address, Args... args)
		{
			// 使用了 __attribute__((regparm(2))) 后
			// _callback_trunk_cdecl_x86 会认为前两个参数，是用 寄存器传递的
			// 剩下的参数，用栈传递
			// 于是，虽然加了两个参数，但是这个函数的调用约定本质上
			// 和  R(Args...) 是一模一样的！
			// 也就是通过编译器白嫖了一个 _this 参数
			// 于是就免去了对 _asm_get_this_pointer 的调用
			// 并且编译器生成的 prologue 会绝对避开 EAX 寄存器
			// 这正是之前的版本里莫名其妙的 崩溃 的由来。
			// 原来 EAX 会在 prologue 里，调用 _asm_get_this_pointer 前就被污染
			// 所以改用 __attribute__((regparm(2))) 就避免了 EAX 被污染
			// 而且让编译器绝对的相信 EAX 是 this

			// 至于 win32，则在汇编里使用 push 两次，将 this 和真正的返回地址都压栈
			// 这样就等于多压了2个参数. 主要是 msvc 没有 __attribute__ 功能
			// 然后在返回的地方，进行栈平齐操作后，再返回到真正的调用处。
			return reinterpret_cast<dynamic_function*>(_this)->call_user_function(std::forward<Args>(args)...);
		}

		static R _callback_trunk_cdecl(Args... args) noexcept
		{
			dynamic_function* _this = reinterpret_cast<dynamic_function*>(_asm_get_this_pointer());
			return _this->call_user_function(std::forward<Args>(args)...);
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
				return user_function(parent, std::forward<Args>(args)...);
			}
			else
			{
				return user_function(std::forward<Args>(args)...);
			}
		}

		ParentClass* parent;
		UserFunction user_function;
	};

	template <calling_convertion callabi, typename function_ptr_t, typename DerivedClass, typename UserFunction, typename R, typename... Args>
	struct c_function_ptr_base : public c_function_ptr
	{
		using wrapper_class = dynamic_function<UserFunction, DerivedClass, callabi, R, Args...>;

		std::unique_ptr<wrapper_class> _impl;

		c_function_ptr_base(c_function_ptr_base&&) = default;
		c_function_ptr_base(const c_function_ptr_base&) = delete;

		explicit c_function_ptr_base(UserFunction&& lambda)
			: _impl(new wrapper_class(static_cast<DerivedClass*>(this), std::forward<UserFunction>(lambda)))
		{
		}

		void* raw_function_ptr()
		{
			return  _impl->raw_function_ptr();
		}

		operator function_ptr_t()
		{
			return reinterpret_cast<function_ptr_t>(raw_function_ptr());
		}

	};

	template<typename UserFunction, typename R, typename... Args>
	struct c_function_ptr_impl<R(Args...), UserFunction> : public c_function_ptr_base<default_calling_convertion, R(*)(Args...) ,c_function_ptr_impl<UserFunction, R(Args...)>, UserFunction, R, Args...>
	{
		typedef R (*function_ptr_t)(Args...);
		using c_function_ptr_base<default_calling_convertion, R(*)(Args...) ,c_function_ptr_impl<UserFunction, R(Args...)>, UserFunction, R, Args...>::c_function_ptr_base;
	};

	template<typename UserFunction, typename R, typename... Args>
	struct c_function_ptr_impl<R(*)(Args...), UserFunction> : public c_function_ptr_base<default_calling_convertion, R(*)(Args...) ,c_function_ptr_impl<R(*)(Args...), UserFunction>, UserFunction, R, Args...>
	{
		typedef R (*function_ptr_t)(Args...);
		using c_function_ptr_base<default_calling_convertion, R(*)(Args...) ,c_function_ptr_impl<R(*)(Args...), UserFunction>, UserFunction, R, Args...>::c_function_ptr_base;
	};

#ifdef _M_IX86
	template<typename UserFunction, typename R, typename... Args>
	struct c_function_ptr_impl<R( __stdcall *)(Args...), UserFunction> : public c_function_ptr_base<x86_stdcall, R(__stdcall *)(Args...) , c_function_ptr_impl<R( __stdcall *)(Args...), UserFunction>, UserFunction, R, Args...>
	{
		typedef R (__stdcall *function_ptr_t)(Args...);
		using c_function_ptr_base<x86_stdcall, R(__stdcall *)(Args...) , c_function_ptr_impl<R( __stdcall *)(Args...), UserFunction>, UserFunction, R, Args...>::c_function_ptr_base;
	};
#endif

	export template<typename  CallbackSignature, typename RealCallable>
	auto new_function(RealCallable&& callable)
	{
		return new c_function_ptr_impl<CallbackSignature, RealCallable>(std::forward<RealCallable>(callable));
	}

	template<typename  CallbackSignature, typename RealCallable>
	concept has_self_as_first_arg = user_function_type_trait_has_parent_class<
			c_function_ptr_impl<CallbackSignature, RealCallable>, RealCallable, CallbackSignature
		>::value;

	export template<typename  CallbackSignature, typename RealCallable>
		requires (!has_self_as_first_arg<CallbackSignature, RealCallable>)
	auto make_function(RealCallable&& callable)
	{
		return c_function_ptr_impl<CallbackSignature, RealCallable>(std::forward<RealCallable>(callable));
	}

	export template<typename function_ptr_t>
	struct function_wrapper
	{
		c_function_ptr* wrappered_function;
		operator function_ptr_t()
		{
			return reinterpret_cast<function_ptr_t>(wrappered_function->raw_function_ptr());
		}
	};

	export template<typename  CallbackSignature, typename RealCallable>
		requires (has_self_as_first_arg<CallbackSignature, RealCallable>)
	auto make_function(RealCallable&& callable)
	{
		using function_ptr_t = typename c_function_ptr_impl<CallbackSignature, RealCallable>::function_ptr_t;

		return function_wrapper<function_ptr_t>{ new_function<CallbackSignature>(std::forward<RealCallable>(callable)) };
	}


} // namespace trampoline


