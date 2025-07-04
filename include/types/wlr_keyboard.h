#include <wlr/types/wlr_keyboard.h>

#ifdef __cplusplus
extern "C" {
#endif

void keyboard_key_update(struct wlr_keyboard *keyboard,
		struct wlr_keyboard_key_event *event);

bool keyboard_modifier_update(struct wlr_keyboard *keyboard);

void keyboard_led_update(struct wlr_keyboard *keyboard);

#ifdef __cplusplus
}
#endif
