#pragma once
#include <any>
#include <functional>
#include <tuple>
//
#include <libviewer/utility.hpp>

namespace viewer {

namespace generic {
template <typename T>
concept callable = requires(T v) {
  std::function{v};
};
}  // namespace generic

namespace meta {

template <typename T, typename U>
concept equal = std::same_as<T, U>;

template <typename T>
struct function;

template <typename R, typename... Args>
struct function<std::function<R(Args...)>> {
  using result = R;
  using arguments = std::tuple<Args...>;
  template <size_t n>
  using argument = std::tuple_element_t<n, std::tuple<Args...>>;

  static constexpr size_t argument_count = sizeof...(Args);

  explicit function(std::function<R(Args...)>) {}
};

template <generic::callable T>
function(T) -> function<decltype(std::function{std::declval<T>()})>;

// We have to introduce a second using declaration.
// Otherwise, strange compiler errors seem to happen.
// We do not know why this happens.

template <generic::callable T>
using qualified_result = typename decltype(function{std::declval<T>()})::result;

template <generic::callable T>
using result = std::decay_t<qualified_result<T>>;

template <generic::callable T>
constexpr auto argument_count = decltype(function{
    std::declval<T>()})::argument_count;

template <generic::callable T>
using arguments = typename decltype(function{std::declval<T>()})::arguments;

template <generic::callable T, size_t N = 0>
requires(N < argument_count<T>)  //
    using qualified_argument = std::decay_t<typename decltype(function{
        std::declval<T>()})::template argument<N>>;

template <generic::callable T, size_t N = 0>
requires(N < argument_count<T>)  //
    using argument = std::decay_t<qualified_argument<T, N>>;

}  // namespace meta

template <typename I, typename O, typename R, typename W>  //
struct serializer {
  using input_type = I;
  using output_type = O;

  using reader = R;
  using writer = W;

  template <typename... T>
  constexpr void read(input_type in, std::tuple<T...>& data) {
    std::apply([&](T&... t) { (_read(in, t), ...); }, data);
  }

  template <typename... T>
  constexpr void write(const std::tuple<T...>& data, output_type out) {
    std::apply([&](const T&... t) { (_write(t, out), ...); }, data);
  }

  // For registering a dynamic function no templates or overloads are allowed.
  // We need to provide a fully instantiated callable.
  template <generic::callable F>  //
  constexpr void call(F&& f, input_type in, output_type out) {
    meta::arguments<F> args;
    read(in, args);
    if constexpr (std::same_as<meta::qualified_result<F>, void>)
      std::apply(std::forward<F>(f), args);
    else {
      const auto result = std::apply(std::forward<F>(f), args);
      _write(result, out);
    }
  }

  template <generic::callable T>  //
  inline void any_call(std::any f, input_type in, output_type out) {
    call(any_cast<T>(f), in, out);
  }

  struct dynamic_function {
    // A callable could be literally any kind of type.
    // Hence, we use the 'any' type from STL
    // to store function pointers.
    // Every dynamic needs to provide the same type.
    using callable_type = std::any;

    // Every function may have a different prototype.
    // Therefore every function needs a caller to tell
    // the runtime environment how to call the typed
    // function with untyped arguments.
    using caller_type =
        std::function<void(callable_type&, input_type, output_type)>;

    // template <generic::callable F>
    // dynamic_function(serializer &s, F &&f)
    //     : callable{std::forward<F>(f)}, caller{s.any_call<F>} {}

    void operator()(input_type in, output_type out) {
      std::invoke(caller, callable, in, out);
    }

    callable_type callable;
    caller_type caller;
  };

  template <generic::callable F>  //
  inline auto create(F&& f) -> dynamic_function {
    dynamic_function result{};
    result.callable = f;
    result.caller = [this](std::any x, input_type y, output_type z) {
      return any_call<F>(x, y, z);
    };
    // return {std::forward<F>(f), this->any_call<F>};
    return result;
  }

  reader _read{};
  writer _write{};
};

}  // namespace viewer
