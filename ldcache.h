#pragma once

#include <linux/limits.h>

struct cache_entry
{
	char name[NAME_MAX];
	char path[PATH_MAX];
};

struct cache_entry *parse(const char *path);
