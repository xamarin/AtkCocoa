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
#import "ACAccessibilityTreeCellElement.h"

#import "ACAccessibilityTreeColumnElement.h"
#import "ACAccessibilityTreeRowElement.h"

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
    ACAccessibilityTreeRowElement *_rowElement;
    ACAccessibilityTreeColumnElement *_columnElement;
    ACAccessibilityDisclosureButton *_disclosureElement;
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

    [_rowElement release];
    [_columnElement release];

    [super dealloc];
}
- (void)addToRow:(ACAccessibilityTreeRowElement *)rowElement column:(ACAccessibilityTreeColumnElement *)columnElement
{
    _rowElement = [rowElement retain];
    _columnElement = [columnElement retain];

    [columnElement accessibilityAddChildElement:self];
    [rowElement accessibilityAddChildElement:self];
}

- (GdkRectangle)frameInGtkWindowSpace
{
    GdkRectangle cellSpace;
    GtkTreeViewColumn *column;
    GtkWidget *treeView;
    GtkTreePath *path;
    int x, y;

    column = [_columnElement column];
    treeView = gtk_tree_view_column_get_tree_view (column);
    path = gtk_tree_row_reference_get_path ([_rowElement rowReference]);

	gtk_tree_view_get_cell_area (GTK_TREE_VIEW (treeView), path, column, &cellSpace);
	gtk_tree_path_free (path);
	
	gtk_widget_translate_coordinates (treeView, gtk_widget_get_toplevel (treeView), 0, 0, &x, &y);

    cellSpace.x += x;
    cellSpace.y += y;

    g_print ("Getting cellspace for cell\n");

    return cellSpace;
}

- (void)addDisclosureButton
{
    if (_disclosureElement != nil) {
        return;
    }

    _disclosureElement = [[ACAccessibilityDisclosureButton alloc] init];
    [_disclosureElement setDelegate:self];

    [self accessibilityAddChildElement:_disclosureElement];
}

- (void)removeDisclosureButton
{
    if (_disclosureElement == nil) {
        return;
    }
    
    [_disclosureElement setDelegate:nil];
    [_disclosureElement release];
    _disclosureElement = nil;
}

- (void)performDisclosurePress
{
    GtkTreeViewColumn *column;
    GtkTreePath *path;
    GtkWidget *treeView;

    column = [_columnElement column];
    treeView = gtk_tree_view_column_get_tree_view (column);
    path = gtk_tree_row_reference_get_path ([_rowElement rowReference]);

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
@end
