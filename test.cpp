
#include "fluent++.hpp"

#include "Callback.h"
#include "CallbackContainer.h"

#include <iostream>
#include <exception>

void CastSpell(int id, Callback<bool> const& callback)
{
    std::cout << "Cast " << id << std::endl;

    // on success call true
    callback(true);
}

void MoveTo(int point, Callback<bool> const& callback)
{
    std::cout << "Move to point " << point << std::endl;

    // on success call true
    callback(true);
}

int main(int argc, char** argv)
{
    make_waterfall<Callback<bool>>()
        .then(std::bind(&CastSpell, 71382, std::placeholders::_1))
        .then([](bool success, Callback<bool> const& callback)
        {
            MoveTo(1, callback);
        })
        .then([](bool success)
        {
            // Do something final
            std::cout << "finish everything" << std::endl;
        });

    typedef Callback<bool> cbd1;
    typedef WeakCallback<int> cbd2;
    typedef SharedCallback<std::string> cbd3;

    cbd1 _cbd1;
    cbd2 _cbd2;
    cbd3 _cbd3;

    auto _test_make_shared_callback = make_shared_callback([](bool)
    {
    
    });

    Callback<> weak_cb_test;

    {
        CallbackContainer callback;

        std::shared_ptr<int> dealloc_test(new int{2}, [](int* me)
        {
            std::cout << "dealloc ok" << std::endl;
            delete me;
        });

        auto const cb_void = callback([dealloc_test]
        {
            std::cout << "huhu i'm a..." << std::endl;

            auto const cp = dealloc_test;
        });

        dealloc_test.reset();

        auto const cb_bool = callback([](bool success)
        {
            std::cout << "...weak wrapped callback!" << std::endl;
        });

        weak_cb_test = callback([]
        {
            std::cout << "huhu i'm crashsafe (you wont see me)!" << std::endl;
            std::logic_error("bad logic");
        });

        cb_void();

        int i = 0;
        ++i;

        cb_bool(true);
    }

    // This will never be executed because the CallbackContainer was deallocated and its weak callback proxies are crash safe.
    weak_cb_test();

    return 0;
}