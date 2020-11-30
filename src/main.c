/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>
#include <limits.h>
#include "log.h"
#include "rh/types.h"
#include "np/naming_proxy.h"
#include "server.h"
#include "client.h"

static const char optstring[] = "b:chI:p:qsS:tT:uv";
static const struct option longopts[] = {
        {"benchmark", required_argument, NULL, 'b'},
        {"client",    no_argument,       NULL, 'c'},
        {"help",      no_argument,       NULL, 'h'},
        {"instances", required_argument, NULL, 'I'},
        {"port",      required_argument, NULL, 'p'},
        {"server",    no_argument,       NULL, 's'},
        {"service",   required_argument, NULL, 'S'},
        {"tcp",       no_argument,       NULL, 't'},
        {"threads",   required_argument, NULL, 'T'},
        {"udp",       no_argument,       NULL, 'u'},
        {NULL,        no_argument,       NULL, '\0'}
};

static void __attribute__((noreturn)) usage(const uint_8 status, const char *const progname) {
    fprintf((status != 0) ? stderr : stdout, "Usage: %s [-c | -s [-t | -u] -p PORT ]\n", progname);

    if (status != 0) {
        fprintf(stderr, "Try '%s --help' for more information.\n", progname);
        exit(status);
    }

    printf("Run a client/server application supporting concurrent TCP/UDP connections and test the response time.\n\n");

    printf("Mode selection and protocol control:\n");
    printf("  -b, --benchmark=NUM  send NUM requests and print the response time\n");
    printf("  -c, --client         run as client\n");
    printf("  -s, --server         run as server\n");
    printf("  -t, --tcp            use the Transmission Control Protocol (TCP)\n");
    printf("  -u, --udp            use the User Datagram Protocol (UDP)\n");
    printf("  -p, --port=PORT      use PORT as the TCP/UDP port\n");
    printf("  -T, --threads        number of server threads (default: 4)\n");
    printf("  -I, --instances      number of service instances (default: 10)\n");
    printf("  -S, --service        service address in the format <SERVICE_NAME>+<PROTO>://<HOSTNAME>:<PORT>\n");
    printf("  -h, --help           display this help text and exit\n");

    exit(status);
}

int main(int argc, char *argv[]) {
    const char *progname = "csocket";
    int_32 opt;
    bool client = false, server = false, tcp = false, udp = false;
    uint_16 benchmark = 0, port = 0;
    uint_8 threads_num = 4, instances_num = 10;

    srand((uint_32) (time(NULL) - 16777215U));

    signal(SIGPIPE, SIG_IGN);

    while ((opt = getopt_long(argc, argv, optstring, longopts, NULL)) != -1) {
        switch (opt) {
            case 'b': {
                char *endptr;
                long optval = strtol(optarg, &endptr, 10);
                benchmark = (uint_16) optval;

                if (*endptr != '\0' || optval <= 0 || optval > USHRT_MAX || endptr == optarg)
                    die(EXIT_MISTAKE, 0, "%s: invalid benchmark argument", optarg);
            }
                break;
            case 'c':
                client = true;
                break;
            case 'h':
                usage(EXIT_SUCCESS, progname);
            case 'I': {
                char *endptr;
                long optval = strtol(optarg, &endptr, 10);
                instances_num = (uint_8) optval;

                if (*endptr != '\0' || optval <= 0 || optval > CHAR_MAX || endptr == optarg)
                    die(EXIT_MISTAKE, 0, "%s: invalid instances argument", optarg);
            }
                break;
            case 'p': {
                char *endptr;
                long optval = strtol(optarg, &endptr, 10);
                port = (uint_16) optval;

                if (*endptr != '\0' || optval <= 0 || optval > USHRT_MAX || endptr == optarg)
                    die(EXIT_MISTAKE, 0, "%s: invalid port argument", optarg);
            }
                break;
            case 'q':
                log_silence();
                break;
            case 's':
                server = true;
                break;
            case 'S': {
                char arg[256];
                strcpy(arg, optarg);

                const char *service_name = strtok(arg, "+");
                const char *hostname = strtok(NULL, "+");

                if (service_name == NULL || hostname == NULL || !np_add_service(service_name, hostname))
                    usage(EXIT_MISTAKE, progname);
            }
                break;
            case 't':
                tcp = true;
                break;
            case 'T': {
                char *endptr;
                long optval = strtol(optarg, &endptr, 10);
                threads_num = (uint_8) optval;

                if (*endptr != '\0' || optval <= 0 || optval > CHAR_MAX || endptr == optarg)
                    die(EXIT_MISTAKE, 0, "%s: invalid threads argument", optarg);
            }
                break;
            case 'u':
                udp = true;
                break;
            case 'v':
                log_increase_level();
                break;
            default:
                usage(EXIT_MISTAKE, progname);
        }
    }

    if ((!client && !server) || (server && ((!tcp && !udp) || !port)) || (client && (tcp || udp || port)) || (benchmark > 0 && !client)) {
        usage(EXIT_MISTAKE, progname);
    }

    if (server) {
        run_server(tcp ? TCP : UDP, port, threads_num, instances_num);
    } else if (benchmark) {
        return run_client_benchmark(benchmark);
    } else {
        run_client();
    }
}
