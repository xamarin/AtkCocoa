/* GAIL - The GNOME Accessibility Implementation Library
 * Copyright 2002 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GAIL_RADIO_SUB_MENU_ITEM_H__
#define __GAIL_RADIO_SUB_MENU_ITEM_H__

#include "gailchecksubmenuitem.h"

G_BEGIN_DECLS

#define GAIL_TYPE_RADIO_SUB_MENU_ITEM               (gail_radio_sub_menu_item_get_type ())
#define GAIL_RADIO_SUB_MENU_ITEM(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), GAIL_TYPE_RADIO_SUB_MENU_ITEM, GailRadioSubMenuItem))
#define GAIL_RADIO_SUB_MENU_ITEM_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), GAIL_TYPE_RADIO_SUB_MENU_ITEM, GailRadioSubMenuItemClass))
#define GAIL_IS_RADIO_SUB_MENU_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GAIL_TYPE_RADIO_SUB_MENU_ITEM))
#define GAIL_IS_RADIO_SUB_MENU_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), GAIL_TYPE_RADIO_SUB_MENU_ITEM))
#define GAIL_RADIO_SUB_MENU_ITEM_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj), GAIL_TYPE_RADIO_SUB_MENU_ITEM, GailRadioSubMenuItemClass))

typedef struct _GailRadioSubMenuItem              GailRadioSubMenuItem;
typedef struct _GailRadioSubMenuItemClass         GailRadioSubMenuItemClass;

struct _GailRadioSubMenuItem
{
  GailCheckSubMenuItem parent;

  GSList *old_group;
};

GType gail_radio_sub_menu_item_get_type (void);

struct _GailRadioSubMenuItemClass
{
  GailCheckSubMenuItemClass parent_class;
};

AtkObject* gail_radio_sub_menu_item_new (GtkWidget *widget);

G_END_DECLS

#endif /* __GAIL_RADIO_SUB_MENU_ITEM_H__ */
