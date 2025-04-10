#pragma once

#include "ConnectedClient.h"
#include "Packet.h"
#include <boost/asio.hpp>

namespace rcon_server::rcon {

class Server final : public std::enable_shared_from_this<Server> {
public:
    explicit Server(
        unsigned short                                                     port,
        unsigned short                                                     maxConnections,
        const std::string&                                                 password,
        const std::function<void(const std::shared_ptr<ConnectedClient>)>& onNewConnection = {},
        const std::function<void(const std::shared_ptr<ConnectedClient>)>& onClientAuth    = {},
        const std::function<std::string(const std::shared_ptr<rcon::ConnectedClient>, const std::string&)>& onCommand =
            {}
    )
    : acceptor({this->ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)}),
      maxConnections(maxConnections),
      password(password),
      onNewConnection(onNewConnection),
      onClientAuth(onClientAuth),
      onCommand(onCommand) {}

    void start();
    void stop();

private:
    boost::asio::io_context        ioContext;
    boost::asio::ip::tcp::acceptor acceptor;

    unsigned short                                                                               maxConnections;
    std::string                                                                                  password;
    std::function<void(const std::shared_ptr<ConnectedClient>)>                                  onNewConnection;
    std::function<void(const std::shared_ptr<ConnectedClient>)>                                  onClientAuth;
    std::function<std::string(const std::shared_ptr<rcon::ConnectedClient>, const std::string&)> onCommand;

    std::unordered_map<boost::asio::ip::tcp::socket*, std::shared_ptr<ConnectedClient>> clients;
    std::mutex                                                                          clientsMutex;

    void startAccepting();
    void startReading(std::shared_ptr<ConnectedClient> client);

    void readPacket(std::shared_ptr<ConnectedClient> client, size_t sizeOfPacket);
    void writePacket(std::shared_ptr<ConnectedClient> client, const Packet& packet);

    void processPacket(std::shared_ptr<ConnectedClient> client, const std::vector<char>& buffer);
};

} // namespace rcon_server::rcon