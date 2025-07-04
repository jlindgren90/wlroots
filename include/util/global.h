#ifndef UTIL_GLOBAL_H
#define UTIL_GLOBAL_H

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Destroy a transient global.
 *
 * Globals that are created and destroyed on the fly need special handling to
 * prevent race conditions with wl_registry. Use this function to destroy them.
 */
void wlr_global_destroy_safe(struct wl_global *global);

#ifdef __cplusplus
}
#endif

#endif
