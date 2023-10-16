#pragma once

#include <match/ops.hpp>
#include <sc/format.hpp>

#include <stdx/concepts.hpp>
#include <stdx/tuple.hpp>
#include <stdx/type_traits.hpp>

#include <cstdint>
#include <type_traits>

namespace msg {
constexpr inline class walk_index_t {
    template <match::matcher M>
    [[nodiscard]] friend constexpr auto tag_invoke(walk_index_t, M const &m,
                                                   stdx::callable auto const &f,
                                                   std::size_t &idx) {
        if constexpr (stdx::is_specialization_of_v<M, match::or_t>) {
            tag_invoke(walk_index_t{}, m.lhs, f, idx);
            ++idx;
            tag_invoke(walk_index_t{}, m.rhs, f, idx);
        } else if constexpr (stdx::is_specialization_of_v<M, match::and_t>) {
            tag_invoke(walk_index_t{}, m.lhs, f, idx);
            tag_invoke(walk_index_t{}, m.rhs, f, idx);
        } else if constexpr (stdx::is_specialization_of_v<M, match::not_t>) {
            tag_invoke(walk_index_t{}, m.m, f, idx);
        } else {
            // this type can't be indexed: do nothing
        }
    }

  public:
    template <typename... Ts>
    constexpr auto operator()(Ts &&...ts) const
        noexcept(noexcept(tag_invoke(std::declval<walk_index_t>(),
                                     std::forward<Ts>(ts)...)))
            -> decltype(tag_invoke(*this, std::forward<Ts>(ts)...)) {
        return tag_invoke(*this, std::forward<Ts>(ts)...);
    }
} walk_index{};

template <typename FieldType, typename T, T ExpectedValue> struct equal_to_t {
    using is_matcher = void;

    using field_type = FieldType;
    constexpr static auto expected_values = stdx::make_tuple(ExpectedValue);

    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        return ExpectedValue == msg.template get<FieldType>();
    }

    [[nodiscard]] constexpr auto describe() const {
        if constexpr (std::is_integral_v<T>) {
            return format("{} == 0x{:x}"_sc, FieldType::name,
                          sc::int_<static_cast<std::uint32_t>(ExpectedValue)>);
        } else {
            return format("{} == {} (0x{:x})"_sc, FieldType::name,
                          sc::enum_<ExpectedValue>,
                          sc::int_<static_cast<std::uint32_t>(ExpectedValue)>);
        }
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        if constexpr (std::is_integral_v<T>) {
            return format(
                "{} (0x{:x}) == 0x{:x}"_sc, FieldType::name,
                static_cast<std::uint32_t>(msg.template get<FieldType>()),
                sc::int_<static_cast<std::uint32_t>(ExpectedValue)>);
        } else {
            return format(
                "{} (0x{:x}) == {} (0x{:x})"_sc, FieldType::name,
                static_cast<std::uint32_t>(msg.template get<FieldType>()),
                sc::enum_<ExpectedValue>,
                sc::int_<static_cast<std::uint32_t>(ExpectedValue)>);
        }
    }

  private:
    [[nodiscard]] friend constexpr auto tag_invoke(walk_index_t,
                                                   equal_to_t const &,
                                                   stdx::callable auto const &f,
                                                   std::size_t &idx) {
        f.template operator()<FieldType>(idx, ExpectedValue);
    }
};

template <typename FieldType, typename T, T... ExpectedValues>
using in_t = decltype((equal_to_t<FieldType, T, ExpectedValues>{} or ... or
                       match::never));

template <typename FieldType, typename T, T expected_value>
struct greater_than_t {
    using is_matcher = void;

    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        return msg.template get<FieldType>() > expected_value;
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{} > 0x{:x}"_sc, FieldType::name,
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        return format("{} (0x{:x}) > 0x{:x}"_sc, FieldType::name,
                      static_cast<std::uint32_t>(msg.template get<FieldType>()),
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }
};

template <typename FieldType, typename T, T expected_value>
struct greater_than_or_equal_to_t {
    using is_matcher = void;

    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        return msg.template get<FieldType>() >= expected_value;
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{} >= 0x{:x}"_sc, FieldType::name,
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        return format("{} (0x{:x}) >= 0x{:x}"_sc, FieldType::name,
                      static_cast<std::uint32_t>(msg.template get<FieldType>()),
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }
};

template <typename FieldType, typename T, T expected_value> struct less_than_t {
    using is_matcher = void;

    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        return msg.template get<FieldType>() < expected_value;
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{} < 0x{:x}"_sc, FieldType::name,
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        return format("{} (0x{:x}) < 0x{:x}"_sc, FieldType::name,
                      static_cast<std::uint32_t>(msg.template get<FieldType>()),
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }
};

template <typename FieldType, typename T, T expected_value>
struct less_than_or_equal_to_t {
    using is_matcher = void;

    template <typename MsgType>
    [[nodiscard]] constexpr auto operator()(MsgType const &msg) const -> bool {
        return msg.template get<FieldType>() <= expected_value;
    }

    [[nodiscard]] constexpr auto describe() const {
        return format("{} <= 0x{:x}"_sc, FieldType::name,
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }

    template <typename MsgType>
    [[nodiscard]] constexpr auto describe_match(MsgType const &msg) const {
        return format("{} (0x{:x}) <= 0x{:x}"_sc, FieldType::name,
                      static_cast<std::uint32_t>(msg.template get<FieldType>()),
                      sc::int_<static_cast<std::uint32_t>(expected_value)>);
    }
};
} // namespace msg
