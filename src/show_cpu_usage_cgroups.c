#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>

#define MAX_LINE_SIZE 1024

static uint64_t now_nsec(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (((uint64_t) now.tv_sec) * 1000UL * 1000UL * 1000UL) + ((uint64_t) now.tv_nsec);
}

static uint64_t get_cpu_usage(void)
{
    uint64_t result = 0;
    FILE * file = fopen("/sys/fs/cgroup/cpuacct/cpuacct.usage", "rb");
    if (NULL != file)
    {
        char line[MAX_LINE_SIZE];
        if (NULL != fgets(line, MAX_LINE_SIZE, file))
        {
            char* c = &(line[0]);
            while (('0' <= (*c)) && ((*c) <= '9'))
            {
                result *= 10;
                result += (*c) - '0';
                c++;
            }
        }
        fclose(file);
    }

    return result;
}

int main(int arg, char* argv[])
{
    uint64_t start_time = now_nsec();
    uint64_t start_usage = get_cpu_usage();

    for(;;)
    {
        sleep(1);

        uint64_t const end_time = now_nsec();
        uint64_t const end_usage = get_cpu_usage();
        uint64_t const usage = end_usage - start_usage;
        uint64_t const total = end_time - start_time;

        printf("cpu usage: %" PRIu64 "%%  \n\x1b[1F", usage * 100UL / total);
        start_time = end_time;
        start_usage = end_usage;
    }

    return EXIT_SUCCESS;
}
