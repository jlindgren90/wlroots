#ifndef TYPES_WLR_REGION_H
#define TYPES_WLR_REGION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct wl_client;

/*
 * Creates a new region resource with the provided new ID.
 */
struct wl_resource *region_create(struct wl_client *client,
	uint32_t version, uint32_t id);

#ifdef __cplusplus
}
#endif

#endif
