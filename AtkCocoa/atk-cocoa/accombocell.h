//
//  accombocell.h
//  AtkCocoa
//
//  Created by iain on 27/09/2019.
//  Copyright © 2019 Microsoft. All rights reserved.
//

#ifndef accombocell_h
#define accombocell_h

#include <atk/atk.h>
#include "gailrenderercell.h"

G_BEGIN_DECLS

#define AC_TYPE_COMBO_CELL            (ac_combo_cell_get_type ())
#define AC_COMBO_CELL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), AC_TYPE_COMBO_CELL, AcComboCell))
#define AC_COMBO_CELL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), AC_COMBO_CELL, AcComboCellClass))
#define AC_IS_COMBO_CELL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AC_TYPE_COMBO_CELL))
#define AC_IS_COMBO_CELL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), AC_TYPE_COMBO_CELL))
#define AC_COMBO_CELL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), AC_TYPE_COMBO_CELL, AcComboCellClass))

typedef struct _AcComboCell                  AcComboCell;
typedef struct _AcComboCellClass             AcComboCellClass;

struct _AcComboCell
{
    GailRendererCell parent;
};

GType ac_combo_cell_get_type (void);

struct _AcComboCellClass
{
    GailRendererCellClass parent_class;
};

AtkObject *ac_combo_cell_new (void);

G_END_DECLS

#endif /* accombocell_h */
