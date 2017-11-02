#pragma once
#include <iostream>

extern FILE *open(char *pfilename, char *mode);

extern int write(FILE *fd, const void *data, int size);

extern int seek(FILE *fd, int pos, int mod);

extern int read(FILE *fd, void *data, int size);

extern int close(FILE *fd);
