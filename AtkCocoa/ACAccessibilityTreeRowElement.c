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

#import "atk-cocoa/ACAccessibilityTreeRowElement.h"

#include <gtk/gtk.h>

@implementation ACAccessibilityTreeRowElement {
    GtkTreeRowReference *_row;
    GtkWidget *_view;

    int _descendantCount; // The total of all children, grandchildren, underneath this node
    GSequence *_children;
    ACAccessibilityTreeRowElement *_parent;
    GSequenceIter *_iterInParent;
    BOOL _rowIsDirty;
}

- (instancetype)initWithDelegate:(AcElement *)delegate treeRow:(GtkTreeRowReference *)row treeView:(GtkTreeView *)treeView
{
    self = [super initWithDelegate:delegate];

    _row = row;
    _view = g_object_ref (G_OBJECT (treeView));
    
    _descendantCount = 0;
    _children = NULL;
    _parent = nil;
    _rowIsDirty = YES;

    [self setAccessibilityRole:NSAccessibilityRowRole];
    [self setAccessibilitySubrole:NSAccessibilityOutlineRowSubrole];

    return self;
}

- (void)dealloc
{
    if (_row) {
        gtk_tree_row_reference_free (_row);
    }

    if (_view) {
        g_object_unref (_view);
    }

    if (_children != NULL) {
        g_sequence_free (_children);
    }
}

- (GtkTreeRowReference *)rowReference
{
    return _row;
}

- (BOOL)rowIsDirty
{
    return _rowIsDirty;
}

