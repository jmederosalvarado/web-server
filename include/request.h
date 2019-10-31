#ifndef REQUEST_H
#define REQUEST_H

struct request
{
    char path[1024];
    char orderby[20];
};

#endif