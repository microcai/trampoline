
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
    auto test_cb = trampoline::c_function_ptr<callback_function_t>([=](int, void*, float)
    {
        std::cout << happy << " world" << std::endl;

        return 0;
    });

    return test_callback(test_cb);
}
