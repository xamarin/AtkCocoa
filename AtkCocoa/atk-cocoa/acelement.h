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

#ifndef __AC_ELEMENT_H__
#define __AC_ELEMENT_H__

#include <gtk/gtk.h>

@protocol NSAccessibility;
@class NSArray;
@class NSDictionary;
@class NSString;

G_BEGIN_DECLS

#define AC_TYPE_ELEMENT                     (ac_element_get_type ())
#define AC_ELEMENT(obj)                     (G_TYPE_CHECK_INSTANCE_CAST ((obj), AC_TYPE_ELEMENT, AcElement))
#define AC_ELEMENT_CLASS(klass)             (G_TYPE_CHECK_CLASS_CAST ((klass), AC_TYPE_ELEMENT, AcElementClass))
#define AC_IS_ELEMENT(obj)                  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AC_TYPE_ELEMENT))
#define AC_IS_ELEMENT_CLASS(klass)          (G_TYPE_CHECK_CLASS_TYPE ((klass), AC_TYPE_ELEMENT))
#define AC_ELEMENT_GET_CLASS(obj)           (G_TYPE_INSTANCE_GET_CLASS ((obj), AC_TYPE_ELEMENT, AcElementClass))

typedef struct _AcElement                   AcElement;
typedef struct _AcElementClass              AcElementClass;
typedef struct _AcElementPrivate            AcElementPrivate;

struct _AcElement
{
  GtkAccessible parent;

  AcElementPrivate *priv;
};

GType ac_element_get_type (void);

struct _AcElementClass
{
  GtkAccessibleClass parent_class;

  id<NSAccessibility> (*get_accessibility_element) (AcElement *element);
  void (*child_was_added) (AcElement *element, AcElement *child);
  
  NSArray * (*get_actions) (AcElement *element);

  /* Actions */
  gboolean (*perform_cancel) (AcElement *element);
  gboolean (*perform_confirm) (AcElement *element);
  gboolean (*perform_decrement) (AcElement *element);
  gboolean (*perform_delete) (AcElement *element);
  gboolean (*perform_increment) (AcElement *element);
  gboolean (*perform_pick) (AcElement *element);
  gboolean (*perform_press) (AcElement *element);
  gboolean (*perform_raise) (AcElement *element);
  gboolean (*perform_show_alternate_ui) (AcElement *element);
  gboolean (*perform_show_default_ui) (AcElement *element);
  gboolean (*perform_show_menu) (AcElement *element);
};

void ac_element_set_owner (AcElement *element, GObject *owner);
GObject *ac_element_get_owner (AcElement *element);

void ac_element_add_child (AcElement *parent,
                           AcElement *child);
void ac_element_remove_child (AcElement *parent,
                              AcElement *child);
id<NSAccessibility> ac_element_get_accessibility_element (AcElement *element);
void ac_element_invalidate_accessibility_element (AcElement *element);
const char *ac_element_get_text (AcElement *element);

NSArray *ac_element_get_actions (AcElement *element);

void ac_element_notify (AcElement *element,
		              		  NSString *notificationName,
				                NSDictionary *userInfo);


gboolean ac_element_perform_cancel (AcElement *element);
gboolean ac_element_perform_confirm (AcElement *element);
gboolean ac_element_perform_decrement (AcElement *element);
gboolean ac_element_perform_delete (AcElement *element);
gboolean ac_element_perform_increment (AcElement *element);
gboolean ac_element_perform_pick (AcElement *element);
gboolean ac_element_perform_press (AcElement *element);
gboolean ac_element_perform_raise (AcElement *element);
gboolean ac_element_perform_show_alternate_ui (AcElement *element);
gboolean ac_element_perform_show_default_ui (AcElement *element);
gboolean ac_element_perform_show_menu (AcElement *element);

void ac_element_focus_and_ignore_next (AcElement *element);

G_END_DECLS

#endif /* __AC_ELEMENT_H__ */
