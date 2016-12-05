/*
 * AtkCocoa
 * Copyright 2016 Microsoft Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

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
