#include <path_utils.h>
#include <sys/stat.h>

void set_root(char *r)
{
    root = r;
}

char *get_root()
{
    return root;
}

int is_dir(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISDIR(path_stat.st_mode);
}

int is_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}