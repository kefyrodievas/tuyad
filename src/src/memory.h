#pragma once

struct meminfo
{
    int total;
    int free;
    int shared;
    int buffered;
};

int get_memory_info(struct meminfo *memory);