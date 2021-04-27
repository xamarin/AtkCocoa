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

#include "config.h"
#include <gtk/gtk.h>
#include <gdk/gdkquartz.h>

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include "atk-cocoa/acelement.h"
#include "atk-cocoa/acdebug.h"
#include "atk-cocoa/acmarshal.h"
#import "atk-cocoa/ACAccessibilityElement.h"

static void ac_element_class_init (AcElementClass *klass);
static void ac_element_init (AcElement *element);
static void ac_element_dispose (GObject *obj);
static void ac_element_finalize (GObject *obj);
static void ac_element_real_focus_event (AtkObject *object, gboolean focus);
static ACAccessibilityElement *ac_element_get_real_accessibility_element (AcElement *element);
static NSArray *ac_element_real_get_actions (AcElement *element);
static gboolean ac_element_real_perform_cancel (AcElement *element);
static gboolean ac_element_real_perform_confirm (AcElement *element);
static gboolean ac_element_real_perform_decrement (AcElement *element);
static gboolean ac_element_real_perform_delete (AcElement *element);
static gboolean ac_element_real_perform_increment (AcElement *element);
static gboolean ac_element_real_perform_pick (AcElement *element);
static gboolean ac_element_real_perform_press (AcElement *element);
static gboolean ac_element_real_perform_raise (AcElement *element);
static gboolean ac_element_real_perform_show_alternate_ui (AcElement *element);
static gboolean ac_element_real_perform_show_default_ui (AcElement *element);
static gboolean ac_element_real_perform_show_menu (AcElement *element);

#define AC_ELEMENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), AC_TYPE_ELEMENT, AcElementPrivate))

enum {
	GET_ACTIONS,

	/* Action signals */
	PERFORM_CANCEL,
	PERFORM_CONFIRM,
	PERFORM_DECREMENT,
	PERFORM_DELETE,
	PERFORM_INCREMENT,
	PERFORM_PICK,
	PERFORM_PRESS,
	PERFORM_RAISE,
	PERFORM_SHOW_ALTERNATE_UI,
	PERFORM_SHOW_DEFAULT_UI,
	PERFORM_SHOW_MENU,

	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

struct _AcElementPrivate {
	void *real_element; /* ACAccessibilityElement * to get around ARC issues */
	GObject *owner;
};

G_DEFINE_TYPE (AcElement, ac_element, GTK_TYPE_ACCESSIBLE)

static void
ac_element_class_init (AcElementClass *klass)
{
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);
  GtkAccessibleClass *accessible_class = GTK_ACCESSIBLE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  class->focus_event = ac_element_real_focus_event;
  klass->get_accessibility_element = ac_element_get_real_accessibility_element;
  klass->get_actions = ac_element_real_get_actions;

  klass->perform_cancel = ac_element_real_perform_cancel;
  klass->perform_confirm = ac_element_real_perform_confirm;
  klass->perform_decrement = ac_element_real_perform_decrement;
  klass->perform_delete = ac_element_real_perform_delete;
  klass->perform_increment = ac_element_real_perform_increment;
  klass->perform_pick = ac_element_real_perform_pick;
  klass->perform_press = ac_element_real_perform_press;
  klass->perform_raise = ac_element_real_perform_raise;
  klass->perform_show_alternate_ui = ac_element_real_perform_show_alternate_ui;
  klass->perform_show_default_ui = ac_element_real_perform_show_default_ui;
  klass->perform_show_menu = ac_element_real_perform_show_menu;

  object_class->dispose = ac_element_dispose;
  object_class->finalize = ac_element_finalize;
  g_type_class_add_private (G_OBJECT_CLASS (klass), sizeof (AcElementPrivate));

