#include <utils.h>
#include <string.h>
#include <unistd.h>

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

int readn(int fd, char *buf, size_t n)
{
    int count, rc;
    while (n--)
    {
        if ((rc = read(fd, buf, 1)) == 1)
        {
            if (*buf == '\n')
                break;
            else
            {
                buf++;
                count++;
            }
        }
        else if (rc == 0)
            break;
        else
            return -1;
    }
    *buf = '\0';
    return count;
}
