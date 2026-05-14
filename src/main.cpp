#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include "App.h"

namespace fs = std::filesystem;

static void createMtvRoot() {
    std::vector<std::string> dirs = {
        "mtv_root",
        "mtv_root/applications",
        "mtv_root/applications/YouTube",
        "mtv_root/applications/Settings",
        "mtv_root/applications/Games",
        "mtv_root/applications/Browser",
        "mtv_root/applications/Music",
        "mtv_root/applications/Store",
        "mtv_root/savedata",
        "mtv_root/savedata/slot1",
        "mtv_root/savedata/slot2",
        "mtv_root/savedata/slot3",
        "mtv_root/accdat",
        "mtv_root/bin",
        "mtv_root/tmp",
        "mtv_root/logs",
        "mtv_root/cache"
    };
    for (const auto& d : dirs)
        fs::create_directories(d);

    std::ofstream manifest("mtv_root/manifest.json");
    manifest << R"({
  "name": "MahirTV",
  "version": "1.0.0",
  "build": "2025.05.13",
  "arch": "linux-x86_64",
  "applications": ["YouTube","Settings","Games","Browser","Music","Store"],
  "saves": ["slot1","slot2","slot3"]
})";
    manifest.close();

    // App manifests
    auto writeAppMeta = [](const std::string& app) {
        std::ofstream f("mtv_root/applications/" + app + "/meta.json");
        f << "{\"name\":\"" << app << "\",\"installed\":true}";
    };
    for (auto& a : {"YouTube","Settings","Games","Browser","Music","Store"})
        writeAppMeta(a);
}

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    createMtvRoot();

    App app;
    if (!app.init()) {
        std::cerr << "Failed to init MahirTV\n";
        return 1;
    }
    app.run();
    return 0;
}
