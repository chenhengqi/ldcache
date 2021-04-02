#include <stdio.h>

#include "ldcache.h"

int main(int argc, char **argv) {
    struct ldcache *cache;
    int i;

    if (argc < 2) {
        printf("missing ld.so.cache\n");
        return -1;
    }

    cache = parse_ldcache(argv[1]);
    if (!cache) {
        return -1;
    }

    for (i = 0; i < cache->size; i++) {
        printf("%32s %32s\n", cache->entries[i].name, cache->entries[i].path);
    }

    free_ldcache(cache);
    return 0;
}
