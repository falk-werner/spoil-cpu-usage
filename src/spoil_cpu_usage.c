#include <sys/time.h>
#include <unistd.h>

#include <time.h>
#include <signal.h>
#include <limits.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static volatile sig_atomic_t running = 1;

static void print_usage(void)
{
    printf(
        "spoil_cpu_usage, (C) 2026 Falk Werner <github.com/falk-werner>\n"
        "Tool to demonstrate htop error when computing CPU usage.\n"
        "\n"
        "Usage:\n"
        "  ./spoil_cpu_usage [-n]\n"
        "\n"
        "Options:\n"
        "-n, --no-shift - disable shift mode\n"
        "-h, --help     - display this message\n"
        "\n"
        "Description:\n"
        "This tool starts a cyclic task utilizing roughly 50%% of the CPU.\n"
        "The task is synchronized to clock ticks interval to provoke an\n"
        "error in how htop computes CPU usage on system with RT_PREEMPT\n"
        "patch enabled.\n"
        "\n"
        "Modes:\n"
        "- shift mode (default)\n"
        "  In shift mode, the task is shifted by 10%% within it's interval\n"
        "  every 5 seconds. This leads to htop toggling between 0%% and\n"
        "  100%% CPU usage over time.\n"
        "- no-shift mode (enabled by -n command line option)\n"
        "  In no-shift mode, the task is not shifted at all. Depending on the\n"
        "  timing htop will eigther show 0%% or 100%% CPU usage as long as\n."
        "  task is running.\n"
        "\n"
        "References:\n"
        "- https://docs.kernel.org/admin-guide/cpu-load.html\n"
        "\n"
        "Examples:\n"
        "  ./spoil_cpu_usage\n"
        "  ./spoil_cpu_usage -n\n"
    );
}

static void on_shutdown(int signal_id)
{
    running = 0;
}

static uint64_t now_usec(void)
{
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);
    return ( (((uint64_t) timestamp.tv_sec) * (1000UL * 1000UL)) + (((uint64_t) timestamp.tv_nsec) / (1000UL)) );
}

static uint64_t sleep_until(uint64_t const timestamp)
{
    uint64_t const now = now_usec();
    if (timestamp > now)
    {
        uint64_t const duration_nsec = (timestamp - now) * 1000UL;
        struct timespec const duration = {.tv_sec = 0, .tv_nsec = duration_nsec};
        nanosleep(&duration, NULL);
    }
}

static void compute_until(uint64_t * highest_prime, uint64_t * checks, uint64_t const timeout_usec)
{
    uint64_t const start = now_usec();

    *checks = 0;
    *highest_prime = 2;

    uint64_t operations = 0;
    for (uint64_t candidate = 3; ; candidate++)
    {
        int found = 0;
        for(uint64_t i = 2; (i < candidate) && (!found); i++) {
            if ((candidate % i) == 0) {
                found = 1;
                *highest_prime = candidate;
            }

            operations++;
            if ((operations > (10UL * 1000UL))) {
                (*checks)++;
                operations = 0;
                if (now_usec() >= timeout_usec) {                    
                    return;
                }
            }
        }        
    }
}

int main(int argc, char* argv[])
{
    bool do_shift = true;
    if (argc > 1)
    {
        if ((0 == strcmp("-h", argv[1])) || (0 == strcmp("--help", argv[1])))
        {
            print_usage();
            return EXIT_SUCCESS;
        }
        else if ((0 == strcmp("-n", argv[1])) || (0 == strcmp("--no-shift", argv[1])))
        {
            do_shift = false;
        }
        else
        {
            fprintf(stderr, "error: unknown command line argument\n");
            print_usage();
            return EXIT_FAILURE;
        }
    }

    long const user_hz = sysconf(_SC_CLK_TCK);
    uint64_t const interval_usec = (1000UL * 1000UL) / user_hz;
    uint64_t const shift_usec = (do_shift) ? interval_usec / 10UL : 0UL;
    uint64_t const max_count = (5UL * 1000UL * 1000UL) / interval_usec; // 10s

    printf("user hz: %ld\n", user_hz);
    printf("interval usec: %" PRIu64 "\n", interval_usec);

    uint64_t highest_prime = 0;
    uint64_t checks = 0;

    signal(SIGINT, on_shutdown);
    uint64_t max_diff = 0;

    uint64_t const now = now_usec();
    uint64_t next_compute = now + (interval_usec / 2);
    uint64_t next = now + interval_usec;
    uint64_t count = 0;
    while (running)
    {
        compute_until(&highest_prime, &checks, next_compute);
        sleep_until(next);

        uint64_t const end = now_usec();        
        uint64_t const diff  = (next > end) ? next - end : end - next;
        if (diff > max_diff) {
            max_diff = diff;
        }

        while (next_compute <= end) {
            next += interval_usec;
            next_compute += interval_usec;
            count++;
            if (count == max_count) {
                count = 0;
                next+= shift_usec;
                next_compute += shift_usec;
            }
        }
    }

    printf("highest prime: %" PRIu64 "\n", highest_prime);
    printf("time checks per iteration: %" PRIu64 "\n", checks);
    printf("max. jitter: %" PRIu64 "\n", max_diff);

    return EXIT_SUCCESS;
}
