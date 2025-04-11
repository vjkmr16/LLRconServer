#include "Server.h"
#include "Constants.h"
#include "utils/Utils.h"

namespace rcon_server::rcon {

void Server::start() {
    startAccepting();
    std::thread([this]() -> void { ioContext.run(); }).detach();
}

void Server::stop() {
    acceptor.close();
    ioContext.stop();

    clients.clear();
}

void Server::startAccepting() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioContext);

    acceptor.async_accept(*socket, [this, socket](boost::system::error_code ec) -> void {
        if (!ec) {
            std::lock_guard<std::mutex> guard(clientsMutex);
            if (clients.size() <= maxConnections) {
                std::shared_ptr<ConnectedClient> client = std::make_shared<ConnectedClient>(socket, false);

                onNewConnection(client);

                clients[socket.get()] = client;
                readPacket(client);
            } else {
                socket->close();
            }
        }

        startAccepting();
    });
}

void Server::readPacket(std::shared_ptr<ConnectedClient> client) {
    auto buffer = std::make_shared<std::vector<char>>(4096);

    client->socket->async_read_some(
        boost::asio::buffer(*buffer),
        [this, client, buffer](boost::system::error_code ec, [[maybe_unused]] size_t length) -> void {
            onDebugInfo(client, "[Server::readPacket] New packet. Length: " + std::to_string(length));

            if (!ec) {
                std::stringstream ss;
                for (size_t i = 0; i < length; ++i) {
                    ss << std::hex << static_cast<int>((*buffer)[i]) << " ";
                }
                onDebugInfo(client, "[Server::readPacket] Raw packet data: " + ss.str());

                size_t sizeOfPacket = utils::Utils::bit32ToInt(*buffer);
                onDebugInfo(client, "[Server::readPacket] Size of Packet: " + std::to_string(sizeOfPacket));

                if (sizeOfPacket <= MIN_PACKET_SIZE) {
                    onDebugInfo(client, "[Server::readPacket] Invalid Packet");
                    readPacket(client);
                } else {
                    onDebugInfo(client, "[Server::readPacket] Correct Packet. Processing...");

                    buffer->erase(buffer->begin(), buffer->begin() + 4);
                    buffer->resize(sizeOfPacket);

                    processPacket(client, *buffer);
                }
            } else {
                onClientDisconnect(client);
                if (ec != boost::asio::error::eof) {
                    onDebugInfo(client, "[Server::readPacket] Error occurred: " + ec.message());
                }

                std::lock_guard<std::mutex> guard(clientsMutex);
                clients.erase(client->socket.get());
            }
        }
    );
}

void Server::writePacket(std::shared_ptr<ConnectedClient> client, const Packet& packet) {
    boost::asio::async_write(
        *client->socket,
        boost::asio::buffer(packet.data.data(), packet.length),
        [this, client](boost::system::error_code ec, [[maybe_unused]] size_t length) -> void {
            if (!ec) {
                readPacket(client);
            } else {
                onClientDisconnect(client);
                onDebugInfo(client, "[Server::readPacket] Error occurred: " + ec.message());

                std::lock_guard<std::mutex> guard(clientsMutex);
                clients.erase(client->socket.get());
            }
        }
    );
}

void Server::processPacket(std::shared_ptr<ConnectedClient> client, const std::vector<char>& buffer) {
    std::string packetData(buffer.begin() + 8, buffer.end() - 2);
    int         id   = utils::Utils::bit32ToInt(buffer);
    int         type = utils::Utils::typeToInt(buffer);

    onDebugInfo(client, "[Server::processPacket] Packet id: " + std::to_string(id));
    onDebugInfo(client, "[Server::processPacket] Packet type: " + std::to_string(type));
    onDebugInfo(client, "[Server::processPacket] Packet data: " + packetData);

    Packet packet;

    if (!client->isAuthenticated) {
        if (packetData == password) {
            packet = utils::Utils::compilePacket(id, DataType::SERVERDATA_AUTH_RESPONSE, "");

            client->isAuthenticated = true;
            onClientAuth(client);
        } else {
            packet = utils::Utils::compilePacket(-1, DataType::SERVERDATA_AUTH_RESPONSE, "");
        }
    } else {
        if (type != DataType::SERVERDATA_EXECCOMMAND) {
            onDebugInfo(client, "[Server::processPacket] Invalid Packet type (" + std::to_string(type) + ").");

            packet = utils::Utils::compilePacket(
                id,
                DataType::SERVERDATA_RESPONSE_VALUE,
                "Invalid packet type (" + std::to_string(type) + "). Double check your packets."
            );
        } else {
            std::string data = onCommand(client, packetData);
            onDebugInfo(client, "[Server::processPacket] New Packet data: " + data);

            packet = utils::Utils::compilePacket(id, DataType::SERVERDATA_RESPONSE_VALUE, data);
        }
    }

    writePacket(client, packet);
}

} // namespace rcon_server::rcon
