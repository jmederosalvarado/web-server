#include <utils.h>
#include <string.h>

void array_shift_left(void *array_start, void *array_end, size_t size)
{
    char *start = (char *)array_start;
    char *end = (char *)array_end;

    while (start < end - size)
    {
        strncpy(start, start + size, size);
        start += size;
    }
}
