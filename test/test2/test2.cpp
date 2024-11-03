#include "trampoline.hpp"
#include <iostream>

typedef int (* callback_function_t)(int, void*, float);

int test_callback(callback_function_t cb)
{
    int i = 0;
    while (cb(i, 0, i)) i++;
    return 0;
}

int test_once_callback(callback_function_t cb)
{
    return cb(0, 0, 0);
}

int main()
{
    std::string happy = "hello";
    auto test_cb = trampoline::c_function_ptr<callback_function_t>(
        [=](int arg1, void* arg2, float arg3)
        {
            std::cout << happy << " world " << arg3 << std::endl;

            if (arg1 > 100)
                return 0;
            return 1;
        }
    );

    test_callback(test_cb);

    // 一次性回调，自动删除自身哦！
    // lambda 会多一个 self 参数.
    // c_function_ptr 会自动推导
    auto test_once_cb_auto_delete = new trampoline::c_function_ptr<callback_function_t>(
        [=](auto self, int arg1, void* arg2, float arg3)
        {
            std::unique_ptr<std::decay_t<decltype(*self)>> auto_delete(self);
            std::cout << happy << " once " << arg3 << std::endl;
            return 0;
        }
    );

    test_once_callback(*test_once_cb_auto_delete);
    return 0;
}