  signals[GET_ACTIONS] = g_signal_new ("request-actions", G_TYPE_FROM_CLASS (klass),
  									   G_SIGNAL_RUN_LAST, 0, 
  									   g_signal_accumulator_first_wins, NULL,
  									   _ac_marshal_POINTER__VOID,	
  									   G_TYPE_POINTER, 0);
  signals[PERFORM_CANCEL] = g_signal_new ("perform-cancel", G_TYPE_FROM_CLASS (klass),
  										  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[PERFORM_CONFIRM] = g_signal_new ("perform-confirm", G_TYPE_FROM_CLASS (klass),
  										  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[PERFORM_DECREMENT] = g_signal_new ("perform-decrement", G_TYPE_FROM_CLASS (klass),
  										  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[PERFORM_DELETE] = g_signal_new ("perform-delete", G_TYPE_FROM_CLASS (klass),
  										  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[PERFORM_INCREMENT] = g_signal_new ("perform-increment", G_TYPE_FROM_CLASS (klass),
  										  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[PERFORM_PICK] = g_signal_new ("perform-pick", G_TYPE_FROM_CLASS (klass),
  										  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[PERFORM_PRESS] = g_signal_new ("perform-press", G_TYPE_FROM_CLASS (klass),
  										  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[PERFORM_RAISE] = g_signal_new ("perform-raise", G_TYPE_FROM_CLASS (klass),
  										  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[PERFORM_SHOW_ALTERNATE_UI] = g_signal_new ("perform-show-alternate-ui", G_TYPE_FROM_CLASS (klass),
  										  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[PERFORM_SHOW_DEFAULT_UI] = g_signal_new ("perform-show-default-ui", G_TYPE_FROM_CLASS (klass),
  										  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[PERFORM_SHOW_MENU] = g_signal_new ("perform-show-menu", G_TYPE_FROM_CLASS (klass),
  										  G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void
ac_element_init (AcElement *element)
{
	element->priv = AC_ELEMENT_GET_PRIVATE (element);
}

static void
ac_element_dispose (GObject *obj)
{
	AcElement *element = AC_ELEMENT (obj);

	if (element->priv->real_element != NULL) {
		AC_NOTE (DESTRUCTION, g_print ("Disposing element %s - %ld\n", G_OBJECT_TYPE_NAME (obj), CFGetRetainCount (element->priv->real_element)));
        NSAccessibilityPostNotification(ac_element_get_accessibility_element(element), NSAccessibilityUIElementDestroyedNotification);
		CFRelease (element->priv->real_element);
		element->priv->real_element = NULL;
	}

	G_OBJECT_CLASS (ac_element_parent_class)->dispose (obj);
}

static void
ac_element_finalize (GObject *obj)
{
	AC_NOTE (DESTRUCTION, g_print ("Finalizing object %s\n", G_OBJECT_TYPE_NAME (obj)));

	G_OBJECT_CLASS (ac_element_parent_class)->finalize (obj);
}

gboolean ignore_next_focus = FALSE;
static void
ac_element_real_focus_event (AtkObject *object,
                             gboolean focus)
{
  if (!ignore_next_focus) {
    ACAccessibilityElement *element = ac_element_get_accessibility_element (AC_ELEMENT (object));
    [element setAccessibilityFocused:focus];
  }
  ignore_next_focus = FALSE;
}

void
ac_element_focus_and_ignore_next (AcElement *element)
{
  ignore_next_focus = FALSE;
  if (!ignore_next_focus) {
    ACAccessibilityElement *real_element = ac_element_get_accessibility_element (element);
    [real_element setAccessibilityFocused:YES];
    [[NSApplication sharedApplication] setAccessibilityApplicationFocusedUIElement:real_element];
    NSAccessibilityPostNotificationWithUserInfo([NSApplication sharedApplication],
                                                NSAccessibilityFocusedUIElementChangedNotification,
                                                @{NSAccessibilityUIElementsKey: @[real_element]});
    ignore_next_focus = TRUE;
  } else {
    ignore_next_focus = FALSE;
  }
}
static id<NSAccessibility>
ac_element_get_real_accessibility_element (AcElement *element)
{
	return [[ACAccessibilityElement alloc] initWithDelegate:element];
}

static NSArray *
ac_element_real_get_actions (AcElement *element)
{
	char **actions;
	NSMutableArray *realActions;

	g_signal_emit (element, signals[GET_ACTIONS], 0, &actions);

	if (actions == NULL) {
		return NULL;
	}

	realActions = [NSMutableArray array];
	for (int i = 0; actions[i]; i++) {
		NSString *name = [[NSString alloc] initWithCString:actions[i] encoding:NSUTF8StringEncoding];
		[realActions addObject:name];
	}

	return realActions;
}

static gboolean
ac_element_real_perform_cancel (AcElement *element)
{
	g_signal_emit (element, signals[PERFORM_CANCEL], 0);
	return TRUE;
}

static gboolean
ac_element_real_perform_confirm (AcElement *element)
{
	g_signal_emit (element, signals[PERFORM_CONFIRM], 0);
	return TRUE;
}

static gboolean
ac_element_real_perform_decrement (AcElement *element)
{
	g_signal_emit (element, signals[PERFORM_DECREMENT], 0);
	return TRUE;
}

static gboolean
ac_element_real_perform_delete (AcElement *element)
{
	g_signal_emit (element, signals[PERFORM_DELETE], 0);
	return TRUE;
}

static gboolean
ac_element_real_perform_increment (AcElement *element)
{
	g_signal_emit (element, signals[PERFORM_INCREMENT], 0);
	return TRUE;
}

static gboolean
ac_element_real_perform_pick (AcElement *element)
{
	g_signal_emit (element, signals[PERFORM_PICK], 0);
	return TRUE;
}

static gboolean
ac_element_real_perform_press (AcElement *element)
{
	g_signal_emit (element, signals[PERFORM_PRESS], 0);
	return TRUE;
}

static gboolean
ac_element_real_perform_raise (AcElement *element)
{
	g_signal_emit (element, signals[PERFORM_RAISE], 0);
	return TRUE;
}

static gboolean
ac_element_real_perform_show_alternate_ui (AcElement *element)
{
	g_signal_emit (element, signals[PERFORM_SHOW_ALTERNATE_UI], 0);
	return TRUE;
}

static gboolean
ac_element_real_perform_show_default_ui (AcElement *element)
{
	g_signal_emit (element, signals[PERFORM_SHOW_DEFAULT_UI], 0);
	return TRUE;
}

static gboolean
ac_element_real_perform_show_menu (AcElement *element)
{
	g_signal_emit (element, signals[PERFORM_SHOW_MENU], 0);
	return TRUE;
}

void
ac_element_set_owner (AcElement *element,
					  GObject *owner)
{
	AcElementPrivate *priv;

	g_return_if_fail (AC_IS_ELEMENT (element));
	g_return_if_fail (G_IS_OBJECT (owner));

	priv = element->priv;

	priv->owner = owner;
	g_object_add_weak_pointer (owner, (gpointer *)&priv->owner);

	// This will call into subclasses to get the appropriate id<NSAccessibility> for that object
	void *real_element = (__bridge void *) ac_element_get_accessibility_element (element);

	// Set the NSAccessibilityElement as data on the AtkObject
	// so it can be accessed from managed code which can't know about AcElement
	g_object_set_data (G_OBJECT (element), "xamarin-private-atkcocoa-nsaccessibility", real_element);
}

GObject *
ac_element_get_owner (AcElement *element)
{
	AcElementPrivate *priv;

	g_return_val_if_fail (AC_IS_ELEMENT (element), NULL);

	priv = element->priv;

	return priv->owner;
}

id<NSAccessibility>
ac_element_get_accessibility_element (AcElement *element)
{
	AcElementClass *klass;

	g_return_val_if_fail (AC_IS_ELEMENT (element), NULL);

	if (element->priv->real_element == NULL) {
		klass = AC_ELEMENT_GET_CLASS (element);
		if (klass->get_accessibility_element) {
			element->priv->real_element = (__bridge_retained void *) klass->get_accessibility_element (element);
            if (element != NULL) {
                ac_element_notify(element, NSAccessibilityCreatedNotification, nil);
            }
		} else {
			g_warning ("No accessibility element method for %s", G_OBJECT_TYPE_NAME (element));
		}
	}

	return (__bridge id<NSAccessibility>) element->priv->real_element;
}

/* Force the system to drop its cache of the accessibility element, in case the accessibility element changes
   like when a GtkWindow is realised */
void
ac_element_invalidate_accessibility_element (AcElement *element)
{
	g_return_if_fail (AC_IS_ELEMENT (element));

	if (element->priv->real_element != NULL) {
        CFBridgingRelease(element->priv->real_element);
		element->priv->real_element = NULL;
	}
}

id<NSAccessibility>
get_real_accessibility_parent (AcElement *element,
							   GtkWidget **ownerWidget)
{
	id<NSAccessibility> possibleParent = ac_element_get_accessibility_element (element);

	// Force everything to be a widget at the moment
	*ownerWidget = (GtkWidget *) ac_element_get_owner (element);
	while ([possibleParent isAccessibilityElement] == NO) {
		AC_NOTE (TREE, g_print ("Parent is not accessible\n"));

		GtkWidget *parentWidget = gtk_widget_get_parent (*ownerWidget);
		if (parentWidget == NULL) {
			// If we're as high as we can go, then the widget tree hasn't been added to the window yet,
			// so we have to add it to the highest parent, and it'll get fixed up when that parent is added to the full tree
			break;
		}

		element = (AcElement *) gtk_widget_get_accessible (parentWidget);
		AC_NOTE (TREE, g_print ("   Widget is %s (%p)\n", G_OBJECT_TYPE_NAME (parentWidget), parentWidget));
		possibleParent = ac_element_get_accessibility_element (element);
		*ownerWidget = parentWidget;
	}

	AC_NOTE (TREE, NSLog (@"Accessible parent is %p (%@)\n", possibleParent, [possibleParent description]));
	return possibleParent;
}

static void
update_window_and_toplevel (NSArray *children, id window)
{
  int i;

  if (children == nil || [children count] == 0) {
    return;
  }

  for (i = 0; i < [children count]; i++) {
    id<NSAccessibility> child = (id<NSAccessibility>)[children objectAtIndex:i];
    ACAccessibilityElement *e = nil;

    // Check objects are still valid
    if ([child isKindOfClass:[ACAccessibilityElement class]]) {
      e = (ACAccessibilityElement *)child;
      if (!ATK_IS_OBJECT ([e delegate])) {
        continue;
      }
    }

    if ([child respondsToSelector:@selector(setAccessibilityTopLevelUIElement:)]) {
      [child setAccessibilityTopLevelUIElement:window];
    }

    if ([child respondsToSelector:@selector(setAccessibilityWindow:)]) {
      [child setAccessibilityWindow:window];
    }

    if (![e hasDynamicChildren] && [child respondsToSelector:@selector(accessibilityChildren)]) {
      update_window_and_toplevel ([child accessibilityChildren], window);
    }
  }
}

void
ac_element_add_child (AcElement *parent,
					  AcElement *child)
{
	AcElementPrivate *parent_priv, *child_priv;
	AcElementClass *parent_class;
	NSArray *accessibilityChildren;
	NSMutableArray *newChildren;
	GtkWidget *parentOwnerWidget, *childWidget;
	GtkWidget *toplevelWindow;
	NSWindow *nsWindow = nil;

	id<NSAccessibility> parent_element, child_element;
	id<NSAccessibility> realChildAdded;

	g_return_if_fail (AC_IS_ELEMENT (parent));
	g_return_if_fail (AC_IS_ELEMENT (child));

	parent_priv = parent->priv;
	child_priv = child->priv;
	AC_NOTE (TREE, g_print ("Adding child: %s\n", atk_object_get_name (ATK_OBJECT (child))));
	AC_NOTE (TREE, g_print ("Adding parent: %s\n", atk_object_get_name (ATK_OBJECT (parent))));
	AC_NOTE (TREE, g_print ("ATKCocoa: Adding child %s(%s) to %s(%s)\n", G_OBJECT_TYPE_NAME (child), G_OBJECT_TYPE_NAME (child_priv->owner), G_OBJECT_TYPE_NAME (parent), G_OBJECT_TYPE_NAME (parent_priv->owner)));

    childWidget = (GtkWidget*)ac_element_get_owner(child);
    // GtkNSView emits its mapped signal 50ms before it is actually mapped
    // so we need to ignore it here and just accept it will be mapped very shortly
    if (!GTK_IS_NS_VIEW(childWidget) && !gtk_widget_get_mapped(childWidget)) {
      return;
    }
	parent_element = get_real_accessibility_parent (parent, &parentOwnerWidget);
	child_element = ac_element_get_accessibility_element (child);
	realChildAdded = child_element;

	// We can't use -accessibilityWindow or -accessibilityTopLevelUIElement here because
	// of a bug with NSAccessibilityElement that causes an infinite loop
	// TSI filed 4th August 2016 - waiting for a response.
	toplevelWindow = gtk_widget_get_toplevel (parentOwnerWidget);
	if (toplevelWindow && gtk_widget_get_realized (toplevelWindow)) {
		nsWindow = gdk_quartz_window_get_nswindow (gtk_widget_get_window (toplevelWindow));
	}

	if ([child_element isAccessibilityElement]){
		if ([parent_element isKindOfClass:[ACAccessibilityElement class]]) {
			[(ACAccessibilityElement *)parent_element accessibilityAddChildElement:child_element];

			if (nsWindow) {
				[child_element setAccessibilityWindow:nsWindow];
				[child_element setAccessibilityTopLevelUIElement:nsWindow];

              gboolean update = TRUE;
              if ([child_element isKindOfClass:[ACAccessibilityElement class]]) {
                update = ![((ACAccessibilityElement *)child_element) hasDynamicChildren];
              }
              if (update) {
                update_window_and_toplevel ([child_element accessibilityChildren], nsWindow);
              }
			}
		} else {
			NSMutableArray *new_children = [[parent_element accessibilityChildren] mutableCopy];
			[new_children addObject:child_element];
			[parent_element setAccessibilityChildren:new_children];
			[child_element setAccessibilityParent:parent_element];

			if (nsWindow) {
				[child_element setAccessibilityWindow:nsWindow];
				[child_element setAccessibilityTopLevelUIElement:nsWindow];

              gboolean update = TRUE;
              if ([child_element isKindOfClass:[ACAccessibilityElement class]]) {
                update = ![((ACAccessibilityElement *)child_element) hasDynamicChildren];
              }
              if (update) {
				update_window_and_toplevel ([child_element accessibilityChildren], nsWindow);
              }
			}
		}
	} else {
		// If the child is not accessible, then we need to add its unignored children
		// This will fix up the situation mentioned in get_real_accessibility_parent
		NSArray *unignoredChildren = NULL;
		AC_NOTE (TREE, NSLog (@"ATKCocoa:    Child %@ is not accessible\n", child_element));

		if ([child_element accessibilityChildren]) {
			BOOL isElement = [parent_element isKindOfClass:[ACAccessibilityElement class]];

			unignoredChildren = NSAccessibilityUnignoredChildren ([child_element accessibilityChildren]);

			if ([unignoredChildren count] == 0) {
				return;
			}
			realChildAdded = unignoredChildren[0];
			[child_element setAccessibilityChildren:nil];

			AC_NOTE (TREE, g_print ("ATKCocoa:       Adding %lu children\n", [unignoredChildren count]));
			for (int i = 0; i < [unignoredChildren count]; i++) {
				id<NSAccessibility> child = unignoredChildren[i];

				if (isElement) {
					[(ACAccessibilityElement *)parent_element accessibilityAddChildElement:child];
				} else {
					[child setAccessibilityParent:parent_element];
				}
				if (nsWindow) {
				    [child setAccessibilityTopLevelUIElement:nsWindow];
				    [child setAccessibilityWindow:nsWindow];

					update_window_and_toplevel ([child accessibilityChildren], nsWindow);
				}

			}

			if (!isElement) {
				NSMutableArray *new_children = [[parent_element accessibilityChildren] mutableCopy];
				[new_children addObjectsFromArray:unignoredChildren];
				[parent_element setAccessibilityChildren:new_children];
			}
			AC_NOTE (TREE, g_print ("ATKCocoa:       Parent has %lu children\n", [[parent_element accessibilityChildren] count]));
		} else {
			// Not adding anything in this case
			realChildAdded = NULL;
		}

		AC_NOTE (TREE, NSLog (@"%@ added", realChildAdded));
	}

	if (realChildAdded == NULL) {
		return;
	}

	parent_class = AC_ELEMENT_GET_CLASS (parent);
	if (parent_class->child_was_added) {
		AcElement *delegateChild = [(ACAccessibilityElement *)realChildAdded delegate];
		parent_class->child_was_added (parent, delegateChild);
	}
}

static void
find_accessible_children (GtkWidget *widget,
						  gpointer data)
{
	NSMutableArray *allyChildren = (__bridge NSMutableArray *)data;
	AtkObject *atkElement;
	AcElement *element;
	id<NSAccessibility> realElement;

	atkElement = gtk_widget_get_accessible (widget);
	if (!AC_IS_ELEMENT (atkElement)) {
		return;
	}

	element = AC_ELEMENT (atkElement);
	realElement = ac_element_get_accessibility_element (element);

	AC_NOTE (TREE, NSLog (@"      - Found %@", realElement));
	if ([realElement isAccessibilityElement]) {
		AC_NOTE (TREE, NSLog (@"         - Is accessible"));
		// If the child is accessible, that's all we need to do
		[allyChildren addObject:realElement];
	} else {
		AC_NOTE (TREE, NSLog (@"         - Is not accessible"));
		if (!GTK_IS_CONTAINER (widget)) {
			AC_NOTE (TREE, NSLog (@"         - Is not container"));
			// If the widget is not a container, then there is nothing more to do
			return;
		}

		AC_NOTE (TREE, NSLog (@"         - Is container, going futher up tree"));
		// Otherwise go further up the tree looking for accessible children
		gtk_container_forall (GTK_CONTAINER (widget), find_accessible_children, data);
	}
}

void
ac_element_remove_child (AcElement *parent,
						 AcElement *child)
{
	AcElementPrivate *parent_priv, *child_priv;
    GtkWidget *childWidget;
	id<NSAccessibility> parent_element, child_element;

	g_return_if_fail (AC_IS_ELEMENT (parent));
	g_return_if_fail (AC_IS_ELEMENT (child));

	AC_NOTE (TREE, g_print ("Removing %s (%s) from %s (%s)\n", G_OBJECT_TYPE_NAME (child), G_OBJECT_TYPE_NAME (child->priv->owner), G_OBJECT_TYPE_NAME (parent), G_OBJECT_TYPE_NAME (parent->priv->owner)));

	child_element = ac_element_get_accessibility_element (child);
	parent_element = [child_element accessibilityParent];

	AC_NOTE (TREE, NSLog (@"Child element: %p - %@", child_element, child_element));
	AC_NOTE (TREE, NSLog (@"   - accessibility children: %lu", [[child_element accessibilityChildren] count]));
	AC_NOTE (TREE, NSLog (@"Parent element: %p - %@", parent_element, parent_element));

	GtkWidget *parentWidget;
	parent_element = get_real_accessibility_parent (parent, &parentWidget);

	AC_NOTE (TREE, NSLog (@"Real parent element: %p - %@ %s", parent_element, parent_element, G_OBJECT_TYPE_NAME (parentWidget)));

	BOOL childAccessible = [child_element isAccessibilityElement];
	BOOL parentAccessible = [parent_element isAccessibilityElement];

	// By getting the real accessibility parent, the parent should always be accessible
	// (or if it isn't, then it should be the temporary parent which we treat as being accessible
	// until the tree is complete)
	if (childAccessible) {
		AC_NOTE (TREE, NSLog (@"   - Child is accessible"));

		// If both the child and the parent (after getting the correct parent) are both
		// accessibility enabled, then it's just a simple case to remove the child from the parent
		NSMutableArray *new_children = [[parent_element accessibilityChildren] mutableCopy];
		[new_children removeObject:child_element];
		[parent_element setAccessibilityChildren:new_children];
	} else {
		AC_NOTE (TREE, NSLog (@"   - Child is not accessible"));
		// If the parent is accessible, but the child is not, then gather all the child's directly
		// accessible children and remove them from the parent
		GObject *owner = ac_element_get_owner (child);
		if (!GTK_IS_WIDGET (owner)) {
			g_warning ("Child owner is not GtkWidget: %s", G_OBJECT_TYPE_NAME (owner));
			return;
		}

		if (!GTK_IS_CONTAINER (owner)) {
			AC_NOTE (TREE, NSLog (@"   - Child is not a container: %s", G_OBJECT_TYPE_NAME (owner)));
			// Can't have children
			return;
		}

		NSMutableArray *allyChildren = [NSMutableArray array];
		gtk_container_forall (GTK_CONTAINER (owner), find_accessible_children, (__bridge void *) allyChildren);

		AC_NOTE (TREE, NSLog (@"   - Went looking for children, found: %lu", [allyChildren count]));
		if ([allyChildren count] == 0) {
			return;
		}

		NSMutableArray *parentChildren = [[parent_element accessibilityChildren] mutableCopy];
		AC_NOTE (TREE, NSLog (@"   - parent had %lu children", [parentChildren count]));

		int i = 0;
		for (id<NSAccessibility> child in allyChildren) {
			AC_NOTE (TREE, NSLog (@"   %d %@ - %@", i, child, [child accessibilityParent]));
			i++;

			[parentChildren removeObject:child];
		}

		AC_NOTE (TREE, NSLog (@"   - parent now has %lu children", [parentChildren count]));
		[parent_element setAccessibilityChildren:parentChildren];

		// Need to set the removed children as accessibility children on @child
		// otherwise the removed children will get lost if @child is reparented somewhere else
		[child_element setAccessibilityChildren:allyChildren];
	}
}

NSString *
ac_element_get_text (AcElement *element)
{
	NSString *result = nil;

	if (ATK_IS_TEXT (element)) {
		const char *text = atk_text_get_text (ATK_TEXT (element), 0, -1);

		result = nsstring_from_cstring(text);
		g_free(text);
	}

	return nil;
}

NSArray *
ac_element_get_actions (AcElement *element)
{
	AcElementClass *klass;

	g_return_val_if_fail (AC_IS_ELEMENT (element), NULL);

	klass = AC_ELEMENT_GET_CLASS (element);
	if (klass->get_actions) {
		return klass->get_actions (element);
	} else {
		return NULL;
	}
}

gboolean
ac_element_perform_cancel (AcElement *element)
{
	AcElementClass *klass;

	g_return_val_if_fail (AC_IS_ELEMENT (element), FALSE);

	klass = AC_ELEMENT_GET_CLASS (element);
	if (klass->perform_cancel) {
		return klass->perform_cancel (element);
	}

	return FALSE;
}

gboolean
ac_element_perform_confirm (AcElement *element)
{
	AcElementClass *klass;

	g_return_val_if_fail (AC_IS_ELEMENT (element), FALSE);

	klass = AC_ELEMENT_GET_CLASS (element);
	if (klass->perform_confirm) {
		return klass->perform_confirm (element);
	}

	return FALSE;
}

gboolean
ac_element_perform_decrement (AcElement *element)
{
	AcElementClass *klass;

	g_return_val_if_fail (AC_IS_ELEMENT (element), FALSE);

	klass = AC_ELEMENT_GET_CLASS (element);
	if (klass->perform_decrement) {
		return klass->perform_decrement (element);
	}

	return FALSE;
}

gboolean
ac_element_perform_delete (AcElement *element)
{
	AcElementClass *klass;

	g_return_val_if_fail (AC_IS_ELEMENT (element), FALSE);

	klass = AC_ELEMENT_GET_CLASS (element);
	if (klass->perform_delete) {
		return klass->perform_delete (element);
	}

	return FALSE;
}

gboolean
ac_element_perform_increment (AcElement *element)
{
	AcElementClass *klass;

	g_return_val_if_fail (AC_IS_ELEMENT (element), FALSE);

	klass = AC_ELEMENT_GET_CLASS (element);
	if (klass->perform_increment) {
		return klass->perform_increment (element);
	}

	return FALSE;
}

gboolean
ac_element_perform_pick (AcElement *element)
{
	AcElementClass *klass;

	g_return_val_if_fail (AC_IS_ELEMENT (element), FALSE);

	klass = AC_ELEMENT_GET_CLASS (element);
	if (klass->perform_pick) {
		return klass->perform_pick (element);
	}

	return FALSE;
}

gboolean
ac_element_perform_press (AcElement *element)
{
	AcElementClass *klass;

	g_return_val_if_fail (AC_IS_ELEMENT (element), FALSE);

	klass = AC_ELEMENT_GET_CLASS (element);
	if (klass->perform_press) {
		return klass->perform_press (element);
	}

	return FALSE;
}

gboolean
ac_element_perform_raise (AcElement *element)
{
	AcElementClass *klass;

	g_return_val_if_fail (AC_IS_ELEMENT (element), FALSE);

	klass = AC_ELEMENT_GET_CLASS (element);
	if (klass->perform_raise) {
		return klass->perform_raise (element);
	}

	return FALSE;
}

gboolean
ac_element_perform_show_alternate_ui (AcElement *element)
{
	AcElementClass *klass;

	g_return_val_if_fail (AC_IS_ELEMENT (element), FALSE);

	klass = AC_ELEMENT_GET_CLASS (element);
	if (klass->perform_show_alternate_ui) {
		return klass->perform_show_alternate_ui (element);
	}

	return FALSE;
}

gboolean
ac_element_perform_show_default_ui (AcElement *element)
{
	AcElementClass *klass;

	g_return_val_if_fail (AC_IS_ELEMENT (element), FALSE);

	klass = AC_ELEMENT_GET_CLASS (element);
	if (klass->perform_show_default_ui) {
		return klass->perform_show_default_ui (element);
	}

	return FALSE;
}

gboolean
ac_element_perform_show_menu (AcElement *element)
{
	AcElementClass *klass;

	g_return_val_if_fail (AC_IS_ELEMENT (element), FALSE);

	klass = AC_ELEMENT_GET_CLASS (element);
	if (klass->perform_show_menu) {
		return klass->perform_show_menu (element);
	}

	return FALSE;
}

void
ac_element_notify (AcElement *element,
				   NSString *notificationName,
				   NSDictionary *userInfo)
{
	id realElement = ac_element_get_accessibility_element (element);
    if (realElement == NULL) {
        return;
    }

	if (userInfo) {
		NSAccessibilityPostNotificationWithUserInfo (realElement, notificationName, userInfo);
	} else {
		NSAccessibilityPostNotification (realElement, notificationName);
	}
}
