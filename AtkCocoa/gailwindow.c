/* GAIL - The GNOME Accessibility Implementation Library
 * Copyright 2001, 2002, 2003 Sun Microsystems Inc.
 * Copyright 2016 Microsoft Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <string.h>

#undef GTK_DISABLE_DEPRECATED

#include <gtk/gtk.h>
#include <gdk/gdkquartz.h>

#import <objc/runtime.h>
#import <Cocoa/Cocoa.h>

#include "atk-cocoa/gailwindow.h"
#include "atk-cocoa/gailtoplevel.h"
#include "atk-cocoa/gail-private-macros.h"

#include "atk-cocoa/acdebug.h"

#import "atk-cocoa/ACAccessibilityElement.h"

@implementation NSApplication (AtkCocoa)

+ (void)load
{
  static dispatch_once_t onceToken;
  dispatch_once (&onceToken, ^{
    Class klass = [self class];

    SEL originalSelector = @selector (accessibilitySetValue:forAttribute:);
    SEL swizzledSelector = @selector (atkcocoa_accessibilitySetValue:forAttribute:);

    Method originalMethod = class_getInstanceMethod (klass, originalSelector);
    Method swizzledMethod = class_getInstanceMethod (klass, swizzledSelector);

    BOOL success = class_addMethod (klass, originalSelector, method_getImplementation (swizzledMethod), method_getTypeEncoding (swizzledMethod));
    if (success) {
      class_replaceMethod (klass, swizzledSelector, method_getImplementation (originalMethod), method_getTypeEncoding (originalMethod));
    } else {
      method_exchangeImplementations (originalMethod, swizzledMethod);
    }
  });
}

- (void)atkcocoa_accessibilitySetValue:(id)value
                          forAttribute:(NSAccessibilityAttributeName)attribute
{
  [self atkcocoa_accessibilitySetValue:value forAttribute:attribute];

  if ([attribute isEqualToString:@"AXEnhancedUserInterface"]) {
    NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];

    NSString *name = [value boolValue] ? @"AtkCocoaAccessibilityEnabled" : @"AtkCocoaAccessibilityDisabled";
    [nc postNotificationName:name object:self];
  }
}

@end

/* Swizzle an accessibilityHitTest: into NSWindow that understands our accessibility element */

@implementation NSWindow (AtkCocoa)

+ (void)load
{
  static dispatch_once_t onceToken;
  dispatch_once (&onceToken, ^{
    Class klass = [self class];

    SEL originalSelector = @selector (accessibilityHitTest:);
    SEL swizzledSelector = @selector (atkcocoa_accessibilityHitTest:);

    Method originalMethod = class_getInstanceMethod (klass, originalSelector);
    Method swizzledMethod = class_getInstanceMethod (klass, swizzledSelector);

    BOOL success = class_addMethod (klass, originalSelector, method_getImplementation (swizzledMethod), method_getTypeEncoding (swizzledMethod));
    if (success) {
      class_replaceMethod (klass, swizzledSelector, method_getImplementation (originalMethod), method_getTypeEncoding (originalMethod));
    } else {
      method_exchangeImplementations (originalMethod, swizzledMethod);
    }
  });
}

// Because convertScreenToBase: is now deprecated
- (CGPoint)atkcocoa_convertPointFromScreen:(CGPoint)point
{
  CGRect rect = CGRectMake (point.x, point.y, 1, 1);
  CGRect windowRect = [self convertRectFromScreen:rect];

  return CGPointMake (windowRect.origin.x, windowRect.origin.y);
}

- (id)atkcocoa_accessibilityHitTest:(NSPoint)point
{
  id retval = [self atkcocoa_accessibilityHitTest:point];

  if (retval != self) {
    return retval;
  }

  CGPoint pointInWindow = [self atkcocoa_convertPointFromScreen:point];
  if ([[self contentView] hitTest:pointInWindow]) {
    NSArray *children = [self accessibilityChildren];
    AC_NOTE (HITTEST, NSLog (@"Number of accessibility children on window: %@: %ld", self, children ? [children count] : -2));
    AC_NOTE (HITTEST, NSLog (@"   Point: %f,%f", point.x, point.y));

    for (id<NSAccessibility> child in children) {
      if ([child isKindOfClass:[ACAccessibilityElement class]]) {
        NSAccessibilityElement *e = (NSAccessibilityElement *)child;
        CGRect frameInParent = [e accessibilityFrameInParentSpace];
        if ([[self contentView] isFlipped]) {
          float halfParentHeight = [[self contentView] frame].size.height / 2;
          float dy = (frameInParent.origin.y + frameInParent.size.height) - halfParentHeight;
          frameInParent.origin.y = halfParentHeight - dy;
        }
        CGRect frameInWindow = [[self contentView] convertRect:frameInParent toView:nil];
        CGRect frameInScreen = [self convertRectToScreen:frameInWindow];

        AC_NOTE (HITTEST, NSLog (@"   %@ - %@", child, [[self contentView] isFlipped] ? @"Flipped" : @"Normal"));

        AC_NOTE (HITTEST, NSLog (@"   frame rect: %@", NSStringFromRect (frameInParent)));
        AC_NOTE (HITTEST, NSLog (@"   window rect: %@", NSStringFromRect (frameInWindow)));
        AC_NOTE (HITTEST, NSLog (@"   screen rect: %@", NSStringFromRect (frameInScreen)));

        if (CGRectContainsPoint (frameInScreen, point)) {
          AC_NOTE (HITTEST, NSLog (@"   Hit %@", child));
          return [e accessibilityHitTest:point];
        }
      } else {
        // Skip this because any non ACAccessibilityElement objects shouldn't be
        // inside the contentView
        AC_NOTE (HITTEST, NSLog (@"   Skipping %@", child));
      }
    }
  }

  return self;
}
@end

