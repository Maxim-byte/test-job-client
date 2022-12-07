#pragma once

#include <string_view>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

class tcp_session {
public:
    tcp_session(const std::string & host, std::uint16_t port, boost::asio::io_context &io);

    ~tcp_session();

    std::error_code try_connect();

    std::string send_and_get_response(std::string &request);

    void close_connection();

private:
    std::string host_;
    std::string port_;
    boost::asio::io_context &io_;
    boost::beast::tcp_stream stream_;
};

