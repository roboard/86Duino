#ifndef __WZ_INI_H
#define __WZ_INI_H

#include <stdlib.h>

class INIFile {
public:
    INIFile();
    ~INIFile();

    int open(const char *ifname, const char *ofname = NULL);
    int commit();
    int close();

    char *get(const char *section_name, const char *key_name, char *value, size_t value_size);
    int add(const char *section_name, const char *key_name, char *value);
    int add(const char *section_name, const char *key_name, int value);
    int add(const char *section_name, const char *key_name, double value);
    int del(const char *section_name);
    int del(const char *section_name, const char *key_name);

private:
    void *private_data;

};

#endif
