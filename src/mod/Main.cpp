#include "Main.h"
#include "manager/MainManager.h"
#include <ll/api/mod/RegisterHelper.h>

namespace rcon_server {

Main& Main::getInstance() {
    static Main instance;
    return instance;
}

bool Main::load() {
    getSelf().getLogger().info("The mod is loading...");

    if (!manager::MainManager::initManagers(getSelf())) {
        getSelf().getLogger().info("Failed to load the mod!");
        return false;
    }

    getSelf().getLogger().info("The mod has been successfully loaded!");
    return true;
}

bool Main::enable() {
    getSelf().getLogger().info("The mod is enabling...");

    manager::MainManager::startRconServer();

    getSelf().getLogger().info("The mod has been successfully enabled!");

    getSelf().getLogger().info("Author: vk.com/lordbomba");
    return true;
}

bool Main::disable() {
    getSelf().getLogger().info("The mod is disabling...");

    manager::MainManager::stopRconServer();

    getSelf().getLogger().info("The mod has been successfully disabled.");
    return true;
}

} // namespace rcon_server

LL_REGISTER_MOD(rcon_server::Main, rcon_server::Main::getInstance());
