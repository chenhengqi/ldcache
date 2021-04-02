#include "ldcache.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CACHE_MAGIC "ld.so-1.7.0"
#define CACHE_MAGIC_NEW "glibc-ld.so.cache1.1"

#define FLAG_ELF 0x01

struct header
{
	char magic[sizeof(CACHE_MAGIC) - 1];
	uint32_t lib_count;
};

struct entry
{
	int32_t flags;
	uint32_t key;
	uint32_t value;
};

struct header_new
{
	char magic[sizeof(CACHE_MAGIC_NEW) - 1];
	uint32_t lib_count;
	uint32_t strs_len;
	uint32_t unused[5];
};

struct entry_new
{
	int32_t flags;
	uint32_t key;
	uint32_t value;
	uint32_t os_ver;
	uint64_t hwcap;
};

static const char *align(const char *address, size_t alignment)
{
	if ((size_t)address % alignment == 0) {
		return address;
	}
	return (address + alignment) - ((size_t)address % alignment);
}

static struct ldcache *parse(const char *data, long size, int old) {
	char *pos, *strs;
	struct header *hdr;
	struct header_new *hdr_new;
	struct entry_new *entry_new;
	struct ldcache *cache;
	int i, n;

	pos = (char *)data;

	if (old) {
		hdr = (struct header *)pos;
		pos += sizeof(struct header);
		if (pos >= data + size) {
			return NULL;
		}

		pos += sizeof(struct entry) * hdr->lib_count;
		if (pos >= data + size) {
			return NULL;
		}
	}

	pos = (char *)align(pos, __alignof__(struct header_new));
	if (pos >= data + size) {
		return NULL;
	}

	hdr_new = (struct header_new *)pos;
	pos += sizeof(struct header_new);
	if (pos >= data + size) {
		return NULL;
	}

	entry_new = (struct entry_new *)pos;
	pos += sizeof(struct entry_new) * hdr_new->lib_count;
	if (pos >= data + size) {
		return NULL;
	}

	strs = (char *)hdr_new;

	pos += hdr_new->strs_len;
	if (pos - data != size) {
		return NULL;
	}

	if (old) {
		if (strncmp(hdr->magic, CACHE_MAGIC, sizeof(CACHE_MAGIC) - 1)) {
			return NULL;
		}
	}

	if (strncmp(hdr_new->magic, CACHE_MAGIC_NEW, sizeof(CACHE_MAGIC_NEW) - 1)) {
		return NULL;
	}

	if (*(pos - 1) != '\0') {
		return NULL;
	}

	cache = malloc(sizeof(struct ldcache));
	if (!cache) {
		return NULL;
	}
	bzero(cache, sizeof(struct ldcache));

	cache->entries = calloc(hdr_new->lib_count, sizeof(struct ldcache_entry));
	if (!cache->entries) {
		free(cache);
		return NULL;
	}
	bzero(cache->entries, sizeof(struct ldcache_entry) * hdr_new->lib_count);

	for (i = 0, n = 0; i < hdr_new->lib_count; i++) {
		if (!(entry_new[i].flags & FLAG_ELF)) {
			continue;
		}

		if (strs + entry_new[i].key >= pos) {
			goto cleanup;
		}

		if (strs + entry_new[i].value >= pos) {
			goto cleanup;
		}

		strncpy(cache->entries[n].name, &strs[entry_new[i].key],
			strlen(&strs[entry_new[i].key]));
		strncpy(cache->entries[n].path, &strs[entry_new[i].value],
			strlen(&strs[entry_new[i].value]));
		n++;
	}

	cache->size = n;
	return cache;

cleanup:
	free(cache->entries);
	free(cache);
	return NULL;
}

struct ldcache *parse_ldcache(const char *path)
{
	struct ldcache *cache = NULL;
	FILE *f;
	char *data;
	long size;

	f = fopen(path, "rb");
	if (!f) {
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);
	data = malloc(size);
	if (!data) {
		fclose(f);
		return NULL;
	}
	fread(data, 1, size, f);
	fclose(f);

	if (data[0] == 'l') {
		cache = parse(data, size, 1);
	}

	if (data[0] == 'g') {
		cache = parse(data, size, 0);
	}

	free(data);
	return cache;
}

void free_ldcache(struct ldcache *cache)
{
	if (!cache) {
		return;
	}
	free(cache->entries);
	free(cache);
}
