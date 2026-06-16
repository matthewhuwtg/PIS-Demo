/**
 * @file main.cpp
 * @brief PIS (Passenger Information System) Demo — Main Entry Point
 *
 * Demonstrates skills required for PIS software development:
 *   - C++23 event-driven architecture
 *   - Unix/Linux IPC (Socket, D-Bus)
 *   - GStreamer-inspired media pipeline
 *   - JSON config parsing (no external dependency)
 *   - Modular design with callback-based event bus
 */

#include <iostream>
#include <memory>
#include <csignal>
#include "include/pis_core.h"
#include "include/utils.h"

static std::unique_ptr<pis::PISEngine> g_engine;

void signalHandler(int signum) {
    std::cout << "\n[Main] Signal " << signum << " received. Shutting down..." << std::endl;
    if (g_engine) {
        g_engine->stop();
    }
    std::exit(signum);
}

struct CmdArgs {
    std::string config_path = "config/sample_config.json";
    int duration_sec = 0;
    bool show_help = false;
};

CmdArgs parseArgs(int argc, char* argv[]) {
    CmdArgs args;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) args.config_path = argv[++i];
        } else if (arg == "-d" || arg == "--duration") {
            if (i + 1 < argc) args.duration_sec = std::stoi(argv[++i]);
        } else if (arg == "-h" || arg == "--help") {
            args.show_help = true;
        }
    }
    return args;
}

void printUsage(const char* prog) {
    std::cout << "PIS (Passenger Information System) Demo v1.0.0" << std::endl;
    std::cout << "Usage: " << prog << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -c, --config FILE    Config file path (default: config/sample_config.json)" << std::endl;
    std::cout << "  -d, --duration SEC   Simulation duration in seconds (default: full route)" << std::endl;
    std::cout << "  -h, --help           Show this help message" << std::endl;
}

int main(int argc, char* argv[]) {
    CmdArgs args = parseArgs(argc, argv);

    if (args.show_help) {
        printUsage(argv[0]);
        return 0;
    }

    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "==============================================" << std::endl;
    std::cout << "  PIS (Passenger Information System) Demo" << std::endl;
    std::cout << "  Built: " << __DATE__ << " " << __TIME__ << std::endl;
    std::cout << "  Standard: C++" << __cplusplus << std::endl;
#ifdef HAS_DBUS
    std::cout << "  D-Bus:  real (libdbus-1)" << std::endl;
#else
    std::cout << "  D-Bus:  mock" << std::endl;
#endif
#ifdef HAS_GSTREAMER
    std::cout << "  GStreamer: available" << std::endl;
#else
    std::cout << "  GStreamer: not available (using pipeline simulation)" << std::endl;
#endif
    std::cout << "==============================================\n" << std::endl;

    std::cout << "Start time: " << utils::getCurrentTime() << std::endl;
    std::cout << std::endl;

    g_engine = std::make_unique<pis::PISEngine>();

    if (!g_engine->initialize(args.config_path)) {
        std::cerr << "[Main] Failed to initialize PIS Engine. Exiting." << std::endl;
        return 1;
    }

    g_engine->printStatus();
    g_engine->run(args.duration_sec);

    std::cout << "[Main] PIS Demo finished at: " << utils::getCurrentTime() << std::endl;
    return 0;
}
