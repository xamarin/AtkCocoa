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

#import <Cocoa/Cocoa.h>
#import "atk-cocoa/ACAccessibilityTreeCellElement.h"

#import "atk-cocoa/ACAccessibilityTreeColumnElement.h"
#import "atk-cocoa/ACAccessibilityTreeRowElement.h"

#include "atk-cocoa/gailtreeview.h"

@protocol ACAccessibilityDisclosureButtonDelegate

- (void)performDisclosurePress;

@end

@interface ACAccessibilityDisclosureButton : NSAccessibilityElement

@property (readwrite, assign) id<ACAccessibilityDisclosureButtonDelegate> delegate;
@end

@implementation ACAccessibilityDisclosureButton

- (NSString *)accessibilityRole
{
    return NSAccessibilityDisclosureTriangleRole;
}

- (BOOL)accessibilityPerformPress
{
    [_delegate performDisclosurePress];
    return YES;
}

@end

// ACAccessibilityTreeCellElement is the child of the TreeRow/TreeColumn that holds all the individual ACAccessibilityCellElements
@interface ACAccessibilityTreeCellElement () <ACAccessibilityDisclosureButtonDelegate>
@end

@implementation ACAccessibilityTreeCellElement {
    __weak ACAccessibilityTreeRowElement *_rowElement;
    __weak ACAccessibilityTreeColumnElement *_columnElement;
    ACAccessibilityDisclosureButton *_disclosureElement;
    NSArray *_children;
    BOOL _updateChildren;
}

- (instancetype)initWithDelegate:(AcElement *)delegate withDisclosureButton:(BOOL)canDisclose
{
    self = [super initWithDelegate:delegate];
    if (!self) {
        return nil;
    }

    [self setAccessibilityRole:NSAccessibilityCellRole];

    if (canDisclose) {
        [self addDisclosureButton];
    }
    return self;
}

- (void)dealloc
{
    [self removeDisclosureButton];
}

- (void)addToRow:(ACAccessibilityTreeRowElement *)rowElement column:(ACAccessibilityTreeColumnElement *)columnElement
{
    _rowElement = rowElement;
    _columnElement = columnElement;
}

- (ACAccessibilityTreeColumnElement *)columnElement
{
    return _columnElement;
}

- (GdkRectangle)frameInGtkWindowSpace
{
    GdkRectangle cellSpace;
    GtkTreeViewColumn *column;
    GtkWidget *treeView;
    GtkTreePath *path;
    int x, y;
    int wx, wy;

    column = [_columnElement column];
    treeView = gtk_tree_view_column_get_tree_view (column);
    path = [_rowElement rowPath];

    if (path == NULL) {
        GdkRectangle rect;
        rect.x = 0;
        rect.y = 0;
        rect.width = 0;
        rect.height = 0;

        return rect;
    }

	gtk_tree_view_get_cell_area (GTK_TREE_VIEW (treeView), path, column, &cellSpace);
	gtk_tree_path_free (path);
	
    // cellSpace coordinates are relative to bin_window, which doesn't include
    // the offset for any headers. Convert to widget coords to add that offset.
    gtk_tree_view_convert_bin_window_to_widget_coords (GTK_TREE_VIEW (treeView), 0, 0, &wx, &wy);

	gtk_widget_translate_coordinates (treeView, gtk_widget_get_toplevel (treeView), 0, 0, &x, &y);

    cellSpace.x += x;
    cellSpace.y += y + wy;

    return cellSpace;
}

- (void)addDisclosureButton
{
    if (_disclosureElement != nil) {
        return;
    }

    _disclosureElement = [[ACAccessibilityDisclosureButton alloc] init];
    [_disclosureElement setDelegate:self];

    _updateChildren = YES;
}

- (void)removeDisclosureButton
{
    if (_disclosureElement == nil) {
        return;
    }
    
    [_disclosureElement setDelegate:nil];
    _disclosureElement = nil;

    _updateChildren = YES;
}

- (NSArray *)accessibilityChildren
{
    GailTreeView *gailview = GAIL_TREE_VIEW([self delegate]);

    if (_children && !_updateChildren) {
        return _children;
    }

    NSMutableArray *children = [NSMutableArray array];
    if (_disclosureElement) {
        [children addObject:_disclosureElement];
    }

    gail_treeview_add_renderer_elements (gailview, _rowElement, _columnElement, children);

    _children = children;
    _updateChildren = NO;
    return children;
}

- (void)performDisclosurePress
{
    GtkTreeViewColumn *column;
    GtkTreePath *path;
    GtkWidget *treeView;

    column = [_columnElement column];
    treeView = gtk_tree_view_column_get_tree_view (column);
    path = [_rowElement rowPath];
    if (path == NULL) {
        return;
    }

    if (gtk_tree_view_row_expanded (GTK_TREE_VIEW (treeView), path)) {
        gtk_tree_view_collapse_row (GTK_TREE_VIEW (treeView), path);
    } else {
        gtk_tree_view_expand_row (GTK_TREE_VIEW (treeView), path, FALSE);
    }

    gtk_tree_path_free (path);
}

// Clear any actions that the accessibility system might try to inherit from the parent TreeView
- (NSArray *)accessibilityActionNames
{
	return nil;
}

- (NSRange)accessibilityRowIndexRange
{
    return NSMakeRange([_rowElement accessibilityIndex], 1);
}

- (NSRange)accessibilityColumnIndexRange
{
    return NSMakeRange([_columnElement accessibilityIndex], 1);
}
@end
