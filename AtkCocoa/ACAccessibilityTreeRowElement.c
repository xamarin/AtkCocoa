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
    BOOL _isRoot;
    GtkWidget *_view;

    int _descendantCount; // The total of all children, grandchildren, underneath this node
    GSequence *_children;
    __weak ACAccessibilityTreeRowElement *_parent;
    GSequenceIter *_iterInParent;
    BOOL _rowIsDirty;
}

- (BOOL)respondsToSelector:(SEL)aSelector
{
    if (aSelector == @selector(isAccessibilityDisclosed) ||
        aSelector == @selector(setAccessibilityDisclosed:)) {
        GtkTreeIter iter;

        GtkTreePath *path = [self rowPath];
        if (path == NULL) {
            return NO;
        }

        GtkTreeModel *model = gtk_tree_row_reference_get_model(_row);
        gtk_tree_model_get_iter(model, &iter, path);
        gtk_tree_path_free(path);

        return gtk_tree_model_iter_has_child(model, &iter);
    }

    return [super respondsToSelector:aSelector];
}

- (instancetype)initWithDelegate:(AcElement *)delegate treeRow:(GtkTreeRowReference *)row treeView:(GtkTreeView *)treeView
{
    self = [super initWithDelegate:delegate];

    _row = row;
    _isRoot = row == NULL && delegate == NULL;

    if (treeView) {
        _view = GTK_WIDGET (treeView);
        g_object_add_weak_pointer(G_OBJECT (treeView), (void **)&_view);
    } else {
        _view = NULL;
    }
    _descendantCount = 0;
    _children = NULL;
    _parent = nil;
    _rowIsDirty = YES;

    return self;
}

- (NSString *)description
{
    char *rowPath;
    if (_row) {
        GtkTreePath *path = gtk_tree_row_reference_get_path(_row);
        if (path) {
            rowPath = gtk_tree_path_to_string(path);
        } else {
            rowPath = g_strdup ("No path");
        }
    } else {
        rowPath = g_strdup ("No row");
    }

    NSString *ret = [NSString stringWithFormat:@"Row %p %s - %s (%p)", self, rowPath,
                     _view ? atk_object_get_name (ATK_OBJECT (gtk_widget_get_accessible(_view))) : "No view", _view];

    g_free (rowPath);
    return ret;
}

- (NSString *)accessibilityRole
{
    return NSAccessibilityRowRole;
}

- (NSString *)accessibilitySubrole
{
    return NSAccessibilityOutlineRowSubrole;
}

- (void)dealloc
{
    if (_row) {
        gtk_tree_row_reference_free (_row);
        _row = NULL;
    }

    if (_view) {
        g_object_remove_weak_pointer(G_OBJECT (_view), (void **)&_view);
        _view = NULL;
    }

    [self removeAllChildren];
    _parent = nil;
}

- (GtkTreeRowReference *)rowReference
{
    return _row;
}

- (GtkTreePath *)rowPath
{
    if (_row == NULL) {
        return NULL;
    }

    return gtk_tree_row_reference_get_path(_row);
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
    GtkTreePath *path = [self rowPath];
    int wx, wy;
    int x, y;

    if (path == NULL) {
        GdkRectangle rect;
        rect.x = 0;
        rect.y = 0;
        rect.width = 0;
        rect.height = 0;

        return rect;
    }

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
    GtkTreePath *path = [self rowPath];
    if (path == NULL) {
        return @"";
    }

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
    GtkTreePath *path = [self rowPath];
    gboolean selected;

    if (path == NULL) {
        return NO;
    }

    selected = gtk_tree_selection_path_is_selected (selection, path);
    gtk_tree_path_free (path);

    return selected; 
}

- (void)setAccessibilitySelected:(BOOL)selected
{
    GtkTreeView *treeView = GTK_TREE_VIEW (ac_element_get_owner (AC_ELEMENT ([self delegate])));
    GtkTreeSelection *selection = gtk_tree_view_get_selection (treeView);
    GtkTreePath *path = [self rowPath];

    if (path == NULL) {
        return;
    }

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
    GtkTreePath *path = [self rowPath];
    gboolean expanded;

    if (path == NULL) {
        return NO;
    }

    expanded = gtk_tree_view_row_expanded (treeView, path);
    gtk_tree_path_free (path);

    return expanded;
}

- (void)setAccessibilityDisclosed:(BOOL)expanded
{
    GtkTreeView *treeView = GTK_TREE_VIEW (ac_element_get_owner (AC_ELEMENT ([self delegate])));
    GtkTreePath *path = [self rowPath];

    if (path == NULL) {
        return;
    }

    if (expanded) {
        gtk_tree_view_expand_row (treeView, path, FALSE);
    } else {
        gtk_tree_view_collapse_row (treeView, path);
    }

    gtk_tree_path_free (path);
}

