# 为 C 接口的回调增加 lambda 支持。甚至它不需要 `void *user_data` 参数！

```c++
#include "trampoline.hpp"
#include <iostream>

typedef int (* callback_function_t)(int, void*, float);

int test_callback(callback_function_t cb)
{
    return cb(1, 0, 2.0f);
}

int main()
{
    std::string happy = "hello";
    auto test_cb = trampoline::c_function_ptr<callback_function_t>([=](int arg1, void* arg2, float arg3)
    {
        std::cout << happy << " world " << arg3 << std::endl;

        return 0;
    });

    return test_callback(test_cb);
}
```

# 支持的平台

目前支持的平台为  x86_64/x86/arm64



# add lambda support for C style callback that does not even have `void* user_data` parameter.

