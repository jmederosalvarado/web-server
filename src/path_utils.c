#include <path_utils.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

char *ctime(const time_t *timep);

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

void get_permissions(struct path *p, struct stat container);
void get_type(struct path *p, struct stat container);
void get_size(struct path *p, struct stat container);
void get_moddate(struct path *p, struct stat container);

void get_info(char *path, struct path *p)
{
    struct stat container;

    if (stat(path, &container) < 0)
    {
        printf("ERROR: %s\n", path);
        perror("stat");
    }

    get_permissions(p, container);
    get_type(p, container);
    get_size(p, container);
    get_moddate(p, container);
}

void get_permissions(struct path *p, struct stat container)
{
    strcpy(p->permissions, "");
    strcat(p->permissions, (S_ISDIR(container.st_mode)) ? "d" : "-");
    strcat(p->permissions, (container.st_mode & S_IRUSR) ? "r" : "-");
    strcat(p->permissions, (container.st_mode & S_IWUSR) ? "w" : "-");
    strcat(p->permissions, (container.st_mode & S_IXUSR) ? "x" : "-");
    strcat(p->permissions, (container.st_mode & S_IRGRP) ? "r" : "-");
    strcat(p->permissions, (container.st_mode & S_IWGRP) ? "w" : "-");
    strcat(p->permissions, (container.st_mode & S_IXGRP) ? "x" : "-");
    strcat(p->permissions, (container.st_mode & S_IROTH) ? "r" : "-");
    strcat(p->permissions, (container.st_mode & S_IWOTH) ? "w" : "-");
    strcat(p->permissions, (container.st_mode & S_IXOTH) ? "x" : "-");
}

void get_type(struct path *p, struct stat container)
{
    switch (container.st_mode & __S_IFMT)
    {
    case __S_IFBLK:
        strcpy(p->type, "block device mode");
        break;

    case __S_IFCHR:
        strcpy(p->type, "character device mode");
        break;

    case __S_IFDIR:
        strcpy(p->type, "directory");
        break;

    case __S_IFIFO:
        strcpy(p->type, "FIFO");
        break;

    case __S_IFLNK:
        strcpy(p->type, "symbolic link");
        break;

    case __S_IFREG:
        strcpy(p->type, "regular file");
        break;

    case __S_IFSOCK:
        strcpy(p->type, "socket");
        break;

    default:
        strcpy(p->type, "unknow");
    }
}

void get_size(struct path *p, struct stat container)
{
    p->size = container.st_size;
}

void get_moddate(struct path *p, struct stat container)
{
    char *tempdate;
    tempdate = ctime(&container.st_mtime);
    strcpy(p->moddate, tempdate);
}