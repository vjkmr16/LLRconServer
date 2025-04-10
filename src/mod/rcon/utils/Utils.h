#pragma once

#include "../Packet.h"
#include <string>

namespace rcon_server::rcon::utils {

class Utils final {
public:
    // See https://developer.valvesoftware.com/wiki/Source_RCON_Protocol#Basic_Packet_Structure
    static Packet compilePacket(int32_t id, int32_t type, const std::string& body);

    static int bit32ToInt(const std::vector<char>& buffer);
    static int typeToInt(const std::vector<char>& buffer);
};

} // namespace rcon_server::rcon::utils