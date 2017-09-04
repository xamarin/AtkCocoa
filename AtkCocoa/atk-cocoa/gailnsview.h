//
//  gailnsview.h
//  AtkCocoa
//
//  Created by iain on 31/08/2017.
//  Copyright © 2017 Microsoft. All rights reserved.
//

#ifndef gailnsview_h
#define gailnsview_h

#include "gailcontainer.h"

G_BEGIN_DECLS

#define GAIL_TYPE_NSVIEW                       (gail_nsview_get_type ())
#define GAIL_NSVIEW(obj)                      (G_TYPE_CHECK_INSTANCE_CAST ((obj), GAIL_TYPE_NSVIEW, GailNsView))
#define GAIL_NSVIEW_CLASS(klass)              (G_TYPE_CHECK_CLASS_CAST ((klass), GAIL_TYPE_NSVIEW, GailNsViewClass))
#define GAIL_IS_NSVIEW(obj)                   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GAIL_TYPE_NSVIEW))
#define GAIL_IS_NSVIEW_CLASS(klass)           (G_TYPE_CHECK_CLASS_TYPE ((klass), GAIL_TYPE_NSVIEW))
#define GAIL_NSVIEW_GET_CLASS(obj)            (G_TYPE_INSTANCE_GET_CLASS ((obj), GAIL_TYPE_NSVIEW, GailNsViewClass))

typedef struct _GailNsView              GailNsView;
typedef struct _GailNsViewClass         GailNsViewClass;

struct _GailNsView
{
    GailWidget parent;
};

GType gail_nsview_get_type (void);

struct _GailNsViewClass
{
    GailWidgetClass parent_class;
};

G_END_DECLS


#endif /* gailnsview_h */