- (NSInteger)accessibilityDisclosureLevel
{
    GtkTreePath *path = [self rowPath];
    if (path == NULL) {
        return 0;
    }

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

- (ACAccessibilityTreeRowElement *)parent
{
    return _parent;
}

// Clear any actions that the accessibility system might try to inherit from the parent TreeView
- (NSArray *)accessibilityActionNames
{
	return nil;
}

- (NSInteger)accessibilityIndex
{
    if (_parent == nil) {
        // If the parent is nil, then we are at the fake root node of the tree
        // that we don't want to count, so return -1 here, so that it cancels out the +1
        // in the caller function.
        return -1;
    }

    NSInteger parentIndex = [_parent accessibilityIndex];
    return parentIndex + [self indexInParent] + 1;
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

- (void)addVisibleRow:(ACAccessibilityTreeRowElement *)child
{
    NSMutableArray *visibleChildren = [[self accessibilityVisibleRows] mutableCopy];
    if (visibleChildren == nil) {
        visibleChildren = [NSMutableArray array];
    }
    [visibleChildren addObject:child];
    [self setAccessibilityVisibleRows:visibleChildren];
}

static void
remove_child (gpointer data)
{
    ACAccessibilityTreeRowElement *e = (__bridge ACAccessibilityTreeRowElement *)data;
    e->_parent = nil;

    CFBridgingRelease(data);
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
        _children = g_sequence_new(remove_child);
    }

    iter = g_sequence_get_iter_at_pos (_children, idx);
    child->_iterInParent = g_sequence_insert_before (iter, (void *)CFBridgingRetain (child));

    child->_parent = self;

    [self adjustDescendantCountBy:1];
    [self addVisibleRow:child];
}

- (void)appendChild:(ACAccessibilityTreeRowElement *)child
{
    if (_children == NULL) {
        _children = g_sequence_new (remove_child);
    }

    child->_iterInParent = g_sequence_append (_children, (void *)CFBridgingRetain (child));
    child->_parent = self;

    [self adjustDescendantCountBy:1];
    [self addVisibleRow:child];
}

- (void)removeVisibleRow:(ACAccessibilityTreeRowElement *)child
{
    NSMutableArray *visibleChildren = [[self accessibilityVisibleRows] mutableCopy];
    [visibleChildren removeObject:child];
    [self setAccessibilityVisibleRows:visibleChildren];
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
    [self removeVisibleRow:child];
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
    [self removeVisibleRow:child];
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

    // Remove all visible children
    [self setAccessibilityVisibleRows:nil];
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
    int idx = 0;
    GSequenceIter *siblingsIter;

    if (_parent == nil) {
        return 0;
    }

    // If none of the parent's children have descendants
    // then the index in the parent is simply the last digit of the child's path
    // so we don't need to iterate over all the children
    if (![_parent childrenHaveDescendants]) {
        GtkTreePath *path = [self rowPath];
        if (path == NULL) {
            // Row reference is now invalid. Probably during destruction
            return 0;
        }
        char *pathString = gtk_tree_path_to_string (path);
        gtk_tree_path_free (path);

        idx = last_path_index (pathString);
        g_free (pathString);

        return idx;
    }

    siblingsIter = _iterInParent;
    if (g_sequence_iter_is_begin (siblingsIter)) {
        return 0;
    }

    while ((siblingsIter = g_sequence_iter_prev(siblingsIter))) {
        idx++;
        ACAccessibilityTreeRowElement *e = GET_DATA (siblingsIter);
        idx += e->_descendantCount;

        if (g_sequence_iter_is_begin(siblingsIter)) {
            break;
        }
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

    childrenCopy = g_sequence_new (remove_child);
    for (int i = 0; i < g_sequence_get_length (_children); i++) {
        GSequenceIter *idx = g_sequence_get_iter_at_pos (_children, indicies[i]);

        if (g_sequence_iter_is_end (idx)) {
            return;
        }

        ACAccessibilityTreeRowElement *e = GET_DATA (idx);

        if (e == NULL) {
            continue;
        }
        e->_iterInParent = g_sequence_append (childrenCopy, (void *) CFBridgingRetain (e));
    }

    g_sequence_free (_children);
    _children = childrenCopy;
}

- (void)dumpChildrenRecursive:(BOOL)recurse
{
    GtkTreePath *path = [self rowPath];
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
