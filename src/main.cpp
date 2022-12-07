#include <random>
#include <thread>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "include/config.hpp"
#include "include/tcp_session.hpp"

namespace {
    constexpr std::uint16_t sleep_time_reconnect_ms = 500;
    constexpr std::uint8_t time_to_reconnect = 10;
}

int main() {
    //read config
    const auto &config = config::instance();

    //setup logger
    auto console_logger = spdlog::stdout_color_mt(config.get_logger_name());
    console_logger->set_error_handler([](const std::string &string) {
        std::cerr << string << std::endl;
    });

    boost::asio::io_context io;
    boost::asio::signal_set signals(io, SIGTERM);

    bool need_to_stop_application = false;
    signals.async_wait([&](const boost::system::error_code &error, int signal_number) {
        io.stop();
        need_to_stop_application = true;
        console_logger->info("Closing application...");
    });

    std::thread io_thread([&]() {
        io.run();
    });

    tcp_session session(config.get_network_config().host, config.get_network_config().port, io);
    auto ec = session.try_connect();

    std::uint8_t time_already_try_to_reconnect = 0;

    auto try_reconnect_if_error = [&](std::error_code &ec) mutable {
        while (ec) {
            if (time_already_try_to_reconnect >= 2 * time_to_reconnect) {
                console_logger->critical("Can't reconnect... : " + ec.message());
                return false;
            }
            console_logger->warn("Reconnecting... : " + ec.message());
            ec = session.try_connect();
            ++time_already_try_to_reconnect;
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time_reconnect_ms));
        }
        return true;
    };

    if (!try_reconnect_if_error(ec)) {
        return -1;
    }

    std::vector<std::string> commands{
            //set command
            "set-led-state ON\n",
            "set-led-color RED\n",
            "set-led-color GREEN\n",
            "set-led-color BLUE\n",
            "set-led-rate 0..10\n",
            //get command
            "get-led-state\n",
            "get-led-color\n",
            "get-led-rate\n",
            //invalid command
            "set-led-state yellow\n",
            "set-led-state\n",
            "get-led-state black\n",
            "set-led-rate a..23\n"
    };

    while (!need_to_stop_application) {
        for (auto &command: commands) {
            std::string response;
            try {
                response = session.send_and_get_response(command);
                boost::trim(response);
                console_logger->info("successfully send and receive data: " + response);
            } catch (const std::exception &ex) {
                console_logger->info(std::string{"can't send request: "} + ex.what());
                ec = session.try_connect();
                if (!try_reconnect_if_error(ec)) {
                    session.close_connection();
                    return -1;
                }
            }
        }
    }
    io_thread.join();
    return 0;
}