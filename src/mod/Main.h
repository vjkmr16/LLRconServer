#pragma once

#include <ll/api/mod/NativeMod.h>

namespace rcon_server {

class Main {

public:
    static Main& getInstance();

    Main() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }
    
    bool load();
    bool enable();
    bool disable();

private:
    ll::mod::NativeMod& mSelf;
};

} // namespace rcon_server
