#include <assert.h>
#include <stdlib.h>
#include <backend/drm/drm.h>
#include <render/eglstreams_allocator.h>
#include <wlr/backend/multi.h>
#include <wlr/interfaces/wlr_buffer.h>
#include <wlr/render/gles2.h>

static const struct wlr_buffer_impl buffer_impl;

static struct wlr_eglstream_buffer *
get_eglstreams_buffer_from_buffer(struct wlr_buffer *buffer) {
	assert(buffer->impl == &buffer_impl);
	return (struct wlr_eglstream_buffer *)buffer;
}

static struct wlr_eglstream_buffer *
create_buffer(struct wlr_eglstreams_allocator *alloc,
	      struct wlr_eglstream_plane *plane) {
	struct wlr_eglstream_buffer *buffer = calloc(1, sizeof(*buffer));
	if (!buffer)
		return NULL;

	wlr_buffer_init(&buffer->base, &buffer_impl, plane->width, plane->height);
	buffer->base.egl_stream = &plane->stream;
	buffer->plane = plane;
	plane->locks++;
	return buffer;
}

static void plane_unlock(struct wlr_eglstream_plane *plane) {
	if (!plane || (plane->locks > 0 && --plane->locks > 0))
		return;

	wlr_egl_destroy_eglstreams_surface(&plane->stream);
	wl_list_remove(&plane->link);
	plane->stream.stream = EGL_NO_STREAM_KHR;
	free(plane);
}

static void buffer_destroy(struct wlr_buffer *wlr_buffer) {
	struct wlr_eglstream_buffer *buffer =
		get_eglstreams_buffer_from_buffer(wlr_buffer);

	plane_unlock(buffer->plane);
	free(buffer);
}

static bool buffer_get_dmabuf(struct wlr_buffer *wlr_buffer,
		struct wlr_dmabuf_attributes *attribs) {
	return false; // TODO
}

static const struct wlr_buffer_impl buffer_impl = {
	.destroy = buffer_destroy,
	.get_dmabuf = buffer_get_dmabuf,
};

static const struct wlr_allocator_interface allocator_impl;

static struct wlr_eglstreams_allocator *
get_eglstreams_alloc_from_alloc(struct wlr_allocator *alloc) {
	assert(alloc->impl == &allocator_impl);
	return (struct wlr_eglstreams_allocator *)alloc;
}

static void multi_backend_callback(struct wlr_backend *backend, void *data) {
	if (wlr_backend_is_drm(backend)) {
		((struct wlr_eglstreams_allocator *)data)->drm =
			(struct wlr_drm_backend *)backend;
	}
}

struct wlr_allocator *
wlr_eglstreams_allocator_create(struct wlr_backend *backend,
		struct wlr_renderer *renderer, uint32_t buffer_caps) {
	assert(wlr_backend_is_multi(backend));
	struct wlr_eglstreams_allocator *alloc = calloc(1, sizeof(*alloc));
	if (!alloc)
		return NULL;

	wlr_allocator_init(&alloc->base, &allocator_impl, buffer_caps);
	wlr_multi_for_each_backend(backend, multi_backend_callback, alloc);
	assert(alloc->drm);

	alloc->egl = wlr_gles2_renderer_get_egl(renderer);
	wl_list_init(&alloc->planes);
	return &alloc->base;
}

static void allocator_destroy(struct wlr_allocator *wlr_alloc) {
	free(get_eglstreams_alloc_from_alloc(wlr_alloc));
}

struct wlr_eglstream_plane *
wlr_eglstream_plane_for_id(struct wlr_allocator *wlr_alloc, uint32_t plane_id) {
	struct wlr_eglstreams_allocator *alloc =
		get_eglstreams_alloc_from_alloc(wlr_alloc);

	struct wlr_eglstream_plane *plane;
	wl_list_for_each(plane, &alloc->planes, link) {
		if (plane->id == plane_id)
			return plane;
	}
	return NULL;
}

static struct wlr_eglstream_plane *
find_or_create_plane(struct wlr_eglstreams_allocator *alloc, int width,
		int height, uint32_t plane_id) {
	if (!alloc->egl)
		return NULL;

	struct wlr_eglstream_plane *plane =
		wlr_eglstream_plane_for_id(&alloc->base, plane_id);

	if (!plane || width != plane->width || height != plane->height) {
		plane = calloc(1, sizeof(*plane));
		if (!plane)
			return NULL;

		plane->id = plane_id;
		plane->stream.drm = alloc->drm;
		plane->stream.egl = alloc->egl;
		plane->width = width;
		plane->height = height;

		if (!wlr_egl_create_eglstreams_surface(&plane->stream, plane_id,
				width, height)) {
			free(plane);
			return NULL;
		}
		wl_list_insert(&alloc->planes, &plane->link);
	}
	return plane;
}

static struct wlr_buffer *
allocator_create_buffer(struct wlr_allocator *wlr_alloc, int width, int height,
		const struct wlr_drm_format *format, void *data) {
	struct wlr_eglstreams_allocator *alloc =
		get_eglstreams_alloc_from_alloc(wlr_alloc);
	struct wlr_eglstream_plane *plane =
		find_or_create_plane(alloc, width, height, (uint32_t)(long)data);
	if (!plane)
		return NULL;

	struct wlr_eglstream_buffer *buffer = create_buffer(alloc, plane);
	return buffer ? &buffer->base : NULL;
}

static const struct wlr_allocator_interface allocator_impl = {
	.destroy = allocator_destroy,
	.create_buffer = allocator_create_buffer,
};
