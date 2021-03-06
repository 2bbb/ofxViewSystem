//
//  opt_arg_function.hpp
//
//  Created by ISHII 2bit on 2018/03/09.
//

#pragma once

#ifndef bbb_opt_arg_function_hpp
#define bbb_opt_arg_function_hpp

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <functional>

namespace bbb {
    namespace opt_arg_function_detail {
        namespace {
            template <bool b, typename type>
            using enable_if_t = typename std::enable_if<b, type>::type;
            template <bool b, typename t, typename f>
            using conditional_t = typename std::conditional<b, t, f>::type;

            template <typename T>
            using get_type = typename T::type;
            
            template <typename embedding_type>
            struct embedding { using type = embedding_type; };
            
            template <typename t>
            struct defer { using type = t; };
            
            template <bool b, typename t, typename f>
            using defered_conditional = conditional_t<b, defer<t>, defer<f>>;
            
            template <typename t>
            using resolve_t = get_type<t>;
        };
        
        inline namespace integer_sequences {
            template <typename type, type ... ns>
            struct integer_sequence {
                using value_type = type;
                static constexpr std::size_t size() noexcept { return sizeof...(ns); }
            };
            
            namespace detail {
                template <typename integer_type, integer_type n, integer_type ... ns>
                struct make_integer_sequence
                : embedding<resolve_t<conditional_t<
                    n == 0,
                    defer<integer_sequence<integer_type, ns ...>>,
                    detail::make_integer_sequence<integer_type, n - 1, n - 1, ns ...>
                >>> {};
            };
            
            template <typename type, type n>
            using make_integer_sequence = get_type<detail::make_integer_sequence<type, n>>;
            
            template <std::size_t ... ns>
            using index_sequence = integer_sequence<std::size_t, ns ...>;
            
            template <std::size_t n>
            using make_index_sequence = make_integer_sequence<std::size_t, n>;
            
            template <typename... types>
            using index_sequence_for = make_index_sequence<sizeof...(types)>;
        };
        
        inline namespace function_traits_utils {
            template <typename>
            struct is_function
            : std::false_type {};
            template <typename res, typename ... arguments>
            struct is_function<std::function<res(arguments ...)>>
            : std::false_type {};
            
            template <std::size_t index, typename ... arguments>
            using type_at_t = typename std::tuple_element<index, std::tuple<arguments ...>>::type;
            
            template <typename patient>
            struct has_call_operator {
                template <typename inner_patient, decltype(&inner_patient::operator())> struct checker {};
                template <typename inner_patient> static std::true_type  check(checker<inner_patient, &inner_patient::operator()> *);
                template <typename>               static std::false_type check(...);
                using type = decltype(check<patient>(nullptr));
                static constexpr bool value = type::value;
            };
            
            namespace detail {
                template <typename ret, typename ... arguments>
                struct function_traits {
                    static constexpr std::size_t arity = sizeof...(arguments);
                    using result_type = ret;
                    using arguments_types_tuple = std::tuple<arguments ...>;
                    template <std::size_t index>
                    using argument_type = type_at_t<index, arguments ...>;
                    using function_type = std::function<ret(arguments ...)>;
                    template <typename function_t>
                    static constexpr function_type cast(function_t f) {
                        return static_cast<function_type>(f);
                    }
                };
            };
            
            template <typename T>
            struct function_traits : public function_traits<decltype(&T::operator())> {};
            
            template <typename class_type, typename ret, typename ... arguments>
            struct function_traits<ret(class_type::*)(arguments ...) const>
            : detail::function_traits<ret, arguments ...> {};
            
            template <typename class_type, typename ret, typename ... arguments>
            struct function_traits<ret(class_type::*)(arguments ...)>
            : detail::function_traits<ret, arguments ...> {};
            
            template <typename ret, typename ... arguments>
            struct function_traits<ret(*)(arguments ...)>
            : detail::function_traits<ret, arguments ...> {};
            
            template <typename ret, typename ... arguments>
            struct function_traits<ret(arguments ...)>
            : detail::function_traits<ret, arguments ...> {};
            
            template <typename ret, typename ... arguments>
            struct function_traits<std::function<ret(arguments ...)>>
            : detail::function_traits<ret, arguments ...> {};
        };
        
