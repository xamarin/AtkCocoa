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

#import "atk-cocoa/ACAccessibilityOutlineElement.h"
#import "atk-cocoa/ACAccessibilityTableHeaderElement.h"
#import "atk-cocoa/ACAccessibilityTreeRowElement.h"
#import "atk-cocoa/ACAccessibilityTreeColumnElement.h"
#import "atk-cocoa/NSAccessibilityElement+AtkCocoa.h"
#include "atk-cocoa/acelement.h"
#include "atk-cocoa/acdebug.h"
#include "atk-cocoa/gailtreeview.h"

@implementation ACAccessibilityOutlineElement {
    ACAccessibilityTableHeaderElement *_headerElement;
}

- (instancetype)initWithDelegate:(AcElement *)delegate
{
	return [super initWithDelegate:delegate];
}

- (BOOL)hasDynamicChildren
{
    return YES;
}

- (NSRect)accessibilityFrame
{
	return [super accessibilityFrame];
}

- (NSString *)accessibilityLabel
{
	return [super accessibilityLabel];
}

- (id<NSAccessibility>)accessibilityParent
{
	return [super accessibilityParent];
}

- (NSArray *)accessibilityChildren
{
    NSArray *children = [super accessibilityChildren];

    return children;
}

- (NSArray *)accessibilityVisibleChildren
{
    return [super accessibilityVisibleChildren];
}

// Implementing accessibilityVisibleCells breaks the outline
/*
- (NSArray *)accessibilityVisibleCells
{
    NSArray *rows = [self accessibilityVisibleRows];
    NSMutableArray *cells = [NSMutableArray array];

    for (id<NSAccessibility> row in rows) {
        [cells addObjectsFromArray:[row accessibilityVisibleCells]];
    }
    return cells;
}
*/

- (id)accessibilityHitTest:(NSPoint) point
{
    GailTreeView *gailview = GAIL_TREE_VIEW([self delegate]);
    GtkTreeView *treeview = GTK_TREE_VIEW(ac_element_get_owner(AC_ELEMENT (gailview)));

    NSWindow *parentWindow = [self accessibilityWindow];
    CGRect screenRect = CGRectMake (point.x, point.y, 1, 1);

    CGRect windowRect = [parentWindow convertRectFromScreen:screenRect];
    CGPoint pointInWindow = CGPointMake (windowRect.origin.x, windowRect.origin.y);

    // Flip the y coords to Gtk origin
    CGPoint pointInGtkWindow;
    float halfWindowHeight = [[parentWindow contentView] frame].size.height / 2;
    int dy = pointInWindow.y - halfWindowHeight;

    pointInGtkWindow = CGPointMake (pointInWindow.x, halfWindowHeight - dy);

    // Convert from window coordinate space to widget.
    int wx, wy;
    gtk_widget_translate_coordinates(gtk_widget_get_toplevel(GTK_WIDGET (treeview)), GTK_WIDGET (treeview),
                                     pointInGtkWindow.x, pointInGtkWindow.y, &wx, &wy);

    // Convert from widget coordinate space to bin_window
    int bx, by;
    gtk_tree_view_convert_widget_to_bin_window_coords(treeview, wx, wy, &bx, &by);

    GtkTreePath *path;
    GtkTreeViewColumn *column;
    int cx, cy;

    if (gtk_tree_view_get_path_at_pos(treeview, bx, by, &path, &column, &cx, &cy)) {
        ACAccessibilityTreeRowElement *rowElement = gail_treeview_row_for_path(gailview, path);
        ACAccessibilityTreeColumnElement *columnElement = gail_treeview_get_column_element(gailview, column);
        NSInteger colIndex = [columnElement accessibilityIndex];
        gtk_tree_path_free (path);

        NSArray *rowChildren = [rowElement accessibilityChildren];
        if (colIndex >= [rowChildren count]) {
            return self;
        }

        return [rowChildren[colIndex] accessibilityHitTest:point];
    }

    // Work out which column the point is in
    return self;
}

- (NSArray *)accessibilityColumns
{
    GailTreeView *gailview = GAIL_TREE_VIEW([self delegate]);
    NSMutableArray *columns = [NSMutableArray array];

    gail_treeview_add_columns(gailview, columns);
    return columns;
}

- (NSArray *)accessibilityColumnHeaderUIElements
{
    NSMutableArray *children = [NSMutableArray array];
    GailTreeView *gailview = GAIL_TREE_VIEW([self delegate]);

    gail_treeview_add_headers(gailview, children);

    return children;
}

- (NSArray *)accessibilityVisibleColumns
{
    return [self accessibilityColumns];
}

- (NSArray *)accessibilityRows
{
    GailTreeView *gailview = GAIL_TREE_VIEW([self delegate]);
    NSMutableArray *children = [NSMutableArray array];

    gail_treeview_add_rows(gailview, children);

    return children;
}

- (NSArray *)accessibilitySelectedChildren
{
    return [self accessibilitySelectedRows];
}

- (NSArray *)accessibilitySelectedRows
{
    GailTreeView *gailview = GAIL_TREE_VIEW([self delegate]);
    // Need to generate the rows before the selection works
    [self accessibilityRows];

    NSMutableArray *rows = [NSMutableArray array];
    gail_treeview_add_selected_rows(gailview, rows);

    return rows;
}

- (NSArray *)accessibilitySelectedColumns
{
    return @[];
}

- (NSArray *)accessibilitySelectedCells
{
    NSArray *rows = [self accessibilitySelectedRows];

    return [rows[0] accessibilityChildren];
}

- (NSArray *)accessibilityVisibleRows
{
    /*
    NSMutableArray *rows = [NSMutableArray array];

    gail_treeview_add_visible_rows(GAIL_TREE_VIEW ([self delegate]), rows);

    return rows;
     */
    return [self accessibilityRows];
}

- (ACAccessibilityTableHeaderElement *)headerElement
{
    return _headerElement;
}

- (void)setHeaderElement:(ACAccessibilityTableHeaderElement *)header
{
    if (_headerElement != nil) {
        [self ac_accessibilityRemoveChildElement:_headerElement];
    }

    _headerElement = header;
    [self accessibilityAddChildElement:_headerElement];
    [self setAccessibilityHeader:header];
}
@end
