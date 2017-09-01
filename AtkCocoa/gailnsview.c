//
//  gailnsview.c
//  AtkCocoa
//
//  Created by iain on 31/08/2017.
//  Copyright © 2017 Microsoft. All rights reserved.
//

#include "config.h"

#import <Cocoa/Cocoa.h>

#include <string.h>
#include <gtk/gtk.h>
#include "atk-cocoa/gail.h"
#include "atk-cocoa/gailnsview.h"
#include "atk-cocoa/acelement.h"

#import "atk-cocoa/ACAccessibilityElement.h"

static void                  gail_nsview_class_init       (GailNsViewClass  *klass);
static void                  gail_nsview_init             (GailNsView       *view);
static void                  gail_nsview_initialize       (AtkObject       *accessible,
                                                          gpointer         data);

G_DEFINE_TYPE (GailNsView, gail_nsview, GAIL_TYPE_CONTAINER)

static void
gail_nsview_class_init (GailNsViewClass *klass)
{
    AtkObjectClass *class = ATK_OBJECT_CLASS (klass);

    class->initialize = gail_nsview_initialize;
}

static void
gail_nsview_init (GailNsView       *view)
{
}

static void
gail_nsview_initialize (AtkObject *accessible,
                       gpointer  data)
{
    GtkNSView *gtkView = GTK_NS_VIEW(data);
    ATK_OBJECT_CLASS (gail_nsview_parent_class)->initialize (accessible, data);

    accessible->role = ATK_ROLE_EMBEDDED;

    NSView *view = (__bridge NSView *)gtk_ns_view_get_nsview(gtkView);
    if (view == nil) {
        g_warning ("No NSView");
        return;
    }

    id<NSAccessibility> element = ac_element_get_accessibility_element(AC_ELEMENT(accessible));
    [element setAccessibilityChildren:@[view]];
}