        template <typename function_type>
        struct opt_arg_function;
        
        template <typename res, typename ... arguments>
        struct opt_arg_function<res(arguments ...)> {
            using function_type = std::function<res(arguments ...)>;
            
            opt_arg_function()
            : f([](arguments ...) -> res { return {}; }) {};
            
            opt_arg_function(const opt_arg_function &) = default;
            opt_arg_function(opt_arg_function &&) = default;
            
            opt_arg_function(const function_type &f)
            : f(f) {};
            opt_arg_function(function_type &&f)
            : f(std::move(f)) {};
            
            template <typename res_, typename ... arguments_>
            opt_arg_function(const std::function<res_(arguments_ ...)> &f_)
            : f(opt_arg_function::convert(f_, make_index_sequence<sizeof...(arguments_)>()))
            {};
            template <typename res_, typename ... arguments_>
            opt_arg_function(std::function<res_(arguments_ ...)> &&f_)
            : f(opt_arg_function::convert(std::move(f_), make_index_sequence<sizeof...(arguments_)>()))
            {};
            template <
                typename function_type,
                typename = typename std::enable_if<
                    !is_function<function_type>::value &&
                    has_call_operator<function_type>::value
                >::type
            >
            opt_arg_function(function_type f)
            : opt_arg_function(function_traits<function_type>::cast(f)){};
            
            opt_arg_function &operator=(const opt_arg_function &) = default;
            opt_arg_function &operator=(opt_arg_function &&) = default;
            
            opt_arg_function &operator=(const function_type &f) { this->f = f; return *this; };
            opt_arg_function &operator=(function_type &&f) { this->f = std::move(f); return *this; };
            
            template <typename function_type>
            auto operator=(function_type f)
            -> typename std::enable_if<
            !is_function<function_type>::value &&
            has_call_operator<function_type>::value,
            opt_arg_function &
            >::type
            { return operator=(function_traits<function_type>::cast(f)); };
            
            template <typename res_, typename ... arguments_>
            opt_arg_function &operator=(const std::function<res_(arguments_ ...)> &f) {
                this->f = opt_arg_function::convert(f, make_index_sequence<sizeof...(arguments_)>());
                return *this;
            };
            template <typename res_, typename ... arguments_>
            opt_arg_function &operator=(std::function<res_(arguments_ ...)> &&f) {
                this->f = opt_arg_function::convert(std::move(f), make_index_sequence<sizeof...(arguments_)>());
                return *this;
            };
            
            template <typename ... other_arguments>
            res operator()(arguments ... args, other_arguments && ...) const { return f(std::forward<arguments>(args) ...); };
            
            operator std::function<res(arguments ...)>() const { return f; };
            
            template <typename ... other_arguments>
            operator std::function<res(arguments ..., other_arguments && ...)>() const {
                auto &&f_ = f;
                return [f_](arguments ... args, other_arguments && ...) -> res { return f_(args ...); };
            };
            
            std::function<res(arguments ...)> as() const { return f; };
            
            template <typename function_type_>
            std::function<function_type_> as() const { return std::function<function_type_>(*this); };
        private:
            template <typename res_, typename ... arguments_, std::size_t ... indices>
            static auto convert(std::function<res_(arguments_ ...)> f,
                                index_sequence<indices ...> &&)
                -> enable_if_t<sizeof...(arguments_) <= sizeof...(arguments), function_type>
            {
                return [f](arguments ... args) {
                    return static_cast<res>(f(get<indices>(std::forward<arguments>(args) ...) ...));
                };
            };
            
            template <std::size_t index>
            static auto get(arguments ... args)
                -> typename std::tuple_element<index, std::tuple<arguments && ...>>::type
            {
                return std::forward<
                    typename std::tuple_element<
                        index,
                        std::tuple<arguments && ...>
                    >::type
                >(std::get<index>(std::tuple<arguments && ...>(std::forward<arguments>(args) ...)));
            };
            
            function_type f;
        };
    };
    using opt_arg_function_detail::opt_arg_function;
};

#endif /* bbb_opt_arg_function_hpp */

