#pragma once

#include <ll/api/mod/NativeMod.h>
#include <string>

namespace rcon_server::manager {

struct RconSettings {
    unsigned short port           = 19130;
    unsigned short maxConnections = 5;
    std::string    password       = "Change this to your password";
};

class ConfigManager final {
public:
    struct MainConfig {
        int          version = 1;
        RconSettings rconSettings;
    };

    static bool              init(ll::mod::NativeMod& mod);
    static const MainConfig& getConfig();

private:
    static MainConfig config;
};

} // namespace rcon_server::manager