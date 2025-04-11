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
                startReading(client);
            } else {
                socket->close();
            }
        }

        startAccepting();
    });
}

void Server::startReading(std::shared_ptr<ConnectedClient> client) {
    auto buffer = std::make_shared<std::vector<char>>(4096);

    client->socket->async_read_some(
        boost::asio::buffer(*buffer),
        [this, client, buffer](boost::system::error_code ec, [[maybe_unused]] size_t length) -> void {
            onDebugInfo(client, "[Server::startReading] Length: " + std::to_string(length));
            onDebugInfo(client, "[Server::startReading] Data: " + std::string{buffer->begin(), buffer->end()});

            if (!ec) {
                size_t sizeOfPacket = utils::Utils::bit32ToInt(*buffer);
                onDebugInfo(client, "[Server::startReading] Size of Packet: " + std::to_string(sizeOfPacket));

                if (sizeOfPacket <= MIN_PACKET_SIZE) {
                    startReading(client);
                } else {
                    readPacket(client, sizeOfPacket);
                }
            } else {
                std::lock_guard<std::mutex> guard(clientsMutex);
                clients.erase(client->socket.get());
            }
        }
    );
}

void Server::readPacket(std::shared_ptr<ConnectedClient> client, size_t sizeOfPacket) {
    auto buffer = std::make_shared<std::vector<char>>(sizeOfPacket);

    boost::asio::async_read(
        *client->socket,
        boost::asio::buffer(*buffer),
        [this, client, buffer](boost::system::error_code ec, [[maybe_unused]] size_t length) -> void {
            onDebugInfo(client, "[Server::readPacket] Length: " + std::to_string(length));
            onDebugInfo(client, "[Server::readPacket] Data: " + std::string{buffer->begin(), buffer->end()});

            if (!ec) {
                processPacket(client, *buffer);
            } else {
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
                startReading(client);
            } else {
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

    Packet packet;

    if (!client->isAuthenticated) {
        if (packetData == password) {
            packet = utils::Utils::compilePacket(id, DataType::SERVERDATA_AUTH_RESPONSE, "");
            onClientAuth(client);
        } else {
            packet = utils::Utils::compilePacket(-1, DataType::SERVERDATA_AUTH_RESPONSE, "");
        }
    } else {
        if (type != DataType::SERVERDATA_EXECCOMMAND) {
            packet = utils::Utils::compilePacket(
                id,
                DataType::SERVERDATA_RESPONSE_VALUE,
                "Invalid packet type (" + std::to_string(type) + "). Double check your packets."
            );
        } else {
            std::string data = onCommand(client, packetData);

            packet = utils::Utils::compilePacket(id, DataType::SERVERDATA_RESPONSE_VALUE, data);
        }
    }

    writePacket(client, packet);
}

} // namespace rcon_server::rcon
