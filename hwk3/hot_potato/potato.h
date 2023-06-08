#ifndef __POTATO_H__
#define __POTATO_H__

#include <string.h>
#include <cstdio>
#include <cstdlib>

class Potato {
public:
    int hops_num;
    int path_length;
    int path[512];
    Potato() : hops_num(0), path_length(0) {
        memset(path, 0, sizeof(path));
    }
};

#endif