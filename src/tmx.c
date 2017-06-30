#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h> /* int32_t */
#include <errno.h>

#include "tmx.h"
#include "tmx_utils.h"

/*
	Public globals
*/

void* (*tmx_alloc_func) (void *address, size_t len) = NULL;
void  (*tmx_free_func ) (void *address) = NULL;
void* (*tmx_img_load_func) (const char *p) = NULL;
void  (*tmx_img_free_func) (void *address) = NULL;

/*
	Public functions
*/

static void set_alloc_functions() {
	if (!tmx_alloc_func) tmx_alloc_func = realloc;
	if (!tmx_free_func) tmx_free_func = free;
}

static void map_post_parsing(tmx_map *map) {
	if (map) {
		if (!mk_map_tile_array(map)) {
			tmx_map_free(map);
			map = NULL;
		}
	}
}

tmx_map* tmx_load(const char *path) {
	tmx_map *map = NULL;
	set_alloc_functions();
	map = parse_xml(path);
	map_post_parsing(map);
	return map;
}

tmx_map* tmx_load_buffer(const char *buffer, int len) {
	tmx_map *map = NULL;
	set_alloc_functions();
	map = parse_xml_buffer(buffer, len);
	map_post_parsing(map);
	return map;
}

tmx_map* tmx_load_fd(int fd) {
	tmx_map *map = NULL;
	set_alloc_functions();
	map = parse_xml_fd(fd);
	map_post_parsing(map);
	return map;
}

tmx_map* tmx_load_callback(tmx_read_functor callback, void *userdata) {
	tmx_map *map = NULL;
	set_alloc_functions();
	map = parse_xml_callback(callback, userdata);
	map_post_parsing(map);
	return map;
}

static void free_props(tmx_properties *h) {
	free_hashtable((void*)h, property_deallocator);
}

static void free_obj(tmx_object *o) {
	if (o) {
		free_obj(o->next);
		tmx_free_func(o->name);
		if (o->points) tmx_free_func(*(o->points));
		tmx_free_func(o->type);
		tmx_free_func(o->points);
		tmx_free_func(o);
	}
}

static void free_objgr(tmx_object_group *o) {
	if (o) {
		free_obj(o->head);
		tmx_free_func(o);
	}
}

static void free_image(tmx_image *i) {
	if (i) {
		tmx_free_func(i->source);
		if (tmx_img_free_func) {
			tmx_img_free_func(i->resource_image);
		}
		tmx_free_func(i);
	}
}

static void free_layers(tmx_layer *l) {
	if (l) {
		free_layers(l->next);
		tmx_free_func(l->name);
		if (l->type == L_LAYER)
			tmx_free_func(l->content.gids);
		else if (l->type == L_OBJGR)
			free_objgr(l->content.objgr);
		else if (l->type == L_IMAGE) {
			free_image(l->content.image);
		}
		free_props(l->properties);
		tmx_free_func(l);
	}
}

static void free_tiles(tmx_tile *t, int tilecount) {
	int i;
	if (t) {
		for (i=0; i<tilecount; i++) {
			free_props(t[i].properties);
			free_image(t[i].image);
			free_obj(t[i].collision);
			tmx_free_func(t[i].animation);
		}
	}
}

static void free_ts(tmx_tileset *ts) {
	if (ts) {
		free_ts(ts->next);
		tmx_free_func(ts->name);
		free_image(ts->image);
		free_props(ts->properties);
		free_tiles(ts->tiles, ts->tilecount);
		tmx_free_func(ts->tiles);
		tmx_free_func(ts);
	}
}

void tmx_map_free(tmx_map *map) {
	if (map) {
		free_ts(map->ts_head);
		free_props(map->properties);
		free_layers(map->ly_head);
		tmx_free_func(map->tiles);
		tmx_free_func(map);
	}
}

tmx_tile* tmx_get_tile(tmx_map *map, unsigned int gid) {
	if (!map) {
		tmx_err(E_INVAL, "tmx_get_tile: invalid argument: map is NULL");
		return NULL;
	}

	gid &= TMX_FLIP_BITS_REMOVAL;

	if (gid < map->tilecount) return map->tiles[gid];

	return NULL;
}

tmx_property* tmx_get_property(tmx_properties *hash, const char *key) {
	if (hash == NULL) {
		return NULL;
	}
	return (tmx_property*) hashtable_get((void*)hash, key);
}

struct property_foreach_data {
	tmx_property_functor callback;
	void *userdata;
};

static void property_foreach(void *val, void *userdata, const char *key UNUSED) {
	struct property_foreach_data *holder = ((struct property_foreach_data*)userdata);
	holder->callback((tmx_property*)val, holder->userdata);
}

void tmx_property_foreach(tmx_properties *hash, tmx_property_functor callback, void *userdata) {
	struct property_foreach_data holder;
	holder.callback = callback;
	holder.userdata = userdata;
	hashtable_foreach((void*)hash, property_foreach, &holder);
}
