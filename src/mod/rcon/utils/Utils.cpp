#include "Utils.h"
#include "../Constants.h"
#include <stdexcept>

namespace rcon_server::rcon::utils {

Packet Utils::compilePacket(int32_t id, int32_t type, const std::string& body) {
    const int32_t sizeOfData = static_cast<int32_t>(body.size()) + MIN_PACKET_SIZE;
    if (sizeOfData > 4096) {
        throw std::runtime_error("This packet is too big to send. Please generate a smaller packet.");
    }

    std::vector<char> data(sizeOfData + 4); /* Create a vector that exactly the size of the packet length. */

    std::memcpy(data.data() + 0, &sizeOfData, sizeof(sizeOfData));                   /* Copy size into it */
    std::memcpy(data.data() + sizeof(sizeOfData), &id, sizeof(id));                  /* Copy id into it */
    std::memcpy(data.data() + sizeof(sizeOfData) + sizeof(id), &type, sizeof(type)); /* Copy type into it */
    std::memcpy(
        data.data() + sizeof(sizeOfData) + sizeof(id) + sizeof(type),
        body.data(),
        body.size()
    ); /* Copy data into it */

    Packet packet;
    packet.length = sizeOfData + 4;
    packet.size   = sizeOfData;
    packet.data   = data;

    return packet;
}

int Utils::bit32ToInt(const std::vector<char>& buffer) {
    return static_cast<int>(buffer[0] | buffer[1] << 8 | buffer[2] << 16 | buffer[3] << 24);
}

int Utils::typeToInt(const std::vector<char>& buffer) {
    return static_cast<int>(buffer[4] | buffer[5] << 8 | buffer[6] << 16 | buffer[7] << 24);
}

} // namespace rcon_server::rcon::utils