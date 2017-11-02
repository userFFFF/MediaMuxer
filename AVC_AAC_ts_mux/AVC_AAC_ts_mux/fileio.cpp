#include "stdafx.h"
#include <iostream>

FILE *open(char *pfilename, char *mode)
{
    return fopen(pfilename, mode);
}

int write(FILE *fd, const void *data, int size)
{
    return fwrite(data, sizeof(char), size, fd);
}

int seek(FILE *fd, int pos, int mod)
{
    return fseek(fd, pos, mod);
}

int read(FILE *fd, void *data, int size)
{
    return fread(data, sizeof(char), size, fd);
}

int close(FILE *fd)
{
    return fclose(fd);
}