enum {
  ACTIVATE,
  CREATE,
  DEACTIVATE,
  DESTROY,
  MAXIMIZE,
  MINIMIZE,
  MOVE,
  RESIZE,
  RESTORE,
  LAST_SIGNAL
};

struct _GailWindowPrivate {
  void *prerealized_element; /* ACAccessibilityElement */
  int audit_id;
};

#define GAIL_WINDOW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GAIL_TYPE_WINDOW, GailWindowPrivate))

static void gail_window_class_init (GailWindowClass *klass);

static void                  gail_window_init            (GailWindow   *accessible);

static void                  gail_window_real_initialize (AtkObject    *obj,
                                                          gpointer     data);
static id<NSAccessibility>   gail_window_real_get_accessibility_element (AcElement *element);
static void                  gail_window_finalize        (GObject      *object);

static const gchar*          gail_window_get_name        (AtkObject     *accessible);

static AtkObject*            gail_window_get_parent     (AtkObject     *accessible);
static gint                  gail_window_get_index_in_parent (AtkObject *accessible);
static gboolean              gail_window_real_focus_gtk (GtkWidget     *widget,
                                                         GdkEventFocus *event);

static AtkStateSet*          gail_window_ref_state_set  (AtkObject     *accessible);
static AtkRelationSet*       gail_window_ref_relation_set  (AtkObject     *accessible);
static void                  gail_window_real_notify_gtk (GObject      *obj,
                                                          GParamSpec   *pspec);
static gint                  gail_window_get_mdi_zorder (AtkComponent  *component);

static gboolean              gail_window_state_event_gtk (GtkWidget           *widget,
                                                          GdkEventWindowState *event);

/* atkcomponent.h */
static void                  atk_component_interface_init (AtkComponentIface    *iface);

static void                  gail_window_get_extents      (AtkComponent         *component,
                                                           gint                 *x,
                                                           gint                 *y,
                                                           gint                 *width,
                                                           gint                 *height,
                                                           AtkCoordType         coord_type);
static void                  gail_window_get_size         (AtkComponent         *component,
                                                           gint                 *width,
                                                           gint                 *height);
static void gail_window_realized (GtkWidget *window,
                                  gpointer data);

static guint gail_window_signals [LAST_SIGNAL] = { 0, };

G_DEFINE_TYPE_WITH_CODE (GailWindow, gail_window, GAIL_TYPE_CONTAINER,
                         G_IMPLEMENT_INTERFACE (ATK_TYPE_COMPONENT, atk_component_interface_init))


