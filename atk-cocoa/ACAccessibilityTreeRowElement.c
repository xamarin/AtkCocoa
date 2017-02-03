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

#import "ACAccessibilityTreeRowElement.h"

#include <gtk/gtk.h>

@implementation ACAccessibilityTreeRowElement {
    GtkTreeRowReference *_row;
    GtkWidget *_view;
}

- (instancetype)initWithDelegate:(AcElement *)delegate treeRow:(GtkTreeRowReference *)row treeView:(GtkTreeView *)treeView
{
    self = [super initWithDelegate:delegate];

    _row = row;
    _view = g_object_ref (G_OBJECT (treeView));
    
    [self setAccessibilityRole:NSAccessibilityRowRole];
    [self setAccessibilitySubrole:NSAccessibilityOutlineRowSubrole];

    return self;
}

- (void)dealloc
{
    gtk_tree_row_reference_free (_row);
    g_object_unref (_view);
}

- (GtkTreeRowReference *)rowReference
{
    return _row;
}

- (GdkRectangle)frameInGtkWindowSpace
{
    GdkRectangle cellSpace;
    GtkTreePath *path = gtk_tree_row_reference_get_path (_row);
    int wx, wy;
    int x, y;

    gtk_tree_view_get_cell_area (GTK_TREE_VIEW (_view), path, NULL, &cellSpace);

    cellSpace.x = 0;
    cellSpace.width = _view->allocation.width;

    // cellSpace coordinates are relative to bin_window, which doesn't include
    // the offset for any headers. Convert to widget coords to add that offset.
    gtk_tree_view_convert_bin_window_to_widget_coords (_view, 0, cellSpace.y, &wx, &wy);

    gtk_widget_translate_coordinates (_view, gtk_widget_get_toplevel (_view), 0, 0, &x, &y);

    cellSpace.x += x;
    cellSpace.y = wy + y;

    return cellSpace;
}

- (NSString *)accessibilityIdentifier
{
    GtkTreePath *path = gtk_tree_row_reference_get_path (_row);
    char *str = gtk_tree_path_to_string (path);
    gtk_tree_path_free (path);

    NSString * nsstr = nsstring_from_cstring (str);
    g_free (str);

    return nsstr;
}

- (BOOL)isAccessibilitySelected
{
    GtkTreeView *treeView = GTK_TREE_VIEW (ac_element_get_owner (AC_ELEMENT ([self delegate])));
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeView);
    GtkTreePath *path = gtk_tree_row_reference_get_path (_row);
    gboolean selected;

    selected = gtk_tree_selection_path_is_selected (selection, path);
    gtk_tree_path_free (path);

    return selected; 
}

- (void)setAccessibilitySelected:(BOOL)selected
{
    GtkTreeView *treeView = GTK_TREE_VIEW (ac_element_get_owner (AC_ELEMENT ([self delegate])));
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeView);
    GtkTreePath *path = gtk_tree_row_reference_get_path (_row);

    char *str = gtk_tree_path_to_string (path);
    g_free (str);

    if (selected) {
        gtk_tree_selection_unselect_all (selection);
        gtk_tree_selection_select_path (selection, path);
    } else {
        gtk_tree_selection_unselect_path (selection, path);
    }

    gtk_tree_path_free (path);
}

- (BOOL)isAccessibilityDisclosed
{
    GtkTreeView *treeView = GTK_TREE_VIEW (ac_element_get_owner (AC_ELEMENT ([self delegate])));
    GtkTreePath *path = gtk_tree_row_reference_get_path (_row);
    gboolean expanded;

    expanded = gtk_tree_view_row_expanded (treeView, path);
    gtk_tree_path_free (path);

    return expanded;
}

- (void)setAccessibilityDisclosed:(BOOL)expanded
{
    GtkTreeView *treeView = GTK_TREE_VIEW (ac_element_get_owner (AC_ELEMENT ([self delegate])));
    GtkTreePath *path = gtk_tree_row_reference_get_path (_row);

    if (expanded) {
        gtk_tree_view_expand_row (treeView, path, FALSE);
    } else {
        gtk_tree_view_collapse_row (treeView, path);
    }

    gtk_tree_path_free (path);
}

- (void)addChildRowElement:(ACAccessibilityTreeRowElement *)child
{
    NSMutableArray *disclosedChildren = [[self accessibilityDisclosedRows] mutableCopy];
    [disclosedChildren addObject:child];
    [self setAccessibilityDisclosedRows:disclosedChildren];

    [child setAccessibilityDisclosedByRow:self];
}

- (void)removeChildRowElement:(ACAccessibilityTreeRowElement *)child
{
    NSMutableArray *disclosedChildren = [[self accessibilityDisclosedRows] mutableCopy];
    [disclosedChildren removeObject:child];
    [self setAccessibilityDisclosedRows:disclosedChildren];

    [child setAccessibilityDisclosedByRow:nil];
}

// Clear any actions that the accessibility system might try to inherit from the parent TreeView
- (NSArray *)accessibilityActionNames
{
	return nil;
}
@end
