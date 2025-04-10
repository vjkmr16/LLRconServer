#pragma once

#include <vector>

namespace rcon_server::rcon {

struct Packet {
    int32_t           length;
    int32_t           size;
    std::vector<char> data;
};

} // namespace rcon_server::rcon