static void
gail_window_class_init (GailWindowClass *klass)
{
  GailWidgetClass *widget_class;
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  AtkObjectClass  *class = ATK_OBJECT_CLASS (klass);
  AcElementClass *element_class = AC_ELEMENT_CLASS (klass);
  GtkAccessibleClass *accessible_class = GTK_ACCESSIBLE_CLASS(klass);

  gobject_class->finalize = gail_window_finalize;

  widget_class = (GailWidgetClass*)klass;
  widget_class->focus_gtk = gail_window_real_focus_gtk;
  widget_class->notify_gtk = gail_window_real_notify_gtk;

  class->get_name = gail_window_get_name;
  class->get_parent = gail_window_get_parent;
  class->get_index_in_parent = gail_window_get_index_in_parent;
  class->ref_relation_set = gail_window_ref_relation_set;
  class->ref_state_set = gail_window_ref_state_set;
  class->initialize = gail_window_real_initialize;

  // FIXME: If this isn't set, then destroying a window will crash when accessible->widget is set to NULL
  // Need to track down where accessible->widget is then being accessed.
  accessible_class->connect_widget_destroyed = NULL;

  element_class->get_accessibility_element = gail_window_real_get_accessibility_element;

  gail_window_signals [ACTIVATE] =
    g_signal_new ("activate",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [CREATE] =
    g_signal_new ("create",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [DEACTIVATE] =
    g_signal_new ("deactivate",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [DESTROY] =
    g_signal_new ("destroy",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [MAXIMIZE] =
    g_signal_new ("maximize",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [MINIMIZE] =
    g_signal_new ("minimize",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [MOVE] =
    g_signal_new ("move",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [RESIZE] =
    g_signal_new ("resize",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
  gail_window_signals [RESTORE] =
    g_signal_new ("restore",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, /* default signal handler */
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

    g_type_class_add_private (gobject_class, sizeof (GailWindowPrivate));
}

static void
gail_window_init (GailWindow   *accessible)
{
  accessible->priv = GAIL_WINDOW_GET_PRIVATE (accessible);
}

static void
audit_element (id<NSAccessibility> element, int depth, int *invalidCount)
{
  NSArray *children = nil;

  if ([element respondsToSelector:@selector (accessibilityChildren)]) {
    children = [element accessibilityChildren];
  }

  if ([element isKindOfClass:[ACAccessibilityElement class]]) {
    ACAccessibilityElement *realElement = (ACAccessibilityElement *)element;
    AcElement *e = [realElement delegate];

    g_print ("%*s--- %p - %p\n", depth, " ", e, realElement);
    g_print ("%*s   Valid: %d\n", depth, " ", AC_IS_ELEMENT (e));
    if (!AC_IS_ELEMENT (e)) {
      g_print ("***********************************\n");
      (*invalidCount)++;
    }
    g_print ("%*s   Type: %s\n", depth, " ", G_OBJECT_TYPE_NAME (e));
  } else {
    g_print ("%*s--- %p\n", depth, " ", element);
    g_print ("%*s   Type: %s\n", depth, " ", [[element debugDescription] UTF8String]);
  }

  g_print ("%*s   --- %lu children\n", depth, " ", [children count]);

  for (id<NSAccessibility> c in children) {
    audit_element (c, depth + 1, invalidCount);
  }

  g_print ("%*s--- end %p\n", depth, " ", element);
}

static gboolean
audit_accessibility_tree (gpointer data)
{
  AcElement *window = AC_ELEMENT (data);
  int invalidCount = 0;
  int depth = 0;

  g_print ("--- --- --- BEGIN AUDIT --- --- ---\n");
  if (AC_IS_ELEMENT (window)) {
    audit_element (ac_element_get_accessibility_element (window), depth, &invalidCount);
  } else {
    invalidCount = 1;
  }
  g_print ("--- --- --- END AUDIT --- --- ---\n");
  g_print ("%d invalid elements\n", invalidCount);
  return TRUE;
}

static void
gail_window_real_initialize (AtkObject *obj,
                             gpointer  data)
{
  GtkWidget *widget = GTK_WIDGET (data);
  GailWindow *window;

  /*
   * A GailWindow can be created for a GtkHandleBox or a GtkWindow
   */
  if (!GTK_IS_WINDOW (widget) &&
      !GTK_IS_HANDLE_BOX (widget))
    gail_return_if_fail (FALSE);

  ATK_OBJECT_CLASS (gail_window_parent_class)->initialize (obj, data);

  window = GAIL_WINDOW (obj);
  window->name_change_handler = 0;
  window->previous_name = g_strdup (gtk_window_get_title (GTK_WINDOW (data)));

  g_signal_connect (data,
                    "window_state_event",
                    G_CALLBACK (gail_window_state_event_gtk),
                    NULL);
  g_signal_connect (data,
                    "realize",
                    G_CALLBACK (gail_window_realized),
                    obj);
  g_object_set_data (G_OBJECT (obj), "atk-component-layer",
                     GINT_TO_POINTER (ATK_LAYER_WINDOW));

  if (GTK_IS_FILE_SELECTION (widget))
    obj->role = ATK_ROLE_FILE_CHOOSER;
  else if (GTK_IS_COLOR_SELECTION_DIALOG (widget))
    obj->role = ATK_ROLE_COLOR_CHOOSER;
  else if (GTK_IS_FONT_SELECTION_DIALOG (widget))
    obj->role = ATK_ROLE_FONT_CHOOSER;
  else if (GTK_IS_MESSAGE_DIALOG (widget))
    obj->role = ATK_ROLE_ALERT;
  else if (GTK_IS_DIALOG (widget))
    obj->role = ATK_ROLE_DIALOG;
  else
    {
      const gchar *name;

      name = gtk_widget_get_name (widget);
      if (name && (!strcmp (name, "gtk-tooltip") ||
                   !strcmp (name, "gtk-tooltips")))
        obj->role = ATK_ROLE_TOOL_TIP;
      else if (GTK_IS_PLUG (widget))
        obj->role = ATK_ROLE_PANEL;
      else if (GTK_WINDOW (widget)->type == GTK_WINDOW_POPUP)
        obj->role = ATK_ROLE_WINDOW;
      else
        obj->role = ATK_ROLE_FRAME;
    }

  /*
   * Notify that tooltip is showing
   */
  if (obj->role == ATK_ROLE_TOOL_TIP &&
      gtk_widget_get_mapped (widget))
    atk_object_notify_state_change (obj, ATK_STATE_SHOWING, 1);

  // window->priv->audit_id = g_timeout_add (10000, audit_accessibility_tree, window);
  window->priv->audit_id = 0;
}

static void
update_toplevel_and_window (NSArray *children, NSWindow *window)
{
  int i;

  if (children == nil || [children count] == 0) {
    return;
  }

  for (i = 0; i < [children count]; i++) {
    id childObject = [children objectAtIndex:i];

    // Some of the window decoration inside the accessibilityChildren array don't respond
    // to accessibility messages.
    if (![childObject respondsToSelector:@selector (setAccessibilityTopLevelUIElement:)]) {
      continue;
    }
    id<NSAccessibility> child = (id<NSAccessibility>)childObject;
    [child setAccessibilityTopLevelUIElement:window];
    [child setAccessibilityWindow:window];

    update_toplevel_and_window ([child accessibilityChildren], window);
  }
}

static void
gail_window_realized (GtkWidget *window,
                      gpointer data)
{
  GailWindow *gw = GAIL_WINDOW (data);
  GailWindowPrivate *priv = gw->priv;

  NSWindow *ns_window = gdk_quartz_window_get_nswindow (gtk_widget_get_window (window));
  NSArray *original_children;
  NSArray *prerealized_children;
  NSMutableArray *new_children;
  int i;

	// Set the NSAccessibilityElement as data on the AtkObject
	// so it can be accessed from managed code which can't know about AcElement
	g_object_set_data (G_OBJECT (data), "xamarin-private-atkcocoa-nsaccessibility", (__bridge void *) ns_window);

  AC_NOTE (WIDGETS, g_print ("ATKCocoa: Realizing window\n"));
  if (priv->prerealized_element == NULL) {
    return;
  }

  id<NSAccessibility> prerealizedElement = (__bridge id<NSAccessibility>)priv->prerealized_element;
  prerealized_children = [prerealizedElement accessibilityChildren];

  if ([prerealized_children count] == 0) {
    CFBridgingRelease (priv->prerealized_element);
    priv->prerealized_element = NULL;

    ac_element_invalidate_accessibility_element (AC_ELEMENT (data));
    return;
  }

  original_children = [ns_window accessibilityChildren];
  new_children = [original_children mutableCopy];

  AC_NOTE (WIDGETS, g_print ("ATKCocoa:    prerealized_children: %lu\n", [prerealized_children count]));
  AC_NOTE (WIDGETS, g_print ("ATKCocoa:    unignored children: %lu\n", [NSAccessibilityUnignoredChildren (prerealized_children) count]));

  for (i = 0; i < [prerealized_children count]; i++) {
    id<NSAccessibility> child = (id<NSAccessibility>)[prerealized_children objectAtIndex:i];
    NSView *contentView = [ns_window contentView];
    [child setAccessibilityParent:contentView];
    [child setAccessibilityWindow:ns_window];
  }

  [new_children addObjectsFromArray:prerealized_children];

  update_toplevel_and_window (new_children, ns_window);

  AC_NOTE (WIDGETS, g_print ("ATKCocoa:    Adding %lu children to window\n", [new_children count]));
  [ns_window setAccessibilityChildren:new_children];
  AC_NOTE (WIDGETS, g_print ("ATKCocoa:    new children: %lu\n", [[ns_window accessibilityChildren] count]));

  CFBridgingRelease (priv->prerealized_element);
  priv->prerealized_element = NULL;
  ac_element_invalidate_accessibility_element (AC_ELEMENT (data));
}

static id<NSAccessibility>
gail_window_real_get_accessibility_element (AcElement *element)
{
  GailWindowPrivate *priv;
  GtkAccessible *accessible;
  GtkWidget *window;
  NSWindow *ns_window;

  accessible = GTK_ACCESSIBLE (element);

  priv = GAIL_WINDOW_GET_PRIVATE (element);

  window = gtk_accessible_get_widget (accessible);

  // Element is defunct
  if (window == NULL) {
    return nil;
  }

  AC_NOTE (WIDGETS, g_print ("Window %s is realised: %s\n", G_OBJECT_TYPE_NAME (ac_element_get_owner (element)), gtk_widget_get_realized (window) ? "yes" : "no"));
  AC_NOTE (WIDGETS, g_print ("GdkWindow: %p\n", gtk_widget_get_window (window)));

  if (gtk_widget_get_realized (window)) {
    ns_window = gdk_quartz_window_get_nswindow (gtk_widget_get_window (window));
    return (id<NSAccessibility>)ns_window;
  } else {
    if (priv->prerealized_element) {
      return (__bridge id<NSAccessibility>)priv->prerealized_element;
    }

    priv->prerealized_element = (__bridge_retained void *) [[ACAccessibilityElement alloc] initWithDelegate:element];
    id<NSAccessibility> prerealizedElement = (__bridge id<NSAccessibility>)priv->prerealized_element;

    [prerealizedElement setAccessibilityRole:@"AXWindow"];

    return prerealizedElement;
  }
}

static void
gail_window_finalize (GObject *object)
{
  GailWindow* window = GAIL_WINDOW (object);

  if (window->priv->prerealized_element) {
    CFBridgingRelease (window->priv->prerealized_element);
    window->priv->prerealized_element = NULL;
  }

  if (window->priv->audit_id > 0) {
    g_source_remove (window->priv->audit_id);
    window->priv->audit_id = 0;
  }

  if (window->name_change_handler)
    {
      g_source_remove (window->name_change_handler);
      window->name_change_handler = 0;
    }
  if (window->previous_name)
    {
      g_free (window->previous_name);
      window->previous_name = NULL;
    }

  G_OBJECT_CLASS (gail_window_parent_class)->finalize (object);
}

static const gchar*
gail_window_get_name (AtkObject *accessible)
{
  const gchar* name;

  name = ATK_OBJECT_CLASS (gail_window_parent_class)->get_name (accessible);
  if (name == NULL)
    {
      /*
       * Get the window title if it exists
       */
      GtkWidget* widget = GTK_ACCESSIBLE (accessible)->widget; 

      if (widget == NULL)
        /*
         * State is defunct
         */
        return NULL;

      gail_return_val_if_fail (GTK_IS_WIDGET (widget), NULL);

      if (GTK_IS_WINDOW (widget))
        {
          GtkWindow *window = GTK_WINDOW (widget);
 
          name = gtk_window_get_title (window);
          if (name == NULL &&
              accessible->role == ATK_ROLE_TOOL_TIP)
            {
              GtkWidget *child;

              child = gtk_bin_get_child (GTK_BIN (window));
              /* could be some kind of egg notification bubble thingy? */

              /* Handle new GTK+ GNOME 2.20 tooltips */
              if (GTK_IS_ALIGNMENT(child))
                {
                  child = gtk_bin_get_child (GTK_BIN (child));
                  if (GTK_IS_BOX(child)) 
                    {
                      GList *children;
                      guint count;
                      children = gtk_container_get_children (GTK_CONTAINER (child));
                      count = g_list_length (children);
                      if (count == 2) 
                        {
                          child = (GtkWidget *) g_list_nth_data (children, 1);
                        }
                      g_list_free (children);                
                    }
                }

              if (!GTK_IS_LABEL (child)) 
              { 
                  g_message ("ATK_ROLE_TOOLTIP object found, but doesn't look like a tooltip.");
                  return NULL;
              }
              name = gtk_label_get_text (GTK_LABEL (child));
            }
        }
    }
  return name;
}

static AtkObject*
gail_window_get_parent (AtkObject *accessible)
{
  AtkObject* parent;

  parent = ATK_OBJECT_CLASS (gail_window_parent_class)->get_parent (accessible);

  return parent;
}

static gint
gail_window_get_index_in_parent (AtkObject *accessible)
{
  GtkWidget* widget = GTK_ACCESSIBLE (accessible)->widget; 
  AtkObject* atk_obj = atk_get_root ();
  gint index = -1;

  if (widget == NULL)
    /*
     * State is defunct
     */
    return -1;

  gail_return_val_if_fail (GTK_IS_WIDGET (widget), -1);

  index = ATK_OBJECT_CLASS (gail_window_parent_class)->get_index_in_parent (accessible);
  if (index != -1)
    return index;

  if (GTK_IS_WINDOW (widget))
    {
      GtkWindow *window = GTK_WINDOW (widget);
      if (GAIL_IS_TOPLEVEL (atk_obj))
        {
	  GailToplevel* toplevel = GAIL_TOPLEVEL (atk_obj);
	  index = g_list_index (toplevel->window_list, window);
	}
      else
        {
	  int i, sibling_count = atk_object_get_n_accessible_children (atk_obj);
	  for (i = 0; i < sibling_count && index == -1; ++i)
	    {
	      AtkObject *child = atk_object_ref_accessible_child (atk_obj, i);
	      if (accessible == child) index = i;
	      g_object_unref (G_OBJECT (child));
	    }
	}
    }
  return index;
}

static gboolean
gail_window_real_focus_gtk (GtkWidget     *widget,
                            GdkEventFocus *event)
{
  AtkObject* obj;

  obj = gtk_widget_get_accessible (widget);
  atk_object_notify_state_change (obj, ATK_STATE_ACTIVE, event->in);

  return FALSE;
}

static AtkRelationSet*
gail_window_ref_relation_set (AtkObject *obj)
{
  GtkWidget *widget;
  AtkRelationSet *relation_set;
  AtkObject *array[1];
  AtkRelation* relation;
  GtkWidget *current_widget;

  gail_return_val_if_fail (GAIL_IS_WIDGET (obj), NULL);

  widget = GTK_ACCESSIBLE (obj)->widget;
  if (widget == NULL)
    /*
     * State is defunct
     */
    return NULL;

  relation_set = ATK_OBJECT_CLASS (gail_window_parent_class)->ref_relation_set (obj);

  if (atk_object_get_role (obj) == ATK_ROLE_TOOL_TIP)
    {
      relation = atk_relation_set_get_relation_by_type (relation_set, ATK_RELATION_POPUP_FOR);

      if (relation)
        {
          atk_relation_set_remove (relation_set, relation);
        }
      if (gtk_widget_get_visible(widget) && gtk_tooltips_get_info_from_tip_window (GTK_WINDOW (widget), NULL, &current_widget))
        {
          array [0] = gtk_widget_get_accessible (current_widget);

          relation = atk_relation_new (array, 1, ATK_RELATION_POPUP_FOR);
          atk_relation_set_add (relation_set, relation);
          g_object_unref (relation);
        }
    }
  return relation_set;
}

static AtkStateSet*
gail_window_ref_state_set (AtkObject *accessible)
{
  AtkStateSet *state_set;
  GtkWidget *widget;
  GtkWindow *window;
  GdkWindowState state;

  state_set = ATK_OBJECT_CLASS (gail_window_parent_class)->ref_state_set (accessible);
  widget = GTK_ACCESSIBLE (accessible)->widget;
 
  if (widget == NULL)
    return state_set;

  window = GTK_WINDOW (widget);

  if (window->has_focus)
    atk_state_set_add_state (state_set, ATK_STATE_ACTIVE);

  if (widget->window)
    {
      state = gdk_window_get_state (widget->window);
      if (state & GDK_WINDOW_STATE_ICONIFIED)
        atk_state_set_add_state (state_set, ATK_STATE_ICONIFIED);
    } 
  if (gtk_window_get_modal (window))
    atk_state_set_add_state (state_set, ATK_STATE_MODAL);

  if (gtk_window_get_resizable (window))
    atk_state_set_add_state (state_set, ATK_STATE_RESIZABLE);
 
  return state_set;
}

static gboolean
idle_notify_name_change (gpointer data)
{
  GailWindow *window;
  AtkObject *obj;

  window = GAIL_WINDOW (data);
  window->name_change_handler = 0;
  if (GTK_ACCESSIBLE (window)->widget == NULL)
    return FALSE;

  obj = ATK_OBJECT (window);
  if (obj->name == NULL)
    {
    /*
     * The title has changed so notify a change in accessible-name
     */
      g_object_notify (G_OBJECT (obj), "accessible-name");
    }
  g_signal_emit_by_name (obj, "visible_data_changed");

  return FALSE;
}

static void
gail_window_real_notify_gtk (GObject		*obj,
                             GParamSpec		*pspec)
{
  GtkWidget *widget = GTK_WIDGET (obj);
  AtkObject* atk_obj = gtk_widget_get_accessible (widget);
  GailWindow *window = GAIL_WINDOW (atk_obj);
  const gchar *name;
  gboolean name_changed = FALSE;

  if (strcmp (pspec->name, "title") == 0)
    {
      name = gtk_window_get_title (GTK_WINDOW (widget));
      if (name)
        {
         if (window->previous_name == NULL ||
             strcmp (name, window->previous_name) != 0)
           name_changed = TRUE;
        }
      else if (window->previous_name != NULL)
        name_changed = TRUE;

      if (name_changed)
        {
          g_free (window->previous_name);
          window->previous_name = g_strdup (name);
       
          if (window->name_change_handler == 0)
            window->name_change_handler = gdk_threads_add_idle (idle_notify_name_change, atk_obj);
        }
    }
  else
    GAIL_WIDGET_CLASS (gail_window_parent_class)->notify_gtk (obj, pspec);
}

static gboolean
gail_window_state_event_gtk (GtkWidget           *widget,
                             GdkEventWindowState *event)
{
  AtkObject* obj;

  obj = gtk_widget_get_accessible (widget);
  atk_object_notify_state_change (obj, ATK_STATE_ICONIFIED,
                         (event->new_window_state & GDK_WINDOW_STATE_ICONIFIED) != 0);
  return FALSE;
}

static void
atk_component_interface_init (AtkComponentIface *iface)
{
  iface->get_extents = gail_window_get_extents;
  iface->get_size = gail_window_get_size;
  iface->get_mdi_zorder = gail_window_get_mdi_zorder;
}

static void
gail_window_get_extents (AtkComponent  *component,
                         gint          *x,
                         gint          *y,
                         gint          *width,
                         gint          *height,
                         AtkCoordType  coord_type)
{
  GtkWidget *widget = GTK_ACCESSIBLE (component)->widget; 
  GdkRectangle rect;
  gint x_toplevel, y_toplevel;

  if (widget == NULL)
    /*
     * State is defunct
     */
    return;

  gail_return_if_fail (GTK_IS_WINDOW (widget));

  if (!gtk_widget_is_toplevel (widget))
    {
      AtkComponentIface *parent_iface;

      parent_iface = (AtkComponentIface *) g_type_interface_peek_parent (ATK_COMPONENT_GET_IFACE (component));
      parent_iface->get_extents (component, x, y, width, height, coord_type);
      return;
    }

  gdk_window_get_frame_extents (widget->window, &rect);

  *width = rect.width;
  *height = rect.height;
  if (!gtk_widget_is_drawable (widget))
    {
      *x = G_MININT;
      *y = G_MININT;
      return;
    }
  *x = rect.x;
  *y = rect.y;
  if (coord_type == ATK_XY_WINDOW)
    {
      gdk_window_get_origin (widget->window, &x_toplevel, &y_toplevel);
      *x -= x_toplevel;
      *y -= y_toplevel;
    }
}

static void
gail_window_get_size (AtkComponent *component,
                      gint         *width,
                      gint         *height)
{
  GtkWidget *widget = GTK_ACCESSIBLE (component)->widget; 
  GdkRectangle rect;

  if (widget == NULL)
    /*
     * State is defunct
     */
    return;

  gail_return_if_fail (GTK_IS_WINDOW (widget));

  if (!gtk_widget_is_toplevel (widget))
    {
      AtkComponentIface *parent_iface;

      parent_iface = (AtkComponentIface *) g_type_interface_peek_parent (ATK_COMPONENT_GET_IFACE (component));
      parent_iface->get_size (component, width, height);
      return;
    }
  gdk_window_get_frame_extents (widget->window, &rect);

  *width = rect.width;
  *height = rect.height;
}

#if defined (GDK_WINDOWING_X11)

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gdk/x11/gdkx.h>

/* _NET_CLIENT_LIST_STACKING monitoring */

typedef struct {
  Window     *stacked_windows;
  int         stacked_windows_len;
  GdkWindow  *root_window;
  guint       update_handler;
  int        *desktop;
  guint       update_desktop_handler;
  gboolean   *desktop_changed;

  guint       screen_initialized : 1;
  guint       update_stacked_windows : 1;
} GailScreenInfo;

static GailScreenInfo *gail_screens = NULL;
static int             num_screens = 0;
static Atom            _net_client_list_stacking = None;
static Atom            _net_wm_desktop = None;

static gint
get_window_desktop (Window window)
{
  Atom            ret_type;
  int             format;
  gulong          nitems;
  gulong          bytes_after;
  guchar         *cardinals;
  int             error;
  int             result;
  int             desktop;

  if (_net_wm_desktop == None)
    _net_wm_desktop =
		XInternAtom (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), "_NET_WM_DESKTOP", False);

  gdk_error_trap_push ();
  result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), window, _net_wm_desktop,
                               0, G_MAXLONG,
                               False, XA_CARDINAL,
                               &ret_type, &format, &nitems,
                               &bytes_after, &cardinals);
  error = gdk_error_trap_pop();
  /* nitems < 1 will occur if the property is not set */
  if (error != Success || result != Success || nitems < 1)
    return -1;

  desktop = *cardinals;

  XFree (cardinals);
  if (nitems != 1)
    return -1;
  return desktop;
}

static void
free_screen_info (GailScreenInfo *info)
{
  if (info->stacked_windows)
    XFree (info->stacked_windows);
  if (info->desktop)
    g_free (info->desktop);
  if (info->desktop_changed)
    g_free (info->desktop_changed);

  info->stacked_windows = NULL;
  info->stacked_windows_len = 0;
  info->desktop = NULL;
  info->desktop_changed = NULL;
}

static gboolean
get_stacked_windows (GailScreenInfo *info)
{
  Atom    ret_type;
  int     format;
  gulong  nitems;
  gulong  bytes_after;
  guchar *data;
  int     error;
  int     result;
  int     i;
  int     j;
  int    *desktops;
  gboolean *desktops_changed;

  if (_net_client_list_stacking == None)
    _net_client_list_stacking =
		XInternAtom (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()), "_NET_CLIENT_LIST_STACKING", False);

  gdk_error_trap_push ();
  ret_type = None;
  result = XGetWindowProperty (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
                               GDK_WINDOW_XWINDOW (info->root_window),
                               _net_client_list_stacking,
                               0, G_MAXLONG,
                               False, XA_WINDOW, &ret_type, &format, &nitems,
                               &bytes_after, &data);
  error = gdk_error_trap_pop ();
  /* nitems < 1 will occur if the property is not set */
  if (error != Success || result != Success || nitems < 1)
    {
      free_screen_info (info);
      return FALSE;
    }

  if (ret_type != XA_WINDOW)
    {
      XFree (data);
      free_screen_info (info);
      return FALSE;
    }

  desktops = g_malloc0 (nitems * sizeof (int));
  desktops_changed = g_malloc0 (nitems * sizeof (gboolean));
  for (i = 0; i < nitems; i++)
    {
      gboolean window_found = FALSE;

      for (j = 0; j < info->stacked_windows_len; j++)
        {
          if (info->stacked_windows [j] == data [i])
            {
              desktops [i] = info->desktop [j];
              desktops_changed [i] = info->desktop_changed [j];
              window_found = TRUE;
              break;
            }
        }
      if (!window_found)
        {
          desktops [i] = get_window_desktop (data [i]);
          desktops_changed [i] = FALSE;
        }
    }
  free_screen_info (info);
  info->stacked_windows = (Window*) data;
  info->stacked_windows_len = nitems;
  info->desktop = desktops;
  info->desktop_changed = desktops_changed;

  return TRUE;
}

static gboolean
update_screen_info (gpointer data)
{
  int screen_n = GPOINTER_TO_INT (data);

  gail_screens [screen_n].update_handler = 0;
  gail_screens [screen_n].update_stacked_windows = FALSE;

  get_stacked_windows (&gail_screens [screen_n]);

  return FALSE;
}

static gboolean
update_desktop_info (gpointer data)
{
  int screen_n = GPOINTER_TO_INT (data);
  GailScreenInfo *info;
  int i;

  info = &gail_screens [screen_n];
  info->update_desktop_handler = 0;

  for (i = 0; i < info->stacked_windows_len; i++)
    {
      if (info->desktop_changed [i])
        {
          info->desktop [i] = get_window_desktop (info->stacked_windows [i]);
          info->desktop_changed [i] = FALSE;
        }
    }

  return FALSE;
}

static GdkFilterReturn
filter_func (GdkXEvent *gdkxevent,
	     GdkEvent  *event,
	     gpointer   data)
{
  XEvent *xevent = gdkxevent;

  if (xevent->type == PropertyNotify)
    {
      if (xevent->xproperty.atom == _net_client_list_stacking)
        {
          int     screen_n;
          GdkWindow *window;

          window = event->any.window;

          if (window)
            {
              screen_n = gdk_screen_get_number (gdk_window_get_screen (window));

              gail_screens [screen_n].update_stacked_windows = TRUE;
              if (!gail_screens [screen_n].update_handler)
                {
                  gail_screens [screen_n].update_handler = gdk_threads_add_idle (update_screen_info,
	        						                 GINT_TO_POINTER (screen_n));
                }
            }
        }
      else if (xevent->xproperty.atom == _net_wm_desktop)
        {
          int     i;
          int     j;
          GailScreenInfo *info;

          for (i = 0; i < num_screens; i++)
            {
              info = &gail_screens [i];
              for (j = 0; j < info->stacked_windows_len; j++)
                {
                  if (xevent->xany.window == info->stacked_windows [j])
                    {
                      info->desktop_changed [j] = TRUE;
                      if (!info->update_desktop_handler)
                        {
                          info->update_desktop_handler = gdk_threads_add_idle (update_desktop_info,
                                                                               GINT_TO_POINTER (i));
                        }
                      break;
                    }
                }
            }
        }
    }
  return GDK_FILTER_CONTINUE;
}

static void
display_closed (GdkDisplay *display,
		gboolean    is_error)
{
  int i;

  for (i = 0; i < num_screens; i++)
    {
      if (gail_screens [i].update_handler)
	{
	  g_source_remove (gail_screens [i].update_handler);
	  gail_screens [i].update_handler = 0;
	}

      if (gail_screens [i].update_desktop_handler)
	{
	  g_source_remove (gail_screens [i].update_desktop_handler);
	  gail_screens [i].update_desktop_handler = 0;
	}

      free_screen_info (&gail_screens [i]);
    }

  g_free (gail_screens);
  gail_screens = NULL;
  num_screens = 0;
}

static void
init_gail_screens (void)
{
  GdkDisplay *display;

  display = gdk_display_get_default ();

  num_screens = gdk_display_get_n_screens (display);

  gail_screens = g_new0 (GailScreenInfo, num_screens);
  gdk_window_add_filter (NULL, filter_func, NULL);

  g_signal_connect (display, "closed", G_CALLBACK (display_closed), NULL);
}

static void
init_gail_screen (GdkScreen *screen,
                  int        screen_n)
{
  XWindowAttributes attrs;

  gail_screens [screen_n].root_window = gdk_screen_get_root_window (screen);

  get_stacked_windows (&gail_screens [screen_n]);

  XGetWindowAttributes (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
			GDK_WINDOW_XWINDOW (gail_screens [screen_n].root_window),
			&attrs); 

  XSelectInput (GDK_DISPLAY_XDISPLAY (gdk_display_get_default ()),
		GDK_WINDOW_XWINDOW (gail_screens [screen_n].root_window),
		attrs.your_event_mask | PropertyChangeMask);
           
  gail_screens [screen_n].screen_initialized = TRUE;
}

static GailScreenInfo *
get_screen_info (GdkScreen *screen)
{
  int screen_n;

  gail_return_val_if_fail (GDK_IS_SCREEN (screen), NULL);

  screen_n = gdk_screen_get_number (screen);

  if (gail_screens && gail_screens [screen_n].screen_initialized)
    return &gail_screens [screen_n];

  if (!gail_screens)
    init_gail_screens ();

  g_assert (gail_screens != NULL);

  init_gail_screen (screen, screen_n);

  g_assert (gail_screens [screen_n].screen_initialized);

  return &gail_screens [screen_n];
}

static gint
get_window_zorder (GdkWindow *window)
{
  GailScreenInfo *info;
  Window          xid;
  int             i;
  int             zorder;
  int             w_desktop;

  gail_return_val_if_fail (GDK_IS_WINDOW (window), -1);

  info = get_screen_info (gdk_window_get_screen (window));

  gail_return_val_if_fail (info->stacked_windows != NULL, -1);

  xid = GDK_WINDOW_XID (window);

  w_desktop = -1;
  for (i = 0; i < info->stacked_windows_len; i++)
    {
      if (info->stacked_windows [i] == xid)
        {
          w_desktop = info->desktop[i];
          break;
        }
    }
  if (w_desktop < 0)
    return w_desktop;

  zorder = 0;
  for (i = 0; i < info->stacked_windows_len; i++)
    {
      if (info->stacked_windows [i] == xid)
        {
          return zorder;
        }
      else
        {
          if (info->desktop[i] == w_desktop)
            zorder++;
        }
     }

  return -1;
}

static gint
gail_window_get_mdi_zorder (AtkComponent *component)
{
  GtkWidget *widget = GTK_ACCESSIBLE (component)->widget;

  if (widget == NULL)
    /*
     * State is defunct
     */
    return -1;

  gail_return_val_if_fail (GTK_IS_WINDOW (widget), -1);

  return get_window_zorder (widget->window);
}

#elif defined (GDK_WINDOWING_WIN32)

static gint
gail_window_get_mdi_zorder (AtkComponent *component)
{
  GtkWidget *widget = GTK_ACCESSIBLE (component)->widget;

  if (widget == NULL)
    /*
     * State is defunct
     */
    return -1;

  gail_return_val_if_fail (GTK_IS_WINDOW (widget), -1);

  return 0;			/* Punt, FIXME */
}

#else

static gint
gail_window_get_mdi_zorder (AtkComponent *component)
{
  GtkWidget *widget = GTK_ACCESSIBLE (component)->widget;

  if (widget == NULL)
    /*
     * State is defunct
     */
    return -1;

  gail_return_val_if_fail (GTK_IS_WINDOW (widget), -1);

  return 0;			/* Punt, FIXME */
}

#endif
