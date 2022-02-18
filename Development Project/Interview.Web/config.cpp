#pragma once

#include "config.h"

#include "toml.hpp"
#include <iostream>

std::string CfgService::GetConnectionString() const {
    std::shared_lock<std::shared_mutex> l(mtx);
    return std::string(connectionStr);
}

unsigned int CfgService::GetServerPort() const {
    std::shared_lock<std::shared_mutex> l(mtx);
    return serverPort;
}

unsigned int CfgService::GetPageSize() const {
    std::shared_lock<std::shared_mutex> l(mtx);
    return pageSize;
}

void CfgService::init() {

    toml::table tbl;
    try {
        tbl = toml::parse_file("config.toml");
    } catch (const toml::parse_error &err) {
        std::cerr << "Parsing failed:\n" << err << "\n";
        return;
    }

    connectionStr = tbl["database"]["connectionString"].value_or("");
    serverPort    = static_cast<uint16_t>(
        tbl["server"]["port"].value_or<int64_t>((int64_t)serverPort));
}

CfgService::CfgService() {
    std::scoped_lock<std::shared_mutex> l(mtx);
    init();
}
