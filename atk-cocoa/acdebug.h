#include <glib.h>

G_BEGIN_DECLS

typedef enum {
	AC_DEBUG_HITTEST = 1 << 0,
	AC_DEBUG_FRAMES = 1 << 1,
	AC_DEBUG_ACTIONS = 1 << 2,
	AC_DEBUG_WIDGETS = 1 << 3,
	AC_DEBUG_TREE = 1 << 4,
	AC_DEBUG_LAYOUT = 1 << 5,

	/* Insert others here */

	AC_DEBUG_ALWAYS = 1 << 31
} AcDebugFlag;

#define AC_NOTE(type, action) G_STMT_START { \
	if (ac_debug_flags & AC_DEBUG_##type || AC_DEBUG_##type == AC_DEBUG_ALWAYS)	 \
		{ action; }; 						} G_STMT_END

extern guint ac_debug_flags;

G_END_DECLS