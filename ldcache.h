#pragma once

#include <linux/limits.h>

struct ldcache_entry
{
	char name[NAME_MAX];
	char path[PATH_MAX];
};

struct ldcache
{
	struct ldcache_entry *entries;
	int size;
};

struct ldcache *parse_ldcache(const char *path);
void free_ldcache(struct ldcache *cache);
