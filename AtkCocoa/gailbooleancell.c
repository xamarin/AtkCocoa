/* GAIL - The GNOME Accessibility Enabling Library
 * Copyright 2001 Sun Microsystems Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <gtk/gtk.h>
#include "atk-cocoa/gailbooleancell.h"
#include "atk-cocoa/acelement.h"

#import <Cocoa/Cocoa.h>
#import "atk-cocoa/ACAccessibiltyBooleanCellElement.h"

static void      gail_boolean_cell_class_init          (GailBooleanCellClass *klass);
static void      gail_boolean_cell_init                (GailBooleanCell *cell);
static void      gail_boolean_cell_initialize (GailCell *cell);

/* Misc */

static gboolean gail_boolean_cell_update_cache         (GailRendererCell     *cell,
                                                        gboolean             emit_change_signal);
static Class gail_boolean_cell_get_element_class (void);

gchar *gail_boolean_cell_property_list[] = {
  "active",
  "radio",
  "sensitive",
  "visible",
  NULL
};

G_DEFINE_TYPE (GailBooleanCell, gail_boolean_cell, GAIL_TYPE_RENDERER_CELL)

static void 
gail_boolean_cell_class_init (GailBooleanCellClass *klass)
{
  GailRendererCellClass *renderer_cell_class = GAIL_RENDERER_CELL_CLASS (klass);
  GailCellClass *cell_class = GAIL_CELL_CLASS (klass);

  renderer_cell_class->update_cache = gail_boolean_cell_update_cache;
  renderer_cell_class->property_list = gail_boolean_cell_property_list;

  cell_class->initialize = gail_boolean_cell_initialize;
  cell_class->get_element_class = gail_boolean_cell_get_element_class;
}

static void
gail_boolean_cell_init (GailBooleanCell *cell)
{
}

static void
gail_boolean_cell_initialize (GailCell *cell)
{
  id<NSAccessibility> realElement = (__bridge id<NSAccessibility>) cell->cell_element;

  [realElement setAccessibilityRole:NSAccessibilityCheckBoxRole];
}

static Class
gail_boolean_cell_get_element_class (void)
{
  return [ACAccessibiltyBooleanCellElement class];
}

AtkObject* 
gail_boolean_cell_new (void)
{
  GObject *object;
  AtkObject *atk_object;
  GailRendererCell *cell;
  GailBooleanCell *boolean_cell;

  object = g_object_new (GAIL_TYPE_BOOLEAN_CELL, NULL);

  g_return_val_if_fail (object != NULL, NULL);

  atk_object = ATK_OBJECT (object);
  atk_object->role = ATK_ROLE_TABLE_CELL;

  cell = GAIL_RENDERER_CELL(object);
  boolean_cell = GAIL_BOOLEAN_CELL(object);

  cell->renderer = gtk_cell_renderer_toggle_new ();
  g_object_ref_sink (cell->renderer);
  boolean_cell->cell_value = FALSE;
  boolean_cell->cell_sensitive = TRUE;
  return atk_object;
}

static gboolean
gail_boolean_cell_update_cache (GailRendererCell *cell, 
                                gboolean         emit_change_signal)
{
  GailBooleanCell *boolean_cell = GAIL_BOOLEAN_CELL (cell);
  GailCell *gail_cell = GAIL_CELL(cell);
  gboolean rv = FALSE;
  gboolean new_boolean;
  gboolean new_sensitive;
  gboolean is_radio;
  gboolean is_visible;

  id<NSAccessibility> realElement = (__bridge id<NSAccessibility>) gail_cell->cell_element;

  g_object_get (G_OBJECT(cell->renderer), "active", &new_boolean,
                                          "sensitive", &new_sensitive,
                                          "radio", &is_radio,
                                          "visible", &is_visible);
  if (is_radio) {
    [realElement setAccessibilityRole:NSAccessibilityRadioButtonRole];
  } else {
    [realElement setAccessibilityRole:NSAccessibilityCheckBoxRole];
  }

  [realElement setAccessibilityHidden:!is_visible];
  
  if (boolean_cell->cell_value != new_boolean)
    {
      // This needs to be fired before the value changes as Cocoa seems to use
      // the current value to work out if it was a tick or an untick action
      id<NSAccessibility> realElement = (__bridge id<NSAccessibility>) gail_cell->cell_element;
//      NSAccessibilityPostNotification(realElement, NSAccessibilityValueChangedNotification);

      rv = TRUE;
      boolean_cell->cell_value = !(boolean_cell->cell_value);

      /* Update cell's state */

      if (new_boolean)
        gail_cell_add_state (GAIL_CELL (cell), ATK_STATE_CHECKED, emit_change_signal);
      else
        gail_cell_remove_state (GAIL_CELL (cell), ATK_STATE_CHECKED, emit_change_signal);
    }

  if (boolean_cell->cell_sensitive != new_sensitive)
    {
      rv = TRUE;
      boolean_cell->cell_sensitive = !(boolean_cell->cell_sensitive);

      /* Update cell's state */

      if (new_sensitive)
        gail_cell_add_state (GAIL_CELL (cell), ATK_STATE_SENSITIVE, emit_change_signal);
      else
        gail_cell_remove_state (GAIL_CELL (cell), ATK_STATE_SENSITIVE, emit_change_signal);
    }

  return rv;
}
