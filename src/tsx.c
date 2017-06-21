#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h> /* int32_t */
#include <errno.h>

#include "tsx.h"
#include "tmx_utils.h"

/*
	Public functions
*/

tmx_tileset_manager* tmx_make_tileset_manager() {
	return NULL;
}

void tmx_free_tileset_manager(tmx_tileset_manager *h) {
	free_hashtable((void*)h, tileset_deallocator);
}

int tmx_load_tileset(const char *path, tmx_tileset_manager *ts_mgr) {
	tmx_tileset *ts;

	if (ts_mgr == NULL) return 0;

	ts = parse_tsx_xml(path);
	if (ts) {
		hashtable_set((void*)ts_mgr, path, (void*)ts, tileset_deallocator);
		return 1;
	}

	return 0;
}

int tmx_load_tileset_buffer(const char *buffer, int len, const char *key, tmx_tileset_manager *ts_mgr) {
	tmx_tileset *ts;

	if (ts_mgr == NULL) return 0;

	ts = parse_tsx_xml_buffer(buffer, len);
	if (ts) {
		hashtable_set((void*)ts_mgr, key, (void*)ts, tileset_deallocator);
		return 1;
	}

	return 0;
}

int tmx_load_tileset_fd(int fd, const char *key, tmx_tileset_manager *ts_mgr) {
	tmx_tileset *ts;

	if (ts_mgr == NULL) return 0;

	ts = parse_tsx_xml_fd(fd);
	if (ts) {
		hashtable_set((void*)ts_mgr, key, (void*)ts, tileset_deallocator);
		return 1;
	}

	return 0;
}

int tmx_load_tileset_callback(tmx_read_functor callback, void *userdata, const char *key, tmx_tileset_manager *ts_mgr) {
	tmx_tileset *ts;

	if (ts_mgr == NULL) return 0;

	ts = parse_tsx_xml_callback(callback, userdata);
	if (ts) {
		hashtable_set((void*)ts_mgr, key, (void*)ts, tileset_deallocator);
		return 1;
	}

	return 0;
}
