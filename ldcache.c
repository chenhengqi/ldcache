#include "ldcache.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CACHE_MAGIC "ld.so-1.7.0"
#define CACHE_MAGIC_NEW "glibc-ld.so.cache1.1"

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

static inline const char *align(const char *address, size_t boundary)
{
	if ((size_t)address % boundary == 0) {
		return address;
	}
	return (address + boundary) - ((size_t)address % boundary);
}

struct cache_entry *parse(const char *path)
{
	FILE * f;
	char *data, *pos, *strs;
	long size;
	struct header *hdr;
	struct header_new *hdr_new;
	struct entry_new *entry_new;
	struct cache_entry *entries;
	int i, n;

	f = fopen(path, "rb");
	if (!f) {
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	data = malloc(size);
	if (!data) {
		fclose(f);
		return NULL;
	}
	fread(data, 1, size, f);

	printf("read file ok\n");

	pos = data;
	hdr = (struct header *)pos;
	pos += sizeof(struct header);
	if (pos >= data + size) {
		return NULL;
	}

	pos += hdr->lib_count * sizeof(struct entry);
	if (pos >= data + size) {
		return NULL;
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
	pos += hdr_new->lib_count * sizeof(struct entry_new);
	if (pos >= data + size) {
		return NULL;
	}

	strs = (char*)hdr_new;

	pos += hdr_new->strs_len;
	if (pos - data != size) {
		return NULL;
	}

	printf("get strs ok\n");

	if (strncmp(hdr->magic, CACHE_MAGIC, sizeof(CACHE_MAGIC) - 1)) {
		return NULL;
	}

	if (strncmp(hdr_new->magic, CACHE_MAGIC_NEW, sizeof(CACHE_MAGIC_NEW) - 1)) {
		return NULL;
	}

	printf("check magic ok\n");

	if (*(pos - 1) != '\0') {
		return NULL;
	}

	entries = calloc(hdr_new->lib_count + 1, sizeof(struct cache_entry));
	if (!entries) {
		return NULL;
	}
	bzero(entries, (hdr_new->lib_count + 1) * sizeof(struct cache_entry));

	printf("alloc memory ok\n");

	for (i = 0, n = 0; i < hdr_new->lib_count; i++) {
		if (!(entry_new[i].flags & 0x01)) {
			continue;
		}

		if (strs + entry_new[i].key >= pos) {
			return NULL;
		}

		if (strs + entry_new[i].value >= pos) {
			return NULL;
		}

		strncpy(entries[n].name, &strs[entry_new[i].key],
			strlen(&strs[entry_new[i].key]));
		strncpy(entries[n].path, &strs[entry_new[i].value],
			strlen(&strs[entry_new[i].value]));
		n++;
	}

	return entries;
}
