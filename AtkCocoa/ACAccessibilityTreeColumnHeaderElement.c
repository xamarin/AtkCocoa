/*
 * AtkCocoa
 * Copyright 2017 Microsoft Corporation
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

#import <Cocoa/Cocoa.h>
#import "atk-cocoa/ACAccessibilityTreeColumnHeaderElement.h"
#import "atk-cocoa/acutils.h"

@implementation ACAccessibilityTreeColumnHeaderElement {
    GtkTreeViewColumn *_column;
}

- (instancetype)initWithDelegate:(AcElement *)delegate treeColumn:(GtkTreeViewColumn *)column
{
    self = [super initWithDelegate:delegate];
    if (!self) {
        return nil;
    }

    _column = column;
    if (column) {
        g_object_add_weak_pointer(G_OBJECT (column), (void **)&_column);
    }

    [self setAccessibilityRole:NSAccessibilityButtonRole];

    return self;
}

- (void)dealloc
{
    if (_column) {
        g_object_remove_weak_pointer(G_OBJECT (_column), (void **)&_column);
    }
}

- (GdkRectangle)frameInGtkWindowSpace
{
    GdkRectangle cellSpace;
    GtkWidget *treeView;
    int wx, wy;
    GtkTreePath *path;
    int x, y;

    // Column is defunct
    if (_column == NULL) {
        GdkRectangle rect;

        rect.x = 0;
        rect.y = 0;
        rect.width = 0;
        rect.height = 0;

        return rect;
    }

    treeView = gtk_tree_view_column_get_tree_view (_column);
    path = gtk_tree_path_new_first ();

    gtk_tree_view_get_cell_area (GTK_TREE_VIEW (treeView), path, _column, &cellSpace);
    gtk_tree_path_free (path);

    cellSpace.y = 0;

    /*
    // convert 0,0 to widget coords to find out the offset of the bin_window for the header
    gtk_tree_view_convert_bin_window_to_widget_coords (GTK_TREE_VIEW (treeView), 0, 0, &wx, &wy);
    cellSpace.height = wy;
     */
    GdkWindow *bin_window = gtk_tree_view_get_bin_window(GTK_TREE_VIEW (treeView));
    if (bin_window) {
        gdk_window_get_position(bin_window, &wx, &wy);
        cellSpace.height = wy;
    } else {
        cellSpace.height = 0;
    }

    gtk_widget_translate_coordinates (treeView, gtk_widget_get_toplevel (treeView), 0, 0, &x, &y);

    cellSpace.x += x;
    cellSpace.y += y;
    
    return cellSpace;
}

- (NSString *)accessibilityTitle
{
    const char *title;

    if (_column == NULL) {
        // Defunct
        return nil;
    }

    title = gtk_tree_view_column_get_title(_column);
    return nsstring_from_cstring(title);
}

- (BOOL)accessibilityPerformPress
{
    if (_column == NULL) {
        // Defunct
        return NO;
    }

    if (!gtk_tree_view_column_get_clickable(_column)) {
        return NO;
    }

    gtk_tree_view_column_clicked(_column);
    return YES;
}
@end
