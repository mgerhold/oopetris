
#include "connection_manager.hpp"
#include "network_manager.hpp"
#include "network_transportable.hpp"
#include <SDL.h>
#include <cstdlib>
#include <memory>
#include <string>
#include <tl/optional.hpp>
#include <vector>

Connection::Connection(TCPsocket socket) : m_socket{ socket } {};

Connection::~Connection() {
    SDLNet_TCP_Close(m_socket);
}

tl::optional<std::string> Connection::send_data(const Transportable* transportable, uint32_t data_size) {

    auto [message, length] = Transportable::serialize(transportable, data_size);

    const auto result = SDLNet_TCP_Send(m_socket, message, length);
    if (result == -1) {
        std::free(message);
        return tl::make_optional("SDLNet_TCP_Send: invalid socket");
    }

    if ((std::size_t) result != length) {
        std::free(message);
        std::string error = "SDLNet_TCP_Send: " + std::string{ SDLNet_GetError() };
        return tl::make_optional(error);
    }

    std::free(message);

    return {};
};


Server::Server(TCPsocket socket) : m_socket{ socket }, m_connections{ std::vector<std::shared_ptr<Connection>>{} } {};

Server::~Server() {
    SDLNet_TCP_Close(m_socket);
}


tl::optional<std::shared_ptr<Connection>> Server::try_get_client() {

    TCPsocket client;
    /* try to accept a connection */
    client = SDLNet_TCP_Accept(m_socket);
    if (client) { /* no connection accepted */
        auto connection = std::make_shared<Connection>(client);
        m_connections.push_back(connection);
        return connection;
    }
    return {};
}

tl::optional<std::shared_ptr<Connection>> Server::get_client(std::size_t ms_delay, std::size_t abort_after) {
    auto start_time = SDL_GetTicks64();
    while (true) {
        /* try to accept a connection */
        auto client = this->try_get_client();
        if (client.has_value()) {
            return client;
        }

        auto elapsed_time = SDL_GetTicks64() - start_time;
        if (elapsed_time >= abort_after) {
            return {};
        }

        SDL_Delay(ms_delay);
        continue;
    }
}

tl::optional<std::string> Server::send_all(const Transportable* transportable, uint32_t data_size) {

    for (size_t i = 0; i < m_connections.size(); ++i) {
        auto result = m_connections.at(i)->send_data(transportable, data_size);
        if (result.has_value()) {
            return tl::make_optional("Error while sending to client: " + std::to_string(i) + " : " + result.value());
        }
    }

    return {};
}

tl::expected<bool, std::string> Server::is_data_available(Uint32 timeout_ms) {

    SDLNet_SocketSet set = SDLNet_AllocSocketSet(1);
    if (!set) {
        return tl::make_unexpected("no more memory for creating a SDLNet_SocketSet");
    }


    auto num_sockets = SDLNet_TCP_AddSocket(set, m_socket);
    if (num_sockets != 1) {
        return tl::make_unexpected("SDLNet_AddSocket failed, this is an implementation error");
    }

    auto result = SDLNet_CheckSockets(set, timeout_ms);
    if (result == -1) {
        return tl::make_unexpected("SDLNet_CheckSockets error (select() system call error)");
    }


    SDLNet_FreeSocketSet(set);

    return result == 1;
}


tl::expected<RawBytes, std::string> Server::get_all_data_blocking() {
    void* memory = std::malloc(Server::chunk_size);
    if (!memory) {
        return tl::make_unexpected("error in malloc for receiving a socket message");
    }
    std::uint32_t data_size = 0;
    while (true) {
        int len = SDLNet_TCP_Recv(m_socket, memory, Server::chunk_size);
        if (len <= 0) {
            free(memory);
            return tl::make_unexpected("SDLNet_TCP_Recv: " + std::string{ SDLNet_GetError() });
        }

        if (len != Server::chunk_size) {

            return RawBytes{ (uint8_t*) memory, data_size + len };
        }

        data_size += Server::chunk_size;
        void* new_memory = std::realloc(memory, data_size + Server::chunk_size);
        if (!new_memory) {
            free(memory);
            return tl::make_unexpected("error in realloc for receiving a socket message");
        }
        memory = new_memory;
    }

    return tl::make_unexpected("error in SDLNet_TCP_Recv: somehow exited the while loop");
}

MaybeData Server::get_data() {

    auto data_available = is_data_available();
    if (!data_available.has_value()) {
        return tl::make_unexpected("in is_data_available: " + data_available.error());
    }

    if (!data_available.value()) {
        return {};
    }

    auto data = get_all_data_blocking();
    if (!data.has_value()) {
        return tl::make_unexpected("in get_all_data_blocking: " + data_available.error());
    }


    RawBytes raw_bytes = data.value();

    auto result = RawTransportData::from_raw_bytes(raw_bytes);
    if (!result.has_value()) {
        free(raw_bytes.first);
        return tl::make_unexpected("in RawTransportData::from_raw_bytes: " + result.error());
    }

    free(raw_bytes.first);
    return tl::make_optional(std::move(result.value()));
}