# 为 C 接口的回调增加 lambda 支持。甚至它不需要 `void *user_data` 参数！

```c++
#include "trampoline.hpp"
#include <iostream>

typedef int (* callback_function_t)(int, void*, float);

int test_callback(callback_function_t cb)
{
    int i = 0;
    while (cb(i, 0, i));
    return 0;
}

int test_once_callback(callback_function_t cb)
{
    return cb(i, 0, i)
}

int main()
{
    std::string happy = "hello";
    auto test_cb = trampoline::c_function_ptr<callback_function_t>(
        [=](int arg1, void* arg2, float arg3)
        {
            std::cout << happy << " world " << arg3 << std::endl;

            return 0;
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

    test_once_callback(test_once_cb_auto_delete);

}
```

# 支持的平台

目前支持的平台为  x86_64/x86/arm64


# add lambda support for C style callback that does not even have `void* user_data` parameter.

