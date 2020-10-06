#include <iostream>
#include <string.h>
#include <sys/resource.h>
#include "daemon/settings.hpp"
#include "daemon/daemon.hpp"
#include "version.hpp"

int main (int argc, char* argv[]) {

    // Поддержка версий
    if (argc != 2){
        std::cerr << argv[0] << "[--version|path/to/config]" << "\n";
        return -1;
    }

    char const* arg = argv[1];

    if (strcmp(arg, "--version") == 0){
        std::cout << call45aggr_version << "." << build_version() << "\n";
        return 0;
    }

    read_settings(arg);

    // Падаем правильно
    {
        if (get_settings().use_core_dump()) {
            struct rlimit limit{RLIM_INFINITY, RLIM_INFINITY};
            setrlimit(RLIMIT_CORE, &limit);
        }
    }

    return app::run();
}
