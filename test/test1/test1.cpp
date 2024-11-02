
#include "trampoline.hpp"
#include <iostream>
#include <memory>

// typedef int ( __stdcall * callback_function_t)(int, void*, float);
typedef int ( * callback_function_t)(int, void*, float);

int test_once_callback(callback_function_t cb, int d)
{
    return cb(d, 0, 2.0f);
}

int test_multi_callback(callback_function_t cb)
{
    cb(33, 0, 1222.0f);
    return cb(1, 0, 2333.0f);
}

int main()
{
    std::string happy = "hello";

    auto test_cb2 = trampoline::c_function_ptr<callback_function_t>([=](int arg1, void* arg2, float arg3)
    {
        // 这个 lambda 的 c_function_ptr 对象不是 new 出来的，因此可以不需要管理生命期
        std::cout << happy << " world " << arg3 << std::endl;

        auto test_cb3 = new trampoline::c_function_ptr<callback_function_t>([=](auto self, int arg1, void* arg2, float arg3)
        {
            // 这个 lambda 的 c_function_ptr 对象 new 出来的，
            // 因此需要第一个参数必须是 c_function_ptr* 类型，可以使用 auto 替代
            // 因此需要使用 scope_exit 自动删除 this_cb
            std::unique_ptr<std::decay_t<decltype(*self)>> auto_delete(self);

            std::cout << happy << " dynamic world " << arg1 << std::endl;
            return 0;
        });

        // return test_multi_callback(test_cb); <---- this will crash!

        return test_once_callback(*test_cb3, arg1);
    });

    return test_multi_callback(test_cb2);
}
