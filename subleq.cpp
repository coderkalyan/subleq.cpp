#include <cstdint>

using i32 = std::int32_t;

// template <typename T>
// struct output {
//     static_assert(false, "output");
// };

template <typename T>
[[deprecated]] inline constexpr i32 output() {
    return 0;
}

template <i32 V>
struct constant {
    static constexpr auto value = V;
};

template <bool B>
struct predicate {
    static constexpr auto value = B;
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

template <typename... Words>
struct memory {
    using words = memory<Words...>;
};

template <typename Memory, typename Index>
struct read;

template <typename Head, typename... Tail, typename Index>
struct read<memory<Head, Tail...>, Index> {
    using type =
        typename read<memory<Tail...>, constant<Index::value - 1>>::type;
};

template <typename Head, typename... Tail>
struct read<memory<Head, Tail...>, constant<0>> {
    using type = Head;
};

template <typename Memory, typename Index>
using read_t = typename read<Memory, Index>::type;

template <typename Memory, i32 Index, typename Value>
struct write;

template <typename Head, typename... Tail, i32 Index, typename Value>
struct write<memory<Head, Tail...>, Index, Value> {
    using type = typename write<memory<Tail...>, Index - 1, Value>::type::words;
};

template <typename Head, typename... Tail, typename Value>
struct write<memory<Head, Tail...>, 0, Value> {
    using type = memory<Value, Tail...>;
};

template <typename Memory, i32 Index, typename Value>
using write_t = typename write<Memory, Index, Value>::type;

template <typename T>
struct program_counter;

template <i32 Index>
struct program_counter<constant<Index>> {
    static constexpr auto value = constant<Index>::value;
};

template <typename Memory, typename T>
struct fetch;

template <typename Memory, i32 Index>
struct fetch<Memory, program_counter<constant<Index>>> {
    using a = read_t<Memory, constant<Index>>;
    using b = read_t<Memory, constant<Index + 1>>;
    using c = read_t<Memory, constant<Index + 2>>;
    using type = inst<a, b, c>;
};

template <typename Memory, i32 Index>
using fetch_t = typename fetch<Memory, program_counter<constant<Index>>>::type;

template <bool B, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> {
    using type = T;
};

template <bool B, typename T = void>
using enable_if_t = typename enable_if<B, T>::type;

template <typename Memory,
          typename Index,
          typename Value,
          typename Tape,
          typename = void>
struct update;

template <typename Memory, i32 Index, typename Value, typename... Tape>
struct update<Memory,
              constant<Index>,
              Value,
              memory<Tape...>,
              enable_if_t<(Index >= 0)>> {
    using load = read_t<Memory, constant<Index>>;
    using modify = typename sub<load, Value>::result;
    using branch = typename leq<modify>::result;

    using type = write_t<Memory, Index, modify>;
    using tape = memory<Tape...>;
};

template <typename Memory, i32 Index, typename Value, typename... Tape>
struct update<Memory,
              constant<Index>,
              Value,
              memory<Tape...>,
              enable_if_t<(Index < 0)>> {
    using branch = predicate<true>;
    using type = Memory;
    using tape = memory<Tape..., Value>;
};

template <bool B, typename X, typename Y>
struct mux {};

template <typename X, typename Y>
struct mux<true, X, Y> {
    using type = X;
};

template <typename X, typename Y>
struct mux<false, X, Y> {
    using type = Y;
};

template <bool B, typename X, typename Y>
using mux_t = typename mux<B, X, Y>::type;

template <typename Memory, typename ProgramCounter, typename Tape>
struct cycle;

template <typename Memory, i32 Index, typename... Tape>
struct cycle<Memory, program_counter<constant<Index>>, memory<Tape...>> {
    // Fetch the instruction from memory.
    using op = fetch_t<Memory, Index>;

    // Read the operands A and B, and perform the subtraction.
    // The RMW operation is handled atomically in the update
    // step in order to make use of SFINAE for memory mapped output.
    using a = read_t<Memory, typename op::a>;
    using rmw = update<Memory, typename op::b, a, memory<Tape...>>;
    using memory = typename rmw::type;

    // Resolve the branch.
    using cmp = typename rmw::branch;
    using inc = program_counter<constant<Index + 3>>;
    using jmp = program_counter<typename op::c>;
    using branch = mux_t<cmp::value, jmp, inc>;

    // Export the running output tape.
    using tape = typename rmw::tape;
};

template <typename Memory,
          typename ProgramCounter,
          typename Cycles,
          typename Tape,
          typename = void>
struct execute;

template <typename Memory, i32 Index, i32 Cycles, typename... Tape>
struct execute<Memory,
               program_counter<constant<Index>>,
               constant<Cycles>,
               memory<Tape...>,
               enable_if_t<(Index >= 0) && (Cycles < 10)>> {
    using step =
        cycle<Memory, program_counter<constant<Index>>, memory<Tape...>>;
    using next = execute<typename step::memory,
                         typename step::branch,
                         constant<Cycles + 1>,
                         typename step::tape>;

    // static constexpr auto foo = output<typename step::tape>();
    static constexpr auto result = next::result;
    // output<typename next::step::branch>();
};

template <typename Memory, i32 Index, i32 Cycles, typename... Tape>
struct execute<Memory,
               program_counter<constant<Index>>,
               constant<Cycles>,
               memory<Tape...>,
               enable_if_t<(Index < 0) || (Cycles >= 10)>> {
    static constexpr auto result = output<memory<Tape...>>();
};

// clang-format off
using program = memory<
    constant<9>, constant<-1>, constant<3>,
    constant<10>, constant<-1>, constant<6>,
    constant<0>, constant<0>, constant<-1>,
    constant<72>, constant<105>, constant<0>
>;
// clang-format on

using pc = program_counter<constant<0>>;
// using op = fetch<program, pc>;

int main(void) {
    // cycle<program, pc> cycle;
    // (void) output<decltype(cycle)::memory>();

    execute<program, pc, constant<0>, memory<>> computer;
    // (void) output<decltype(computer)::memory>();

    return 0;
}
