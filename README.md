# FSM

Implement finite state machine using C++17 features (std::variant, std::optional, std::shared_mutex)

## Example

```cpp
#include "fsm.h"

namespace state {

struct Disconnected {
};

struct Connected {
};

struct Closed {
};

inline constexpr auto disconnected = Disconnected {};
inline constexpr auto connected = Connected {};
inline constexpr auto closed = Closed {};

} // namespace state

namespace event {

struct Connect {
};

struct Close {
};

inline constexpr auto connect = Connect {};
inline constexpr auto close = Close {};

} // namespace event

using SocketState = std::variant<state::Disconnected, state::Connected, state::Closed>;

class Socket : public FSM<Socket, SocketState> {
public:
    auto OnEvent(state::Disconnected&, const event::Connect&) const
    {
        return state::connected;
    }

    auto OnEvent(state::Connected&, const event::Close&) const
    {
        return state::closed;
    }

    template <typename State, typename Event>
    auto OnEvent(State&, const Event&) const
    {
        return std::nullopt;
    }

private:
    friend FSM;

    void PrintStateTransition(const SocketState& next_state) const
    {
        auto current = GetStateName(FSM::Self());
        auto next = GetStateName(next_state);

        std::cout << current << "\t-> " << next << '\n';
    }

    std::string_view GetStateName(const SocketState& state) const
    {
        return std::visit([](auto&& state_variant) {
            using T = std::decay_t<decltype(state_variant)>;

            if constexpr (std::is_same_v<T, state::Disconnected>)
                return "Disconnected";
            else if constexpr (std::is_same_v<T, state::Connected>)
                return "Connected";
            else if constexpr (std::is_same_v<T, state::Closed>)
                return "Closed";
            else
                return "Undefined";
        },
            state);
    }
};

int main()
{
    Socket socket;

    socket.Dispatch(event::connect);
    socket.Dispatch(event::close);

    return 0;
}
```

[Compiler Explorer](https://godbolt.org/z/a8E4e576e)

## Reference

- [CppCon 2018: Mateusz Pusz “Effective replacement of dynamic polymorphism with std::variant”](https://www.youtube.com/watch?v=gKbORJtnVu8)
