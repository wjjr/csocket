/* csocket: Copyright (c) 2020 Wendell Júnior - This code is licensed under MIT license (see LICENSE for details) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "types.h"
#include "log.h"
#include "server.h"
#include "client.h"

static const char optstring[] = "b:chp:qstuv";
static const struct option longopts[] = {
        {"benchmark", required_argument, NULL, 'b'},
        {"client",    no_argument,       NULL, 'c'},
        {"help",      no_argument,       NULL, 'h'},
        {"port",      required_argument, NULL, 'p'},
        {"server",    no_argument,       NULL, 's'},
        {"tcp",       no_argument,       NULL, 't'},
        {"udp",       no_argument,       NULL, 'u'},
        {NULL,        no_argument,       NULL, '\0'}
};

static void __attribute__((noreturn)) usage(const uint_8 status, const char *const progname) {
    fprintf((status != 0) ? stderr : stdout, "Usage: %s [-c | -s] [-t | -u] [-p PORT]\n", progname);

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
    printf("  -h, --help           display this help text and exit\n");

    exit(status);
}

int main(int argc, char *argv[]) {
    const char *progname = "csocket";
    int_32 opt;
    bool client = false, server = false, tcp = false, udp = false;
    uint_16 benchmark = 0, port = 0;
    struct context *context;

    srand((uint_32) (time(NULL) - UINT24_MAX));

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
            case 't':
                tcp = true;
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

    if ((!client && !server) || (!tcp && !udp) || (benchmark > 0 && !client)) {
        usage(EXIT_MISTAKE, progname);
    }

    context = malloc(sizeof(struct context));
    context->mode = server ? SERVER : CLIENT;
    context->protocol = tcp ? TCP : UDP;
    context->port = port;
    context->benchmark_num = benchmark;

    if (context->mode == SERVER) {
        run_server(context);
    } else if (benchmark) {
        return run_client_benchmark(context);
    } else {
        run_client(context);
    }
}
