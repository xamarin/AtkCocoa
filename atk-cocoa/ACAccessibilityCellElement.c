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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#import "ACAccessibilityCellElement.h"
#include "gailrenderercell.h"

#include "acdebug.h"
#include "acutils.h"

// ACAccessibilityCellElement is the individual renderers that make up the contents of a "cell"
// FIXME: Rename to ACAccessibilityCellRendererElement?
@implementation ACAccessibilityCellElement {
	GailCell *_delegate;
	GtkTreeRowReference *_row_ref;
	GtkTreeViewColumn *_column;
	int _indexInColumn;
}

- (instancetype)initWithDelegate:(GailCell *)delegate
							 row:(GtkTreeRowReference *)row_ref 
						  column:(GtkTreeViewColumn *)column
						   index:(int)indexInColumn
{
	self = [super init];
	if (!self) {
		return nil;
	}

	_delegate = delegate;
	_row_ref = row_ref;
	_column = column;
	_indexInColumn = indexInColumn;

	return self;
}

- (GailCell *)delegate
{
	return _delegate;
}

- (NSString *)description
{
	GtkTreePath *path = gtk_tree_row_reference_get_path (_row_ref);
	char *pathStr = gtk_tree_path_to_string (path);
	char *description;
	NSString *desc;

	gtk_tree_path_free (path);

	description = g_strdup_printf ("Cell %d for column %s of tree path: %s", _indexInColumn, gtk_tree_view_column_get_title (_column), pathStr);

	desc = nsstring_from_cstring (description);
	g_free (description);

	return desc;
}

- (GdkRectangle)frameInGtkWindowSpace
{
	GdkRectangle cellSpace;
	GtkWidget *treeView = gtk_tree_view_column_get_tree_view (_column);
	GtkTreePath *path = gtk_tree_row_reference_get_path (_row_ref);
	int x, y;

	gtk_tree_view_get_cell_area (GTK_TREE_VIEW (treeView), path, _column, &cellSpace);
	gtk_tree_path_free (path);

	gtk_widget_translate_coordinates (treeView, gtk_widget_get_toplevel (treeView), 0, 0, &x, &y);

    cellSpace.x += x;
    cellSpace.y += y;

	return cellSpace;
}

- (CGRect)accessibilityFrameInParentSpace
{
	GList *renderers;
	GtkCellRenderer *cell_renderer;
	GdkRectangle cellSpace;
	GtkWidget *treeView = gtk_tree_view_column_get_tree_view (_column);
	GtkTreePath *path = gtk_tree_row_reference_get_path (_row_ref);
	int x, width;

	// column_cell_get_position needs the exact renderer from the column, so can't use the one stored in GailCellRenderer 
	renderers = gtk_cell_layout_get_cells (GTK_CELL_LAYOUT (_column));
	cell_renderer = g_list_nth_data (renderers, _indexInColumn);
	g_list_free (renderers);

	gtk_tree_view_get_cell_area (GTK_TREE_VIEW (treeView), path, _column, &cellSpace);
	gtk_tree_path_free (path);

	gtk_tree_view_column_cell_get_position (_column, cell_renderer, &x, &width);

	return CGRectMake (x, cellSpace.y - cellSpace.height, width, cellSpace.height);
}

// Clear any actions that the accessibility system might try to inherit from the parent TreeView
- (NSArray *)accessibilityActionNames
{
	return nil;
}
@end
