// This tool captures CPU usage by reading /proc/stat,
// which leads to the same error previously observed
// with htop.

#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#define MAX_LINE_SIZE 1024

struct cpu_usage
{
    uint64_t total;
    uint64_t idle;
};

enum parse_mode
{
    WAIT_FOR_START,
    PARSE_NUMBER
};

bool startswith(char const * line, char const * prefix)
{
    return (0 == strncmp(line, prefix, strlen(prefix)));
}

// https://man7.org/linux/man-pages/man5/proc_stat.5.html
void parse_cpu_usage(char const * line, struct cpu_usage * result)
{   
    result->total = 0;
    result->idle = 0;

    uint64_t buffer = 0;
    size_t field_id = 1;
    enum parse_mode mode = WAIT_FOR_START;
    for(size_t pos = 4; line[pos] != '\0'; pos++)
    {
        char const c = line[pos];

        switch (mode)
        {
            case WAIT_FOR_START:
                if (('0' <= c) && (c <= '9'))
                {
                    mode = PARSE_NUMBER;
                    buffer = c - '0';
                }
                break;
            case PARSE_NUMBER:
                if (('0' <= c) && (c <= '9'))
                {
                    buffer *= 10;
                    buffer += c - '0';
                }
                else
                {
                    result->total += buffer;
                    if (field_id == 4)
                    {
                        result->idle = buffer;
                    }
                    mode = WAIT_FOR_START;
                    buffer = 0;
                    field_id++;
                }
                break;
        }
    }
}

static void get_cpu_usage(struct cpu_usage * result)
{
    result->idle = 0;
    result->total = 0;

    FILE* file = fopen("/proc/stat", "rb");
    if (NULL != file)
    {
        char line[MAX_LINE_SIZE];
        while (NULL != fgets(line, MAX_LINE_SIZE, file))
        {
            if (startswith(line, "cpu ")) {
                parse_cpu_usage(line, result);
                break;
            }
        }
        fclose(file);
    }

}

int main(int argc, char* argv[])
{

    struct cpu_usage usage;
    get_cpu_usage(&usage);

    for(;;)
    {
        sleep(1);

        struct cpu_usage next;
        get_cpu_usage(&next);

        uint64_t const total = next.total - usage.total;
        uint64_t const idle = next.idle - usage.idle;
        
        printf("cpu usage: %" PRIu64 "%%\n", 100 - ((idle * 100UL) / total));
        usage.total = next.total;
        usage.idle = next.idle;
    }

    return EXIT_SUCCESS;
}