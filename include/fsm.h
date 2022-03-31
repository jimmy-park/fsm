#ifndef FSM_H_
#define FSM_H_

#include <optional>
#include <shared_mutex>
#include <type_traits>
#include <utility>
#include <variant>

namespace detail {

template <typename T>
inline constexpr bool is_variant_v = false;

template <typename... Ts>
inline constexpr bool is_variant_v<std::variant<Ts...>> = true;

} // namespace detail

template <typename Derived, typename StateVariant, typename = std::enable_if_t<detail::is_variant_v<StateVariant>>>
class FSM {
public:
    template <typename Event>
    void Dispatch(Event&& event)
    {
        auto& derived = static_cast<Derived&>(*this);
        std::unique_lock lock { mutex_ };

        auto next_state = std::visit(
            [&derived, &event](auto& state) -> std::optional<StateVariant> {
                return derived.OnEvent(state, std::forward<Event>(event));
            },
            state_);

        if (next_state) {
            derived.PrintStateTransition(*next_state);
            state_ = *std::move(next_state);
        }
    }

protected:
    template <typename State>
    bool Hold() const
    {
        std::shared_lock lock { mutex_ };

        return std::holds_alternative<State>(state_);
    }

    template <typename State>
    std::optional<State> Get() const
    {
        std::optional<State> state;

        if (std::shared_lock lock { mutex_ }; const auto* value = std::get_if<State>(&state_))
            state = *value;

        return state;
    }

    const StateVariant& Self() const
    {
        return state_;
    }

private:
    StateVariant state_;
    mutable std::shared_mutex mutex_;
};

#endif // FSM_H_