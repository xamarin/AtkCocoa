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

#import "atk-cocoa/ACAccessibilityTreeColumnElement.h"
#import "atk-cocoa/ACAccessibilityTreeColumnHeaderElement.h"

#include "atk-cocoa/gailtreeview.h"

@implementation ACAccessibilityTreeColumnElement {
    GtkTreeViewColumn *_column;
    ACAccessibilityTreeColumnHeaderElement *_customHeaderElement;
}

- (instancetype)initWithDelegate:(AcElement *)delegate treeColumn:(GtkTreeViewColumn *)column
{
    self = [super initWithDelegate:delegate];
    if (!self) {
        return nil;
    }

    _column = column;
    if (_column) {
        g_object_add_weak_pointer(G_OBJECT (column), (void **)&_column);
    }

    return self;
}

- (void)dealloc
{
    if (_column) {
        g_object_remove_weak_pointer(G_OBJECT (_column), (void **)&_column);
    }
}

- (NSArray *)accessibilityChildren
{
    GailTreeView *gailview = GAIL_TREE_VIEW([self delegate]);

    NSMutableArray *children = [NSMutableArray array];

    gail_treeview_add_column_elements(gailview, self, children);

    return children;
}

- (NSString *)accessibilityRole
{
    return NSAccessibilityColumnRole;
}

- (NSString *)accessibilityLabel
{
    if (_column == NULL) {
        return nil;
    }
    return nsstring_from_cstring (gtk_tree_view_column_get_title (_column));
}

- (GdkRectangle)frameInGtkWindowSpace
{
    GdkRectangle cellSpace;
    GtkWidget *treeView;
    GtkTreePath *path;
    int x, y;

    treeView = gtk_tree_view_column_get_tree_view (_column);
    path = gtk_tree_path_new_first ();
    gtk_tree_view_get_cell_area (GTK_TREE_VIEW (treeView), path, _column, &cellSpace);
    gtk_tree_path_free (path);

    cellSpace.y = 0;
    cellSpace.height = treeView->allocation.height;

    gtk_widget_translate_coordinates (treeView, gtk_widget_get_toplevel (treeView), 0, 0, &x, &y);

    cellSpace.x += x;
    cellSpace.y += y;

    return cellSpace;
}

- (GtkTreeViewColumn *)column
{
    return _column;
}

// Clear any actions that the accessibility system might try to inherit from the parent TreeView
- (NSArray *)accessibilityActionNames
{
	return nil;
}

- (id<NSAccessibility>)columnHeaderElement
{
    if (_column == NULL) {
        return nil;
    }

    GtkWidget *header = gtk_tree_view_column_get_widget(_column);
    if (header == NULL) {
        if (_customHeaderElement == nil) {
            _customHeaderElement = [[ACAccessibilityTreeColumnHeaderElement alloc] initWithDelegate:[self delegate]
                                                                                         treeColumn:_column];
        }
        return _customHeaderElement;
    }

    return ac_element_get_accessibility_element(AC_ELEMENT (gtk_widget_get_accessible(header)));
}
@end
