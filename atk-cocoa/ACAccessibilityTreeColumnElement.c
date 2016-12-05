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

#import "ACAccessibilityTreeColumnElement.h"

@implementation ACAccessibilityTreeColumnElement {
    GtkTreeViewColumn *_column;
}

- (instancetype)initWithDelegate:(AcElement *)delegate treeColumn:(GtkTreeViewColumn *)column
{
    self = [super initWithDelegate:delegate];
    if (!self) {
        return nil;
    }

    // FIXME: Should we ref this?
    _column = column;

    [self setAccessibilityRole:NSAccessibilityColumnRole];
    return self;
}

- (NSString *)accessibilityLabel
{
    return nsstring_from_cstring (gtk_tree_view_column_get_title (_column));
}

- (GdkRectangle)frameInGtkWindowSpace
{
    GdkRectangle cellSpace;
    GtkWidget *treeView = gtk_tree_view_column_get_tree_view (_column);

    GtkTreePath *path = gtk_tree_path_new_first ();

    int x, y;

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
@end
