#include "../include/tcp_session.hpp"

#include <iostream>

tcp_session::tcp_session(const std::string &host, std::uint16_t port, boost::asio::io_context &io) :
        host_(host),
        port_(std::to_string(port)),
        io_(io),
        stream_(io_) {
    stream_.expires_never();
}

tcp_session::~tcp_session() {
    close_connection();
}

std::error_code tcp_session::try_connect() {
    boost::beast::error_code ec;
    boost::asio::ip::tcp::resolver resolver(io_);
    stream_.connect(resolver.resolve(host_, port_), ec);
    return ec;
}

std::string tcp_session::send_and_get_response(std::string &request) {
    if (!stream_.socket().is_open()) {
        try_connect();
    }

    boost::asio::write(stream_.socket(), boost::asio::buffer(request));
    static boost::asio::streambuf buffer;
    boost::asio::read_until(stream_, buffer, "\n");
    return {(std::istreambuf_iterator<char>(&buffer)), std::istreambuf_iterator<char>()};
}

void tcp_session::close_connection() {
    boost::beast::error_code ec;
    if (stream_.socket().is_open()) {
        stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    }
}