- (void)setRowIsDirty:(BOOL)dirty
{
    _rowIsDirty = dirty;
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
    gtk_tree_view_convert_bin_window_to_widget_coords (GTK_TREE_VIEW (_view), 0, cellSpace.y, &wx, &wy);

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

- (NSInteger)accessibilityDisclosureLevel
{
    GtkTreePath *path = gtk_tree_row_reference_get_path (_row);
    int depth = gtk_tree_path_get_depth (path) - 1;

    gtk_tree_path_free (path);

    return depth;
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

#pragma mark - Internal tree structure

- (BOOL)childrenHaveDescendants
{
    if (_children == nil) {
        return NO;
    }

    // If the descendant count is not the same as the number of children
    // then there's some grandchildren out there
    return (_descendantCount != g_sequence_get_length (_children));
}

- (void)insertChild:(ACAccessibilityTreeRowElement *)child atIndex:(int)idx
{
    GSequenceIter *iter;

    //  NSLog (@"InsertChild:atIndex: - %@ - %d", child, idx);
    // g_print ("   Parent: %p\n", self);
    // g_print ("   Child: %p\n", child);
    // if ([self rowReference]) {
        // g_print ("   %s :: %d\n", gtk_tree_path_to_string (gtk_tree_row_reference_get_path ([self rowReference])), idx);
    // } else {
        // g_print ("   Fake Root node\n");
    // }
    // g_print ("   Children count: %d\n", _children ? g_sequence_get_length (_children) + 1 : -1);

    if (_children == NULL) {
        _children = g_sequence_new (NULL);
    }

    iter = g_sequence_get_iter_at_pos (_children, idx);
    child->_iterInParent = g_sequence_insert_before (iter, (__bridge void *) child);

    child->_parent = self;

    [self adjustDescendantCountBy:1];
}

- (void)appendChild:(ACAccessibilityTreeRowElement *)child
{
    if (_children == NULL) {
        _children = g_sequence_new (NULL);
    }

    child->_iterInParent = g_sequence_append (_children, (__bridge void *) child);
    child->_parent = self;

    [self adjustDescendantCountBy:1];
}

- (void)removeChildAtIndex:(int)idx
{
    GSequenceIter *iter;
    ACAccessibilityTreeRowElement *child;

    // NSLog (@"removeChildAtIndex: - %d", idx);
    if (_children == NULL) {
        return;
    }

    iter = g_sequence_get_iter_at_pos (_children, idx);
    if (iter == g_sequence_get_end_iter (_children)) {
        return;
    }

    child = (__bridge ACAccessibilityTreeRowElement *)g_sequence_get (iter);
    child->_parent = nil;

    g_sequence_remove (iter);

    if (g_sequence_get_length (_children) == 0) {
        g_sequence_free (_children);
        _children = NULL;
    }

    [self adjustDescendantCountBy:-1];
}

int
last_path_index (const char *path)
{
    const char *p;
    int idx;

    // g_print ("Last path index: %s", path ? path : "<null>");
    if (path == NULL) {
        return -1;
    }

    p = strrchr (path, ':');
    if (p == NULL) {
        p = path;
    } else {
        p++;
    }

    idx = atoi (p);
    // g_print ("   Converted %s (%s) to %d\n", p, path, idx);
    return idx;
}

- (void)removeChild:(ACAccessibilityTreeRowElement *)child
{
    GtkTreePath *path;
    char *pathString;
    int idx;

    // NSLog (@"RemoveChild: - %@ from %@", child, self);
    if (_children == NULL) {
        return;
    }

    g_sequence_remove (child->_iterInParent);
    child->_iterInParent = NULL;

    if (g_sequence_get_length (_children) == 0) {
        g_sequence_free (_children);
        _children = NULL;
    }

    [self adjustDescendantCountBy:-1];
}

- (void)removeFromParent
{
    // NSLog (@"Remove child from parent: %@ - %@", _parent, self);
    if (_parent == nil) {
        return;
    }

    [_parent removeChild:self];
}

- (void)removeAllChildren
{
    int childCount;

    if (_children == NULL) {
        return;
    }

    childCount = g_sequence_get_length (_children);
    g_sequence_free (_children);
    _children = NULL;

    [self adjustDescendantCountBy:-childCount];
}

#define GET_DATA(iter) ((__bridge ACAccessibilityTreeRowElement *)g_sequence_get ((iter)))
- (ACAccessibilityTreeRowElement *)childAtIndex:(int)idx
{
    GSequenceIter *iter;

    // g_print ("Child at index: %d (%p) - %d\n", idx, self, _children ? g_sequence_get_length (_children) : 0);
    if (_children == NULL) {
        // g_print ("   No children\n");
        return NULL;
    }

    iter = g_sequence_get_iter_at_pos (_children, idx);

    return GET_DATA (iter);
}

- (ACAccessibilityTreeRowElement *)childAtPath:(const char *)path
{
    ACAccessibilityTreeRowElement *child;
    GSequenceIter *iter;
    char *pathCopy, *p;
    BOOL end;
    int idx;

    // g_print ("Child at path: %s (%p)\n", path ? path : "<null>", self);
    if (path == NULL) {
        return nil;
    }

    pathCopy = g_strdup (path);
    p = pathCopy;

    while (*p != ':' && *p != '\0') {
        p++;
    }
    if (*p == '\0') {
        end = YES;
    } else {
        *p = '\0';
        end = NO;
    }

    idx = atoi (pathCopy);
    // g_print ("   Idx: %d (%s) - %s\n", idx, pathCopy, path);
    child = [self childAtIndex:idx];
    if (!end) {
        char *nextPath = p + 1;

        // Recurse down until we reach the end of the path
        // g_print ("   Checking child: %p for %s\n", child, nextPath);
        child = [child childAtPath:nextPath];
    } else {
        // g_print ("At end\n");
    }

    g_free (pathCopy);
    // g_print ("Returning %p\n", child);
    return child;
}

- (void)foreachChild:(void(^)(ACAccessibilityTreeRowElement *parent, ACAccessibilityTreeRowElement *child, void *userData))handler userData:(void *)userdata
{
    GSequenceIter *iter, *endIter;

    if (_children == NULL) {
        return;
    }

    iter = g_sequence_get_begin_iter (_children);
    endIter = g_sequence_get_end_iter (_children);
    while (iter != endIter) {
        ACAccessibilityTreeRowElement *element = GET_DATA (iter);
        handler (self, GET_DATA (iter), userdata);
        iter = g_sequence_iter_next (iter);
    }
}

- (void)adjustDescendantCountBy:(int)count
{
    _descendantCount += count;
    if (_parent != nil) {
        [_parent adjustDescendantCountBy:count];
    }
}

- (int)descendantCount
{
    return _descendantCount;
}

- (int)indexInParent
{
    int idx = 1;
    GSequenceIter *siblingsIter;

    if (_parent == nil) {
        NSLog (@"Not in parent: %@", self);
        return 0;
    }

    // If none of the parent's children have descendants
    // then the index in the parent is simply the last digit of the child's path
    // so we don't need to iterate over all the children
    if (![_parent childrenHaveDescendants]) {
        GtkTreePath *path = gtk_tree_row_reference_get_path ([self rowReference]);
        char *pathString = gtk_tree_path_to_string (path);
        gtk_tree_path_free (path);

        idx = last_path_index (pathString);
        g_free (pathString);

        return idx;
    }

    siblingsIter = _iterInParent;
    while (!g_sequence_iter_is_begin (siblingsIter)) {
        idx++;
        siblingsIter = g_sequence_iter_prev (siblingsIter);
    }

    return idx;
}

- (int)indexFromPath:(const char *)path
{
    char *pathCopy;
    return -1;
}

- (void)reorderChildrenToNewIndicies:(int *)indicies
{
    GSequence *childrenCopy;
    GSequenceIter *orig, *end;

    if (_children == NULL) {
        return;
    }

    childrenCopy = g_sequence_new (NULL);
    for (int i = 0; i < g_sequence_get_length (_children); i++) {
        GSequenceIter *idx = g_sequence_get_iter_at_pos (_children, indicies[i]);

        if (g_sequence_iter_is_end (idx)) {
            return;
        }

        ACAccessibilityTreeRowElement *e = GET_DATA (idx);

        if (e == NULL) {
            continue;
        }
        e->_iterInParent = g_sequence_append (childrenCopy, g_sequence_get (idx));
    }

    g_sequence_free (_children);
    _children = childrenCopy;
}

- (void)dumpChildrenRecursive:(BOOL)recurse
{
    GtkTreePath *path = [self rowReference] ? gtk_tree_row_reference_get_path ([self rowReference]): NULL;
    g_print ("%s (%d)\n", path ? gtk_tree_path_to_string (path) : "<null>", _children ? g_sequence_get_length (_children): 0 );

    if (_children == NULL) {
        return;
    }

    if (!recurse) {
        return;
    }
    for (int i = 0; i < g_sequence_get_length (_children); i++) {
        ACAccessibilityTreeRowElement *e = GET_DATA (g_sequence_get_iter_at_pos (_children, i));
        [e dumpChildrenRecursive:YES];
    }
}

- (void)recursiveFlattenTreeIntoArray:(NSMutableArray *)arr addingSelf:(BOOL)addSelf
{
    if (addSelf) {
        [arr addObject:self];
    }

    if (_children == NULL || g_sequence_get_length (_children) == 0) {
        return;
    }

    GSequenceIter *iter = g_sequence_get_begin_iter (_children);

    while (!g_sequence_iter_is_end (iter)) {
        ACAccessibilityTreeRowElement *r = GET_DATA (iter);

        [r recursiveFlattenTreeIntoArray:arr addingSelf:YES];
        iter = g_sequence_iter_next (iter);
    }
}

- (NSArray *)flattenTree
{
    if (_children == NULL) {
        return nil;
    }

    NSMutableArray *flat = [NSMutableArray array];

    // Don't want to add the fake root node to the tree.
    [self recursiveFlattenTreeIntoArray:flat addingSelf:NO];

    return flat;
}
@end
