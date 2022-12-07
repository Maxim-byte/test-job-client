#pragma once

#include <filesystem>
#include <string_view>

#include "config_structs.hpp"

class config {
public:
    static std::filesystem::path path_to_config;

    static config &instance() {
        static config config_manager;
        return config_manager;
    }

    [[nodiscard]] const network_config &get_network_config() const;

    [[nodiscard]] const std::string &get_logger_name() const;

private:
    config();

private:
    network_config network_;
    std::string logger_name_;
};