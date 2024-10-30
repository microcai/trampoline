
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

	////////////////////////////////////////////////////////////////////
	template<typename Signature>
	struct c_function_ptr;

	template<typename R, typename... Args>
	class dynamic_function
	{
		typedef R (*function_ptr)(Args...);

		static R do_invoke(Args... args)
		{
			void* _rax;
			asm("\t mov %%rax,%0" : "=r"(_rax));

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

			memcpy(_jit_code, _machine_code_template(), 16);
			memcpy(_jit_code + 16, &wrap_func_ptr, 8);
		}

		operator function_ptr()
		{
			return reinterpret_cast<function_ptr>(this->_jit_code);
		}

		R operator()(Args... args)
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

		unsigned char _jit_code[24];
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
