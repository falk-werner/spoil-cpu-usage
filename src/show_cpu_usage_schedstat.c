#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <time.h>

#define MAX_LINE_SIZE 1024
#define MAX_CPU_COUNT 32

#define RUNNING_NS_ID 7

struct total_cpu_usage
{
    size_t cpu_count;
    uint64_t timestamp_ns;
    uint64_t usage_ns[MAX_CPU_COUNT];
};

struct cpu_usage
{
    size_t cpu_id;
    uint64_t running_ns;
};

static bool startswith(char const * line, char const * prefix)
{
    return (0 == strncmp(line, prefix, strlen(prefix)));
}

static void parse_cpu_usage(char const * line, struct cpu_usage * result)
{
    result->cpu_id = 0;
    result->running_ns = 0UL;

    // read CPU ID
    size_t pos = 3;    
    while (('0' <= line[pos]) && (line[pos] <= '9'))
    {
        result->cpu_id *= 10;
        result->cpu_id += line[pos] - '0';
        pos++;
    }

    size_t field_id = 0;
    // read fields
    while (line[pos] == ' ')
    {
        field_id++;
        pos++;

        uint64_t buffer = 0;
        while (('0' <= line[pos]) && (line[pos] <= '9'))
        {
            buffer *= 10;
            buffer += line[pos] - '0';
            pos++;
        }

        if (field_id == RUNNING_NS_ID)
        {
            result->running_ns = buffer;
        }
    }
}

static uint64_t now_nsec(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (((uint64_t) now.tv_sec) * 1000UL * 1000UL * 1000UL) + ((uint64_t) now.tv_nsec);
}


static void get_cpu_usage(struct total_cpu_usage * result)
{
    memset(result, 0, sizeof(struct total_cpu_usage));
    result->timestamp_ns = now_nsec();

    FILE * file = fopen("/proc/schedstat", "rb");
    if (NULL != file)
    {
        char line[MAX_LINE_SIZE];
        while (NULL != fgets(line, MAX_LINE_SIZE, file))
        {
            if (startswith(line, "cpu"))
            {
                struct cpu_usage usage;
                parse_cpu_usage(line, &usage);

                if (usage.cpu_id < MAX_CPU_COUNT)
                {
                    result->usage_ns[usage.cpu_id] = usage.running_ns;
                    if (result->cpu_count <= usage.cpu_id)
                    {
                        result->cpu_count = usage.cpu_id + 1;
                    }
                }
            }

        }

        fclose(file);
    }
}

int main(int argc, char* argv[])
{
    struct total_cpu_usage start;
    get_cpu_usage(&start);

    printf("\x1b[2J\x1b[HCPU Usage using /proc/schedstat\n");
    for(;;)
    {
        sleep(1);

        struct total_cpu_usage end;
        get_cpu_usage(&end);

        for(size_t cpu = 0; cpu < end.cpu_count; cpu++)
        {
            uint64_t const usage_ns = end.usage_ns[cpu] - start.usage_ns[cpu];
            uint64_t const time_ns = end.timestamp_ns - start.timestamp_ns;
            uint64_t const usage = (usage_ns * 100UL) / time_ns;
            printf("CPU #%zu: %" PRIu64 "%%  \n", cpu, usage);
        }
        printf("\n\x1b[2;0H");

        memcpy(&start, &end, sizeof(struct total_cpu_usage));
    }

    return EXIT_SUCCESS;
}