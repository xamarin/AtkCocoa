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
#import "ACAccessibilityElement.h"
#include <acelement.h>
#include <gtk/gtk.h>

@interface ACAccessibilityTreeRowElement : ACAccessibilityElement

@property (readwrite) BOOL rowIsDirty;

- (instancetype)initWithDelegate:(AcElement *)delegate treeRow:(GtkTreeRowReference *)row treeView:(GtkTreeView *)treeView;
- (GtkTreeRowReference *)rowReference;
- (void)addChildRowElement:(ACAccessibilityTreeRowElement *)child;
- (void)removeChildRowElement:(ACAccessibilityTreeRowElement *)child;

// Treat Row element like a tree
- (void)insertChild:(ACAccessibilityTreeRowElement *)child atIndex:(int)idx;
- (void)removeChild:(ACAccessibilityTreeRowElement *)child;
- (void)removeChildAtIndex:(int)idx;
- (void)removeFromParent;
- (void)removeAllChildren;
- (ACAccessibilityTreeRowElement *)childAtIndex:(int)idx;

// path should be  in 0:1:2:3 format
- (ACAccessibilityTreeRowElement *)childAtPath:(const char *)path;

- (void)foreachChild:(void(^)(ACAccessibilityTreeRowElement *parent, ACAccessibilityTreeRowElement *child, void *userData))handler userData:(void *)userdata;
- (int)descendantCount;

- (void)reorderChildrenToNewIndicies:(int *)indicies;
int last_path_index (const char *path);
- (void)dumpChildrenRecursive:(BOOL)recurse;

@end
