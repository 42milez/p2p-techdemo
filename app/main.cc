#include <iostream>

#include <lyra/lyra.hpp>

#include "p2p_techdemo/buildinfo.h"

namespace
{
    void
    version()
    {
        const auto *buildinfo = p2p_techdemo_get_buildinfo();
        std::cout << "p2p_techdemo " << buildinfo->project_version << "\n";
        std::cout << "Build: " << buildinfo->system_name << "/" << buildinfo->build_type << std::endl;
    }

    const std::string LOGGER_NAME = "PEER";
    const std::string PATH_LOG    = "/var/log/p2p_techdemo/peer.log";
} // namespace

int
main(int argc, const char **argv)
{
    std::string mode  = "server";
    int port          = 12345;
    bool show_version = false;
    bool show_help    = false;

    auto cli = lyra::cli_parser();
    cli.add_argument(
        lyra::opt(mode, "mode").name("-m").name("--mode").help("Run mode").choices("client", "server").optional());
    cli.add_argument(lyra::opt(port, "port")
                         .name("-p")
                         .name("--port")
                         .help("Port to bind")
                         .choices([](int value) { return 0 <= value && value <= 65535; })
                         .optional());
    cli.add_argument(
        lyra::opt(show_version).name("-v").name("--version").help("Show application version").optional());
    cli.add_argument(lyra::help(show_help));

    auto result = cli.parse({argc, argv});
    if (!result) {
        std::cerr << "Error in command line: " << result.errorMessage() << std::endl;
        return 1;
    }

    if (show_version) {
        version();
        return 0;
    }

    if (show_help) {
        std::cout << cli;
        return 0;
    }

    //    auto &peer = core::Singleton<peer::Peer>::Instance();
    //    peer.init();
    //    peer.run();
}