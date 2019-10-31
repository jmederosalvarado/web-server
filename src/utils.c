#include <utils.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int read_line(int fd, char *buf, size_t n)
{
    int rc = read(fd, buf, n);
    if (rc == -1)
        return -1;

    printf("--> Request:\n");
    printf("\n%s\n", buf);

    int count = 0;
    for (int i = 0; i < strlen(buf); i++)
    {
        if (buf[i] == '\n')
        {
            buf[i] = '\0';
            break;
        }
        count++;
    }
    return count;
}

int max(int a, int b)
{
    return a > b ? a : b;
}

int min(int a, int b)
{
    return a < b ? a : b;
}

void fix_spaces(char *str)
{
    int k = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (i < strlen(str) - 3 && str[i] == '%' && str[i + 1] == '2' && str[i + 2] == '0')
        {
            str[k++] = ' ';
            i += 2;
        }
        else
            str[k++] = str[i];
    }
    str[k] = '\0';
}