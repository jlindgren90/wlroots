#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>

void wlr_box_closest_point(const struct wlr_box *box, double x, double y,
		double *dest_x, double *dest_y) {
	// if box is empty, then it contains no points, so no closest point either
	if (wlr_box_empty(box)) {
		*dest_x = NAN;
		*dest_y = NAN;
		return;
	}

	// Note: the width and height of the box are exclusive; that is,
	// for a 100x100 box at (0,0), the point (99,99) is inside it
	// while the point (100,100) is outside it.
	//
	// Mathematically, there exists no single closest point to the
	// bottom-right corner of the box while remaining inside it. You
	// can construct an infinite series approaching the limit, such
	// as {(99,99), (99.9,99.9), (99.99,99.99)...}, but since the
	// intervals are half-open, there is no "last" point.
	//
	// This function must therefore define an arbitrary "closest"
	// point. For simplicity and consistency, this is defined to be
	// (box.x + width - 1, box.y + height - 1).
	//
	// (The previous implementation was non-linear: with the example
	// 100x100 box, it would return an input point of (99.9,99.9)
	// unchanged, but for an input point (100.1,100.1) the returned
	// point would jump back to (99.0,99.0). This is now fixed.)

	// find the closest x point
	if (x < box->x) {
		*dest_x = box->x;
	} else if (x > box->x + box->width - 1) {
		*dest_x = box->x + box->width - 1;
	} else {
		*dest_x = x;
	}

	// find closest y point
	if (y < box->y) {
		*dest_y = box->y;
	} else if (y > box->y + box->height - 1) {
		*dest_y = box->y + box->height - 1;
	} else {
		*dest_y = y;
	}
}

bool wlr_box_empty(const struct wlr_box *box) {
	return box == NULL || box->width <= 0 || box->height <= 0;
}

bool wlr_box_intersection(struct wlr_box *dest, const struct wlr_box *box_a,
		const struct wlr_box *box_b) {
	bool a_empty = wlr_box_empty(box_a);
	bool b_empty = wlr_box_empty(box_b);

	if (a_empty || b_empty) {
		*dest = (struct wlr_box){0};
		return false;
	}

	int x1 = fmax(box_a->x, box_b->x);
	int y1 = fmax(box_a->y, box_b->y);
	int x2 = fmin(box_a->x + box_a->width, box_b->x + box_b->width);
	int y2 = fmin(box_a->y + box_a->height, box_b->y + box_b->height);

	dest->x = x1;
	dest->y = y1;
	dest->width = x2 - x1;
	dest->height = y2 - y1;

	return !wlr_box_empty(dest);
}

bool wlr_box_contains_point(const struct wlr_box *box, double x, double y) {
	if (wlr_box_empty(box)) {
		return false;
	} else {
		return x >= box->x && x < box->x + box->width &&
			y >= box->y && y < box->y + box->height;
	}
}

bool wlr_box_contains_box(const struct wlr_box *bigger, const struct wlr_box *smaller) {
	if (wlr_box_empty(bigger) || wlr_box_empty(smaller)) {
		return false;
	}

	return smaller->x >= bigger->x &&
		smaller->x + smaller->width <= bigger->x + bigger->width &&
		smaller->y >= bigger->y &&
		smaller->y + smaller->height <= bigger->y + bigger->height;
}

void wlr_box_transform(struct wlr_box *dest, const struct wlr_box *box,
		enum wl_output_transform transform, int width, int height) {
	struct wlr_box src = {0};
	if (box != NULL) {
		src = *box;
	}

	if (transform % 2 == 0) {
		dest->width = src.width;
		dest->height = src.height;
	} else {
		dest->width = src.height;
		dest->height = src.width;
	}

	switch (transform) {
	case WL_OUTPUT_TRANSFORM_NORMAL:
		dest->x = src.x;
		dest->y = src.y;
		break;
	case WL_OUTPUT_TRANSFORM_90:
		dest->x = height - src.y - src.height;
		dest->y = src.x;
		break;
	case WL_OUTPUT_TRANSFORM_180:
		dest->x = width - src.x - src.width;
		dest->y = height - src.y - src.height;
		break;
	case WL_OUTPUT_TRANSFORM_270:
		dest->x = src.y;
		dest->y = width - src.x - src.width;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED:
		dest->x = width - src.x - src.width;
		dest->y = src.y;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_90:
		dest->x = src.y;
		dest->y = src.x;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_180:
		dest->x = src.x;
		dest->y = height - src.y - src.height;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_270:
		dest->x = height - src.y - src.height;
		dest->y = width - src.x - src.width;
		break;
	}
}

bool wlr_fbox_empty(const struct wlr_fbox *box) {
	return box == NULL || box->width <= 0 || box->height <= 0;
}

void wlr_fbox_transform(struct wlr_fbox *dest, const struct wlr_fbox *box,
		enum wl_output_transform transform, double width, double height) {
	struct wlr_fbox src = {0};
	if (box != NULL) {
		src = *box;
	}

	if (transform % 2 == 0) {
		dest->width = src.width;
		dest->height = src.height;
	} else {
		dest->width = src.height;
		dest->height = src.width;
	}

	switch (transform) {
	case WL_OUTPUT_TRANSFORM_NORMAL:
		dest->x = src.x;
		dest->y = src.y;
		break;
	case WL_OUTPUT_TRANSFORM_90:
		dest->x = height - src.y - src.height;
		dest->y = src.x;
		break;
	case WL_OUTPUT_TRANSFORM_180:
		dest->x = width - src.x - src.width;
		dest->y = height - src.y - src.height;
		break;
	case WL_OUTPUT_TRANSFORM_270:
		dest->x = src.y;
		dest->y = width - src.x - src.width;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED:
		dest->x = width - src.x - src.width;
		dest->y = src.y;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_90:
		dest->x = src.y;
		dest->y = src.x;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_180:
		dest->x = src.x;
		dest->y = height - src.y - src.height;
		break;
	case WL_OUTPUT_TRANSFORM_FLIPPED_270:
		dest->x = height - src.y - src.height;
		dest->y = width - src.x - src.width;
		break;
	}
}

#ifdef WLR_USE_UNSTABLE

bool wlr_box_equal(const struct wlr_box *a, const struct wlr_box *b) {
	if (wlr_box_empty(a)) {
		a = NULL;
	}
	if (wlr_box_empty(b)) {
		b = NULL;
	}

	if (a == NULL || b == NULL) {
		return a == b;
	}

	return a->x == b->x && a->y == b->y &&
		a->width == b->width && a->height == b->height;
}

bool wlr_fbox_equal(const struct wlr_fbox *a, const struct wlr_fbox *b) {
	if (wlr_fbox_empty(a)) {
		a = NULL;
	}
	if (wlr_fbox_empty(b)) {
		b = NULL;
	}

	if (a == NULL || b == NULL) {
		return a == b;
	}

	return a->x == b->x && a->y == b->y &&
		a->width == b->width && a->height == b->height;
}

#endif
