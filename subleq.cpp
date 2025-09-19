#include <cinttypes>

using i32 = std::int32_t;

template <i32 V>
struct constant {
    static constexpr auto value = V;
};

template <typename X, typename Y>
struct sub;

template <i32 X, i32 Y>
struct sub<constant<X>, constant<Y>> {
    using result = constant<X - Y>;
};

template <typename X>
struct leq;

template <i32 X>
struct leq<constant<X>> {
    using result = constant<X <= 0>;
};

template <typename A, typename B, typename C>
struct inst;

template <i32 A, i32 B, i32 C>
struct inst<constant<A>, constant<B>, constant<C>> {
    using a = constant<A>;
    using b = constant<B>;
    using c = constant<C>;
};

template <typename T>
struct output {
    static_assert(false, "output");
};

int main(void) {
    return 0;
}
