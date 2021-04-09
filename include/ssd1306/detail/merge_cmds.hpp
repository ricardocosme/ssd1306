#pragma once

#include "ssd1306/commands.hpp"
#include "ssd1306/send_commands.hpp"

#include <stdint.h>

namespace ssd1306 { namespace detail {

template<typename Cmd>
constexpr int size_to(Cmd){ return Cmd::size; }
constexpr int size_to(turn){ return 1; }

template<int N, uint8_t level>
constexpr void handle(uint8_t (&a)[N], int& i, contrast<level> o) {
    a[i++] = 0x81;
    a[i++] = o.level;
}

template<int N, typename Cmd>
constexpr void handle(uint8_t (&a)[N], int& i, Cmd) {
    a[i++] = Cmd::code;
}

template<int N, typename Cmd>
inline constexpr void handle(uint8_t (&a)[N], int& i, turn o) {
    if(o == turn::on) a[i++] = 0xaf;
    else a[i++] = 0xae;
}

template<int N>
struct Data{
    uint8_t data[N];
};

template<typename I2C, int N>
constexpr auto merge_cmds(I2C&& i2c, const uint8_t (&o)[N]) {
    Data<N> ret;
    for(int i{}; i < N; ++i)
        ret.data[i] = o[i];
    return ret;
}
    
template<typename I2C, int N, typename... Cmds>
constexpr auto merge_cmds(I2C&& i2c, const uint8_t (&o)[N], Cmds... cmds) {
    constexpr auto size = N + (size_to(cmds) + ...);
    Data<size> ret;
    int i{};
    for(; i < N; ++i)
        ret.data[i] = o[i];
    (handle(ret.data, i, cmds), ...);
    return ret;
}

}}
