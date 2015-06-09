
/*
 *  Copyright (C) 2015 Naios <naios-dev@live.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * The functional unwrap trait provides the possibility to extract
 * various types out of c++ functional types, like std::function, functors and more.
 */

#ifndef _FUNCTIONAL_UNWRAP_HPP_
#define _FUNCTIONAL_UNWRAP_HPP_

#include <functional>
#include <tuple>

namespace fu
{
    namespace detail
    {
        template<typename Function>
        struct unwrap_function_impl;

        template<typename _RTy, typename... _ATy>
        struct unwrap_function_impl<_RTy(_ATy...)>
        {
            /// The return type of the function.
            typedef _RTy return_type;

            /// The argument types of the function as pack in std::tuple.
            typedef std::tuple<_ATy...> argument_type;

            /// The function provided as std::function
            typedef std::function<_RTy(_ATy...)> function_type;

            /// The function provided as function_ptr
            typedef _RTy(*function_ptr)(_ATy...);
        };

        /// STL: std::function
        template<typename _RTy, typename... _ATy>
        struct unwrap_function_impl<std::function<_RTy(_ATy...)>>
            : unwrap_function_impl<_RTy(_ATy...)> { };

        /// STL: std::tuple
        template<typename _RTy, typename... _ATy>
        struct unwrap_function_impl<std::tuple<_RTy, _ATy...>>
            : unwrap_function_impl<_RTy(_ATy...)> { };

        /// Function pointers
        template<typename _RTy, typename... _ATy>
        struct unwrap_function_impl<_RTy(*const)(_ATy...)>
            : unwrap_function_impl<_RTy(_ATy...)> { };

        /// Class Method pointers
        template<typename _CTy, typename _RTy, typename... _ATy>
        struct unwrap_function_impl<_RTy(_CTy::*)(_ATy...) const>
            : unwrap_function_impl<_RTy(_ATy...)> { };

        /// Unwrap through pointer of functor.
        template<typename Function>
        static auto select_best_unwrap_unary_arg(int)
            -> unwrap_function_impl<decltype(&Function::operator())>;

        /// Unwrap through plain type.
        template<typename Function>
        static auto select_best_unwrap_unary_arg(long)
            -> unwrap_function_impl<Function>;

        template<typename... _FTy>
        struct select_best_unwrap;

        /// Enable only if 1 template argument is given.
        template<typename _FTy>
        struct select_best_unwrap<_FTy>
        {
            typedef decltype(select_best_unwrap_unary_arg<_FTy>(0)) type;
        };

        // Enable if more then 1 template argument is given.
        // (Handles lazy typing)
        template<typename _RTy, typename... _ATy>
        struct select_best_unwrap<_RTy, _ATy...>
        {
            typedef unwrap_function_impl<_RTy(_ATy...)> type;
        };

     } // detail

    /// Trait to unwrap function parameters of various types:
    /// Function style definition: Result(Parameters...)
    /// STL `std::function` : std::function<Result(Parameters...)>
    /// STL `std::tuple` : std::tuple<Result, Parameters...>
    /// C++ Function pointers: `Result(*)(Parameters...)`
    /// C++ Class method pointers: `Result(Class::*)(Parameters...)`
    /// Lazy typed signatures: `Result, Parameters...`
    /// Also provides optimized unwrap of functors and functional objects.
    template<typename... Function>
    using unwrap_function =
        typename detail::select_best_unwrap<Function...>::type;

    /// Trait which defines the return type of the function.
    template<typename... Function>
    using return_type_of_t =
        typename detail::select_best_unwrap<Function...>::type::return_type;

    /// Trait which defines the argument types of the function packed in std::tuple.
    template<typename... Function>
    using argument_type_of_t =
        typename detail::select_best_unwrap<Function...>::type::argument_type;

    /// Trait which defines the std::function type of the function.
    template<typename... Function>
    using function_type_of_t =
        typename detail::select_best_unwrap<Function...>::type::function_type;

    /// Trait which defines the function pointer type of the function.
    template<typename... Function>
    using function_ptr_of_t =
        typename detail::select_best_unwrap<Function...>::type::function_ptr;

} // fu

#endif // _FUNCTIONAL_UNWRAP_HPP_