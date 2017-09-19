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
#import "atk-cocoa/ACAccessibilityCellElement.h"
#import "atk-cocoa/ACAccessibilityTreeRowElement.h"
#include "atk-cocoa/gailrenderercell.h"
#include "atk-cocoa/gailtreeview.h"

#include "atk-cocoa/acdebug.h"
#include "atk-cocoa/acutils.h"

// ACAccessibilityCellElement is the individual renderers that make up the contents of a "cell"
// FIXME: Rename to ACAccessibilityCellRendererElement?
@implementation ACAccessibilityCellElement {
	GailCell *_delegate;
	GtkTreeViewColumn *_column;
	int _indexInColumn;
	__weak ACAccessibilityTreeRowElement *_rowElement;
}

- (instancetype)initWithDelegate:(GailCell *)delegate
					  rowElement:(ACAccessibilityTreeRowElement *)rowElement
						  column:(GtkTreeViewColumn *)column
						   index:(int)indexInColumn
{
	self = [super init];
	if (!self) {
		return nil;
	}

	_delegate = delegate;
	_rowElement = rowElement;
	_column = column;
    if (column) {
        g_object_add_weak_pointer(G_OBJECT (column), (void**)&_column);
    }
	_indexInColumn = indexInColumn;

	return self;
}

- (void)dealloc
{
    if (_column) {
        g_object_remove_weak_pointer(G_OBJECT (_column), (void **)&_column);
        _column = NULL;
    }
}
- (GailCell *)delegate
{
	return _delegate;
}

- (NSString *)description
{
    if (_rowElement == nil) {
        return @"";
    }

	GtkTreePath *path = [_rowElement rowPath];
    char *pathStr = path ? gtk_tree_path_to_string (path) : "<no path>";
	char *description;
	NSString *desc;

	gtk_tree_path_free (path);

	description = g_strdup_printf ("Cell %d for column %s of tree path: %s", _indexInColumn, gtk_tree_view_column_get_title (_column), pathStr);

	desc = nsstring_from_cstring (description);
	g_free (description);
	g_free (pathStr);

	return desc;
}

- (GtkTreeViewColumn *)column
{
	return _column;
}

static char *value_property_names[] = {
	"text",
	"active",
	"value",
	NULL
};

- (id)accessibilityValue
{
	if ([_rowElement rowIsDirty]) {
		GtkTreeView *treeview = GTK_TREE_VIEW (_delegate->widget);
		GailTreeView *gailview = GAIL_TREE_VIEW (gtk_widget_get_accessible (_delegate->widget));

		gail_tree_view_update_row_cells (gailview, treeview, _rowElement);
	}

	GailRendererCell *rendererCell = GAIL_RENDERER_CELL (_delegate);
	GtkCellRenderer *renderer = rendererCell->renderer;

	id retObject = nil;

	for (int i = 0; value_property_names[i]; i++) {
		GValue value = G_VALUE_INIT;

		GParamSpec *pspec = g_object_class_find_property (G_OBJECT_GET_CLASS (renderer), value_property_names[i]);
		if (pspec == NULL) {
			continue;
		}

		g_value_init (&value, pspec->value_type);
		g_object_get_property (G_OBJECT (renderer), value_property_names[i], &value);
		if (G_VALUE_HOLDS_STRING (&value)) {
			retObject = nsstring_from_cstring (g_value_get_string (&value));
		} else if (G_VALUE_HOLDS_BOOLEAN (&value)) {
			retObject = @(g_value_get_boolean (&value));
		} else if (G_VALUE_HOLDS_INT (&value)) {
			retObject = @(g_value_get_int (&value));
		} else if (G_VALUE_HOLDS_CHAR (&value)) {
			retObject = @(g_value_get_schar (&value));
		} else if (G_VALUE_HOLDS_UCHAR (&value)) {
			retObject = @(g_value_get_uchar (&value));
		} else if (G_VALUE_HOLDS_UINT (&value)) {
			retObject = @(g_value_get_uint (&value));
		} else if (G_VALUE_HOLDS_INT64 (&value)) {
			retObject = @(g_value_get_int64 (&value));
		} else if (G_VALUE_HOLDS_UINT64 (&value)) {
			retObject = @(g_value_get_uint64 (&value));
		} else if (G_VALUE_HOLDS_LONG (&value)) {
			retObject = @(g_value_get_long (&value));
		} else if (G_VALUE_HOLDS_ULONG (&value)) {
			retObject = @(g_value_get_ulong (&value));
		} else if (G_VALUE_HOLDS_FLOAT (&value)) {
			retObject = @(g_value_get_float (&value));
		} else if (G_VALUE_HOLDS_DOUBLE (&value)) {
			retObject = @(g_value_get_double (&value));
		}

		g_value_unset(&value);
		if (retObject) {
			break;
		}
	}

	return retObject;
}

- (NSString *)accessibilityTitle
{
	return nil;
}

- (NSString *)accessibilityLabel
{
	return nil;
}

- (CGRect)accessibilityFrameInParentSpace
{
	GList *renderers;
	GtkCellRenderer *cell_renderer;
	GdkRectangle cellSpace;
    GtkWidget *treeView;
    GtkTreePath *path;
	int x, width;

    if (_rowElement == nil) {
        return CGRectZero;
    }

    treeView = gtk_tree_view_column_get_tree_view (_column);
    path = [_rowElement rowPath];

    if (path == NULL) {
        return CGRectZero;
    }

	// column_cell_get_position needs the exact renderer from the column, so can't use the one stored in GailCellRenderer 
	renderers = gtk_cell_layout_get_cells (GTK_CELL_LAYOUT (_column));
	cell_renderer = g_list_nth_data (renderers, _indexInColumn);
	g_list_free (renderers);

	gtk_tree_view_get_cell_area (GTK_TREE_VIEW (treeView), path, _column, &cellSpace);
	gtk_tree_path_free (path);

	gtk_tree_view_column_cell_get_position (_column, cell_renderer, &x, &width);

	// Ignore the y coordinate becaue cellSpace is in the binWindow coordinate system,
	// and I don't think you can have cells that don't start at 0.
	return CGRectMake (x, 0, width, cellSpace.height);
}

// Clear any actions that the accessibility system might try to inherit from the parent TreeView
- (NSArray *)accessibilityActionNames
{
	return nil;
}
@end
