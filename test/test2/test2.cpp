#include <iostream>
#include <memory>

import trampoline;

typedef int (* callback_function_t)(int, int, float);

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
    auto test_cb = trampoline::make_function<callback_function_t>(
        [=](int arg1, int arg2, float arg3)
        {
            std::cout << happy << " world " << arg3 << std::endl;

            if (arg1 > 100)
                return 0;
            return 1;
        }
    );

    test_callback(test_cb);

    // newed_function 是 new 出来的
    // 为了不泄漏资源，需要 进行 delete 操作。
    // 可以在回调里对 self 进行 delete
    // lambda 会多一个 self 参数
    // 因此本函数只能被调用一次
    // 当然其实多调用也可以，就是对 self 的删除要选择合适的时机
    auto newed_function = trampoline::new_function<callback_function_t>(
        [=](auto self, int arg1, int arg2, float arg3)
        {
            std::unique_ptr<std::decay_t<decltype(*self)>> auto_delete(self);
            std::cout << happy << " once " << arg3 << std::endl;
            return 0;
        }
    );

    test_once_callback(*newed_function);

    // 设置回调的时候，不想因为 new 出来的导致转换为 C 对象的时候要解引用，可以选这个版本
    // 注意这个版本的 test_once_cb_auto_delete 也是个指针，也需要对 self 进行 delete
    auto test_once_cb_auto_delete = trampoline::make_function<callback_function_t>(
        [=](auto self, int arg1, int arg2, float arg3)
        {
            std::unique_ptr<std::decay_t<decltype(*self)>> auto_delete(self);
            std::cout << happy << " once " << arg3 << std::endl;
            return 0;
        }
    );

    test_once_callback(test_once_cb_auto_delete);
}
