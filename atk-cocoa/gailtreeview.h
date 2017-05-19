/* GAIL - The GNOME Accessibility Implementation Library
 * Copyright 2001 Sun Microsystems Inc.
 * Copyright 2016 Microsoft Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GAIL_TREE_VIEW_H__
#define __GAIL_TREE_VIEW_H__

#include <gtk/gtk.h>
#include <atk-cocoa/gailcontainer.h>
#include <atk-cocoa/gailcell.h>

@class NSMutableArray;

G_BEGIN_DECLS

#define GAIL_TYPE_TREE_VIEW                  (gail_tree_view_get_type ())
#define GAIL_TREE_VIEW(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GAIL_TYPE_TREE_VIEW, GailTreeView))
#define GAIL_TREE_VIEW_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), GAIL_TYPE_TREE_VIEW, GailTreeViewClass))
#define GAIL_IS_TREE_VIEW(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GAIL_TYPE_TREE_VIEW))
#define GAIL_IS_TREE_VIEW_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), GAIL_TYPE_TREE_VIEW))
#define GAIL_TREE_VIEW_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), GAIL_TYPE_TREE_VIEW, GailTreeViewClass))

typedef struct _GailTreeView              GailTreeView;
typedef struct _GailTreeViewClass         GailTreeViewClass;

struct _GailTreeView
{
  GailContainer parent;

  GtkTreeModel  *tree_model;
  GtkAdjustment *old_hadj;
  GtkAdjustment *old_vadj;

  GHashTable *columnMap; /* Maps GtkTreeViewColumn to ACAccessibilityTreeColumnElement */
  /* These are void * because ARC doesn't like ObjC object types in C structs */
  void *rowRootNode; /* The root ACAccessibilityTreeRowElement * */

  guint32 rowUpdateId;
};

GType gail_tree_view_get_type (void);

struct _GailTreeViewClass
{
  GailContainerClass parent_class;
};

AtkObject* gail_tree_view_ref_focus_cell (GtkTreeView *treeview);
void gail_tree_view_update_row_cells (GailTreeView *gailview,
                                      GtkTreeView *treeview,
                                      ACAccessibilityTreeRowElement *rowElement);
G_END_DECLS

#endif /* __GAIL_TREE_VIEW_H__ */
