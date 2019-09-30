//
//  accombocell.c
//  AtkCocoa
//
//  Created by iain on 27/09/2019.
//  Copyright © 2019 Microsoft. All rights reserved.
//

#include "config.h"

#include <gtk/gtk.h>
#include "atk-cocoa/accombocell.h"
#include "atk-cocoa/acelement.h"

#import <Cocoa/Cocoa.h>

#import "atk-cocoa/ACAccessibilityComboCellElement.h"

static void      ac_combo_cell_class_init          (AcComboCellClass *klass);
static void      ac_combo_cell_init                (AcComboCell *cell);
static void      ac_combo_cell_initialize (GailCell *cell);

/* Misc */

static gboolean ac_combo_cell_update_cache         (GailRendererCell     *cell,
                                                        gboolean             emit_change_signal);
static Class ac_combo_cell_get_element_class (void);

gchar *ac_combo_cell_property_list[] = {
    "active",
    "radio",
    "sensitive",
    NULL
};

G_DEFINE_TYPE (AcComboCell, ac_combo_cell, GAIL_TYPE_RENDERER_CELL)

static void
ac_combo_cell_class_init (AcComboCellClass *klass)
{
    GailRendererCellClass *renderer_cell_class = GAIL_RENDERER_CELL_CLASS (klass);
    GailCellClass *cell_class = GAIL_CELL_CLASS (klass);

    renderer_cell_class->update_cache = ac_combo_cell_update_cache;
    renderer_cell_class->property_list = ac_combo_cell_property_list;

    cell_class->initialize = ac_combo_cell_initialize;
    cell_class->get_element_class = ac_combo_cell_get_element_class;
}

static void
ac_combo_cell_init (AcComboCell *cell)
{
}

static void
ac_combo_cell_initialize (GailCell *cell)
{
    id<NSAccessibility> realElement = (__bridge id<NSAccessibility>) cell->cell_element;
    [realElement setAccessibilityRole:NSAccessibilityMenuButtonRole];
}

static Class
ac_combo_cell_get_element_class (void)
{
    return [ACAccessibilityComboCellElement class];
}

AtkObject*
ac_combo_cell_new (void)
{
    GObject *object;
    AtkObject *atk_object;
    GailRendererCell *cell;
    AcComboCell *combo_cell;

    object = g_object_new (AC_TYPE_COMBO_CELL, NULL);

    g_return_val_if_fail (object != NULL, NULL);

    atk_object = ATK_OBJECT (object);
    atk_object->role = ATK_ROLE_TABLE_CELL;

    cell = GAIL_RENDERER_CELL(object);
    combo_cell = AC_COMBO_CELL(object);

    cell->renderer = gtk_cell_renderer_combo_new ();
    g_object_ref_sink (cell->renderer);

    return atk_object;
}

static gboolean
ac_combo_cell_update_cache (GailRendererCell *cell,
                                gboolean         emit_change_signal)
{
#if 0
    AcComboCell *boolean_cell = ac_combo_cell (cell);
    GailCell *gail_cell = GAIL_CELL(cell);
    gboolean rv = FALSE;
    gboolean new_boolean;
    gboolean new_sensitive;

    g_object_get (G_OBJECT(cell->renderer), "active", &new_boolean,
                  "sensitive", &new_sensitive, NULL);

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
#endif
    return TRUE;
}

