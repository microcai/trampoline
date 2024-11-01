
#pragma once

#include <utility>

namespace trampoline
{
	template<typename Callback> class scoped_exit
	{
	public:
		template<typename C> scoped_exit(C&& c) : callback(std::forward<C>(c))
		{
		}

		scoped_exit(scoped_exit&& mv) : callback(std::move(mv.callback)), canceled(mv.canceled)
		{
			mv.canceled = true;
		}

		scoped_exit(const scoped_exit&) = delete;
		scoped_exit& operator=(const scoped_exit&) = delete;

		~scoped_exit()
		{
			if (!canceled)
			{
				try
				{
					callback();
				}
				catch (...)
				{
				}
			}
		}

		void cancel()
		{
			canceled = true;
		}

	private:
		Callback callback;
		bool canceled = false;
	};

	template<typename Callback>
	scoped_exit<Callback> make_scoped_exit(Callback&& c)
	{
		return scoped_exit<Callback>(std::forward<Callback>(c));
	}

} // namespace trampoline
