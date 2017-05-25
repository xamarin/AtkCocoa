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

#import "ACAccessibilityElement.h"

#include "acelement.h"
#include "acdebug.h"
#include "acutils.h"

@implementation ACAccessibilityElement {
	BOOL _isCreated;
	id _accessibilityWindow;
	AcElement *_delegate;
	NSString *_realTitle;
	NSString *_realRole;
	BOOL _accessibilityElementSet;
	BOOL _accElement;

	NSString *_delegate_type;
	NSString *_owner_type;
	NSString *_identifier;
}

- (instancetype) initWithDelegate:(AcElement *)delegate
{
	self = [super init];
	if (!self) {
		return nil;
	}

	_delegate = delegate;

	_isCreated = YES;

	if (delegate != NULL) {
		_delegate_type = nsstring_from_cstring (g_strdup (G_OBJECT_TYPE_NAME (delegate)));
		_owner_type = nsstring_from_cstring (g_strdup (G_OBJECT_TYPE_NAME (ac_element_get_owner (delegate))));
	}

	return self;
}

- (AcElement *)delegate
{
	return _delegate;
}

- (void)dealloc
{
	AC_NOTE (DESTRUCTION, (NSLog (@"Deallocing: %@", [super description])));
}

- (void)setAccessibilityElement:(BOOL)isElement
{
	if (_isCreated) {
		// setAccessibilityElement is called from NSAccessibilityElement::init so we want to ignore any calls
		// until the init process is finished
		_accessibilityElementSet = YES;
	}
	[super setAccessibilityElement:isElement];
}

- (BOOL)isAccessibilityElement
{
	if (_accessibilityElementSet) {
		return [super isAccessibilityElement];
	}

	// Deduce this from the AtkRole
	return atk_object_get_role (ATK_OBJECT (_delegate)) != ATK_ROLE_FILLER;
}

// Cocoa appears to have a bug where if accessibilityWindow is not set
// it will get into an infinite loop looking for one. We can work around this in ACAccessibilityElement
// by just returning nil if one isn't set.
- (id)accessibilityWindow
{
	return _accessibilityWindow;
}

- (void)setAccessibilityWindow:(id)window
{
	[super setAccessibilityWindow:window];
	_accessibilityWindow = window;
}

- (NSString *)description
{
	return [NSString stringWithFormat:@"%@ (%@ (%p)- %@ (%p) - %@)", [super description], _delegate_type, _delegate, _owner_type, ac_element_get_owner (_delegate), _identifier ?: @"None set"];
}

static char *get_full_object_path (GtkWidget *object)
{
	GString *builder = g_string_new (G_OBJECT_TYPE_NAME (object));

	while ((object = gtk_widget_get_parent (object))) {
		g_string_append_printf (builder, ".%s", G_OBJECT_TYPE_NAME (object));
	}

	char *retval = builder->str;
	g_string_free (builder, FALSE);
	return retval;
}

- (GdkRectangle)frameInGtkWindowSpace
{
	int windowX, windowY;

	GObject *owner = ac_element_get_owner (_delegate);

	if (!GTK_IS_WIDGET (owner)) {
		GdkRectangle emptyRect;
		emptyRect.x = 0;
		emptyRect.y = 0;
		emptyRect.width = 0;
		emptyRect.height = 0;

		return emptyRect;
	}

	GtkWidget *ownerWidget = GTK_WIDGET (owner);
	GdkRectangle ownerRect = ownerWidget->allocation;

	get_coords_in_window (ownerWidget, &windowX, &windowY);

	ownerRect.x = windowX;
	ownerRect.y = windowY;

	return ownerRect;
}
/*
static void
get_coords_in_window (GtkWidget *widget, int *x, int *y)
{
	*x = widget->allocation.x;
	*y = widget->allocation.y;

	AC_NOTE (LAYOUT, NSLog (@"Allocation for %s - %d, %d", G_OBJECT_TYPE_NAME (widget), *x, *y));

	while ((widget = gtk_widget_get_parent (widget))) {
		BOOL shouldIgnore = NO;
		if (strcmp (G_OBJECT_TYPE_NAME (widget), "__gtksharp_32_MonoDevelop_Components_Docking_DockItemContainer") == 0) {
			shouldIgnore = NO;
		} else
		if (GTK_IS_EVENT_BOX (widget)) {
			GtkBin *bin = GTK_BIN (widget);

			AC_NOTE (LAYOUT, NSLog (@"   Widget is EventBox: %s", G_OBJECT_TYPE_NAME (widget)));

			if (bin->child) {
				if (!gtk_widget_get_has_window (bin->child)) {
					AC_NOTE (LAYOUT, NSLog (@"      Ignoring %s as it does not have a window", G_OBJECT_TYPE_NAME (widget)));
					shouldIgnore = YES;
				} else {
					AC_NOTE (LAYOUT, NSLog (@"      Not ignoring %s as it does have a window", G_OBJECT_TYPE_NAME (widget)));
				}
			}
		} else if (GTK_IS_CONTAINER (widget)) {
			shouldIgnore = YES;
		}

		// Widgets inside a GtkContainer have their allocation in grandparent space
		// So we compensate for that here
		if (!shouldIgnore) {
			*x += widget->allocation.x;
			*y += widget->allocation.y;

			AC_NOTE (LAYOUT, NSLog (@"   Allocation for %s - %d, %d (%d, %d)", G_OBJECT_TYPE_NAME (widget), widget->allocation.x, widget->allocation.y, *x, *y));
		} else {
			AC_NOTE (LAYOUT, NSLog (@"   Container %s - %d, %d", G_OBJECT_TYPE_NAME (widget), widget->allocation.x, widget->allocation.y));
		}
	}
}
*/

static void
get_coords_in_window (GtkWidget *widget, int *x, int *y)
{
	gtk_widget_translate_coordinates (widget, gtk_widget_get_toplevel (widget), 0, 0, x, y);
}

- (id)accessibilityHitTest:(NSPoint) point
{
	NSWindow *parentWindow = [self accessibilityWindow];
	CGRect screenRect = CGRectMake (point.x, point.y, 1, 1);

	CGRect windowRect = [parentWindow convertRectFromScreen:screenRect];
	CGPoint pointInWindow = CGPointMake (windowRect.origin.x, windowRect.origin.y);

	// Flip the y coords to Gtk origin
	CGPoint pointInGtkWindow;
	float halfWindowHeight = [[parentWindow contentView] frame].size.height / 2;
	int dy = pointInWindow.y - halfWindowHeight;

	pointInGtkWindow = CGPointMake (pointInWindow.x, halfWindowHeight - dy);

	for (id<NSAccessibility> nsa in [self accessibilityChildren]) {
		// Handle non-Gtk backed accessibilty elements being children of Gtk ones
		if (![nsa isKindOfClass:[ACAccessibilityElement class]]) {
			NSObject *obj = (NSObject *) nsa;
			CGRect frame = [nsa accessibilityFrame];

			if (CGRectContainsPoint (frame, point)) {
				return [obj accessibilityHitTest:point];
			}

			continue;
		}

		ACAccessibilityElement *e = (ACAccessibilityElement *)nsa;
		if (!AC_IS_ELEMENT (e->_delegate)) {
			NSLog (@"Invalid delegate %p found for %@", e->_delegate, [super description]);
			NSLog (@"If anyone finds this message, please attach the log file it is in to https://bugzilla.xamarin.com/show_bug.cgi?id=56649");
			NSLog (@"And inform iain");
			continue;
		}

		GObject *owner = ac_element_get_owner (e->_delegate);
		if (!GTK_IS_WIDGET (owner)) {
			continue;
		}

		GtkWidget *ownerWidget = GTK_WIDGET (owner);

		if (!gtk_widget_get_visible (ownerWidget) || !gtk_widget_get_realized (ownerWidget)) {
			continue;
		}

		GdkRectangle ownerRect = [e frameInGtkWindowSpace];

		if (pointInGtkWindow.x >= ownerRect.x && pointInGtkWindow.x < ownerRect.x + ownerRect.width &&
			pointInGtkWindow.y >= ownerRect.y && pointInGtkWindow.y < ownerRect.y + ownerRect.height) {
			return [e accessibilityHitTest:point];
		}
	}

	return self;
}

- (NSString *)accessibilityTitle
{
	GObject *owner = ac_element_get_owner (_delegate);

	if (GTK_IS_LABEL (owner) || GTK_IS_BUTTON (owner)) {
		return _realTitle ?: nsstring_from_cstring (ac_element_get_text (_delegate));
	} else {
		return _realTitle;
	}
}

- (void)setAccessibilityTitle:(NSString *)title
{
	if (_realTitle == title) {
		return;
	}

	_realTitle = [title copy];
}

- (id)accessibilityValue
{
	GObject *owner = ac_element_get_owner (_delegate);

	if (GTK_IS_ENTRY (owner)) {
		return nsstring_from_cstring (ac_element_get_text (_delegate));
	}

	if (GTK_IS_TOGGLE_BUTTON (owner)) {
		return gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (owner)) ? @(1) : @(0);
	}

	return nil;
}

- (NSString *)accessibilityIdentifier
{
	const char *name = atk_object_get_name (ATK_OBJECT (_delegate));
	if (name == NULL) {
		name = G_OBJECT_TYPE_NAME (ac_element_get_owner (_delegate));
	}
	NSString *ident = nsstring_from_cstring (name);
	if (_identifier == nil) {
		_identifier = ident;
	}

	return ident;
}

- (NSString *)accessibilityHelp
{
	return nsstring_from_cstring (atk_object_get_description (ATK_OBJECT (_delegate)));
}

/*
- (NSRect)accessibilityFrame
{
	int windowX, windowY;
	GObject *owner;
	GtkWidget *ownerWidget;

	owner = ac_element_get_owner (_delegate);
	if (owner == NULL || !GTK_IS_WIDGET (owner)) {
		return CGRectZero;
	}

	ownerWidget = GTK_WIDGET (owner);
	get_coords_in_window (ownerWidget, &windowX, &windowY);

	NSWindow *parentWindow = [self accessibilityWindow];
	float halfWindowHeight = [[parentWindow contentView] frame].size.height / 2;
	int dy = windowY - halfWindowHeight;

	int cocoaY = halfWindowHeight - dy;
	CGRect windowRect = CGRectMake (windowX, cocoaY, ownerWidget->allocation.width, ownerWidget->allocation.height);

	CGRect screenRect = [parentWindow convertRectToScreen:windowRect];

	return screenRect;
}
*/

static void
ns_role_from_atk (AtkRole atk_role, NSString **ns_role, NSString **ns_subrole)
{
	*ns_role = NSAccessibilityUnknownRole;
	*ns_subrole = NULL;

	switch (atk_role) {
		case ATK_ROLE_INVALID:
		break;

		case ATK_ROLE_ACCEL_LABEL:
			*ns_role = NSAccessibilityStaticTextRole;
			break;

		case ATK_ROLE_ALERT:
			break;

		case ATK_ROLE_ANIMATION:
			break;

		case ATK_ROLE_ARROW:
			break;

		case ATK_ROLE_CALENDAR:
			break;

		case ATK_ROLE_CANVAS:
			*ns_role = NSAccessibilityLayoutAreaRole;
			break;

		case ATK_ROLE_CHECK_BOX:
			*ns_role = NSAccessibilityCheckBoxRole;
			break;

		case ATK_ROLE_CHECK_MENU_ITEM:
			*ns_role = NSAccessibilityMenuItemRole;
			break;

		case ATK_ROLE_COLOR_CHOOSER:
			break;

		case ATK_ROLE_COLUMN_HEADER:
			break;

		case ATK_ROLE_COMBO_BOX:
			*ns_role = NSAccessibilityComboBoxRole;
			break;

		case ATK_ROLE_DATE_EDITOR:
			break;

		case ATK_ROLE_DESKTOP_ICON:
			break;

		case ATK_ROLE_DESKTOP_FRAME:
			break;

		case ATK_ROLE_DIAL:
			*ns_role = NSAccessibilityIncrementorRole;
			break;

		case ATK_ROLE_DIALOG:
			*ns_role = NSAccessibilityWindowRole;
			*ns_subrole = NSAccessibilityDialogSubrole;
			break;

		case ATK_ROLE_DIRECTORY_PANE:
			break;

		case ATK_ROLE_DRAWING_AREA:
			break;

		case ATK_ROLE_FILE_CHOOSER:
			break;

		case ATK_ROLE_FILLER:
			*ns_role = NSAccessibilityGroupRole;
			break;

		case ATK_ROLE_FONT_CHOOSER:
			break;

		case ATK_ROLE_FRAME:
			*ns_role = NSAccessibilityGroupRole;
			break;

		case ATK_ROLE_GLASS_PANE:
			*ns_role = NSAccessibilityGroupRole;
			break;

		case ATK_ROLE_HTML_CONTAINER:
			break;

		case ATK_ROLE_ICON:
		case ATK_ROLE_IMAGE:
			*ns_role = NSAccessibilityImageRole;
			break;

		case ATK_ROLE_INTERNAL_FRAME:
			break;

		case ATK_ROLE_LABEL:
			*ns_role = NSAccessibilityStaticTextRole;
			break;

		case ATK_ROLE_LAYERED_PANE:
			*ns_role = NSAccessibilityGroupRole;
			break;

		case ATK_ROLE_LIST:
			*ns_role = NSAccessibilityListRole;
			break;

		case ATK_ROLE_LIST_ITEM:
			*ns_role = NSAccessibilityCellRole;
			break;

		case ATK_ROLE_MENU:
			*ns_role = NSAccessibilityMenuRole;
			break;

		case ATK_ROLE_MENU_BAR:
			*ns_role = NSAccessibilityMenuBarRole;
			break;

		case ATK_ROLE_MENU_ITEM:
			*ns_role = NSAccessibilityMenuItemRole;
			break;

		case ATK_ROLE_OPTION_PANE:
			*ns_role = NSAccessibilityGroupRole;
			break;

		case ATK_ROLE_PAGE_TAB:
			break;

		case ATK_ROLE_PAGE_TAB_LIST:
			*ns_role = NSAccessibilityTabGroupRole;
			break;

		case ATK_ROLE_PANEL:
			*ns_role = NSAccessibilityGroupRole;
			break;

		case ATK_ROLE_PASSWORD_TEXT:
			*ns_role = NSAccessibilityTextFieldRole;
			*ns_subrole = NSAccessibilitySecureTextFieldSubrole;
			break;

		case ATK_ROLE_POPUP_MENU:
			*ns_role = NSAccessibilityMenuRole;
			break;

		case ATK_ROLE_PROGRESS_BAR:
			*ns_role = NSAccessibilityProgressIndicatorRole;
			break;

		case ATK_ROLE_PUSH_BUTTON:
			*ns_role = NSAccessibilityButtonRole;
			break;

		case ATK_ROLE_RADIO_BUTTON:
			*ns_role = NSAccessibilityRadioButtonRole;
			break;

		case ATK_ROLE_RADIO_MENU_ITEM:
			*ns_role = NSAccessibilityMenuItemRole;
			break;

		case ATK_ROLE_ROOT_PANE:
			*ns_role = NSAccessibilityGroupRole;
			break;

		case ATK_ROLE_ROW_HEADER:
			break;

		case ATK_ROLE_SCROLL_BAR:
			*ns_role = NSAccessibilityScrollBarRole;
			break;

		case ATK_ROLE_SCROLL_PANE:
			*ns_role = NSAccessibilityScrollAreaRole;
			break;

		case ATK_ROLE_SEPARATOR:
			break;

		case ATK_ROLE_SLIDER:
			*ns_role = NSAccessibilitySliderRole;
			break;

		case ATK_ROLE_SPLIT_PANE:
			*ns_role = NSAccessibilitySplitGroupRole;
			break;

		case ATK_ROLE_SPIN_BUTTON:
			*ns_role = NSAccessibilityIncrementorRole;
			break;

		case ATK_ROLE_STATUSBAR:
			*ns_role = NSAccessibilityStaticTextRole;
			break;

		case ATK_ROLE_TABLE:
			*ns_role = NSAccessibilityTableRole;
			break;

		case ATK_ROLE_TABLE_CELL:
			*ns_role = NSAccessibilityCellRole;
			break;

		case ATK_ROLE_TABLE_COLUMN_HEADER:
			break;

		case ATK_ROLE_TABLE_ROW_HEADER:
			break;

		case ATK_ROLE_TEAR_OFF_MENU_ITEM:
			*ns_role = NSAccessibilityMenuItemRole;
			break;

		case ATK_ROLE_TERMINAL:
			break;

		case ATK_ROLE_TEXT:
			*ns_role = NSAccessibilityTextAreaRole;
			break;

		case ATK_ROLE_TOGGLE_BUTTON:
			*ns_role = NSAccessibilityButtonRole;
			*ns_subrole = NSAccessibilityToggleSubrole;
			break;

		case ATK_ROLE_TOOL_BAR:
			*ns_role = NSAccessibilityToolbarRole;
			break;

		case ATK_ROLE_TOOL_TIP:
			break;

		case ATK_ROLE_TREE:
			break;

		case ATK_ROLE_TREE_TABLE:
			*ns_role = NSAccessibilityOutlineRole;
			break;

		case ATK_ROLE_UNKNOWN:
			break;

		case ATK_ROLE_VIEWPORT:
			*ns_role = NSAccessibilityGroupRole;
			break;

		case ATK_ROLE_WINDOW:
			*ns_role = NSAccessibilityWindowRole;
			break;

		case ATK_ROLE_HEADER:
			break;

		case ATK_ROLE_FOOTER:
			break;

		case ATK_ROLE_PARAGRAPH:
			break;

		case ATK_ROLE_RULER:
			break;

		case ATK_ROLE_APPLICATION:
			*ns_role = NSAccessibilityApplicationRole;
			break;

		case ATK_ROLE_AUTOCOMPLETE:
			break;

		case ATK_ROLE_EDITBAR:
			break;

		case ATK_ROLE_EMBEDDED:
			break;

		case ATK_ROLE_ENTRY:
			*ns_role = NSAccessibilityTextFieldRole;
			break;

		case ATK_ROLE_CHART:
			break;

		case ATK_ROLE_CAPTION:
			break;

		case ATK_ROLE_DOCUMENT_FRAME:
			break;

		case ATK_ROLE_HEADING:
			break;

		case ATK_ROLE_PAGE:
			break;

		case ATK_ROLE_SECTION:
			break;

		case ATK_ROLE_REDUNDANT_OBJECT:
			break;

		case ATK_ROLE_FORM:
			break;

		case ATK_ROLE_LINK:
			*ns_role = NSAccessibilityLinkRole;
			break;

		case ATK_ROLE_INPUT_METHOD_WINDOW:
			break;

		case ATK_ROLE_TABLE_ROW:
			break;

		case ATK_ROLE_TREE_ITEM:
			break;

		case ATK_ROLE_DOCUMENT_SPREADSHEET:
			break;

		case ATK_ROLE_DOCUMENT_PRESENTATION:
			break;

		case ATK_ROLE_DOCUMENT_TEXT:
			break;

		case ATK_ROLE_DOCUMENT_WEB:
			break;

		case ATK_ROLE_DOCUMENT_EMAIL:
			break;

		case ATK_ROLE_COMMENT:
			break;

		case ATK_ROLE_LIST_BOX:
			break;

		case ATK_ROLE_GROUPING:
			break;

		case ATK_ROLE_IMAGE_MAP:
			break;

		case ATK_ROLE_NOTIFICATION:
			break;

		case ATK_ROLE_INFO_BAR:
			break;

		case ATK_ROLE_LEVEL_BAR:
			break;

		case ATK_ROLE_LAST_DEFINED:
			break;

		default:
			break;
	}
}

- (NSString *)accessibilityRole
{
	NSString *role, *subrole;

	if (_realRole) {
		return _realRole;
	}

	ns_role_from_atk (atk_object_get_role (ATK_OBJECT (_delegate)), &role, &subrole);
	return role;
}

- (void)setAccessibilityRole:(NSString *)role
{
	_realRole = role;
	[super setAccessibilityRole:role];
}

- (NSString *)accessibilitySubrole
{
	NSString *role, *subrole;

	ns_role_from_atk (atk_object_get_role (ATK_OBJECT (_delegate)), &role, &subrole);

	if (subrole == NULL) {
		return [super accessibilitySubrole];
	} else {
		return subrole;
	}
}

- (CGRect)accessibilityFrameInParentSpace
{
	GObject *owner;
	GtkWidget *ownerWidget;
	gboolean needsParentOffset = FALSE;
	GdkRectangle ownerRect, parentRect;
	id<NSAccessibility> parentElement;
	int x, y, parentX, parentY;
	float halfParentHeight, dy;

	owner = ac_element_get_owner (_delegate);
	if (!GTK_IS_WIDGET (owner)) {
		return CGRectZero;
	}

	ownerWidget = GTK_WIDGET (owner);
	ownerRect = [self frameInGtkWindowSpace];

	// accessibilityFrameInParentSpace refers to the parent in the accessibility tree
	// which may not be the same as the parent in the widget tree.
	// So we need to get the widget of the accessibility parent, and work up from there
	parentElement = [self accessibilityParent];
	if (parentElement == NULL) {
		return CGRectZero;
	}

	if ([parentElement isKindOfClass:[ACAccessibilityElement class]]) {
		ACAccessibilityElement *ep = (ACAccessibilityElement *)parentElement;

		//parentWidget = (GtkWidget *) ac_element_get_owner ([ep delegate]);
		needsParentOffset = TRUE;
		parentRect = [ep frameInGtkWindowSpace];
	} else {
		if ([parentElement isKindOfClass:[NSWindow class]]) {
			NSWindow *window = (NSWindow *)parentElement;
			NSView *contentView = [window contentView];
			parentRect.x = contentView.frame.origin.x;
			parentRect.y = contentView.frame.origin.y;
			parentRect.width = contentView.frame.size.width;
			parentRect.height = contentView.frame.size.height;
		} else if ([parentElement isKindOfClass:[NSView class]]) {
			NSView *view = (NSView *)parentElement;
			parentRect.x = view.frame.origin.x;
			parentRect.y = view.frame.origin.y;
			parentRect.width = view.frame.size.width;
			parentRect.height = view.frame.size.height;
			needsParentOffset = TRUE;
		} else {
			NSLog (@"Parent element is %@ and not supported", parentElement);
			return CGRectZero;
		}
	}

	// Convert both the owner and the parent to window coordinates because it's easier than
	// trying to work out which widgets inbetween need to be ignored and which don't.
	//get_coords_in_window (ownerWidget, &x, &y);
	x = ownerRect.x;
	y = ownerRect.y;

	if (needsParentOffset) {
		//get_coords_in_window (parentWidget, &parentX, &parentY);

		x -= parentRect.x;
		y -= parentRect.y;
	} else {
		// if the parent is the window, then we don't have to care about the parent position
		// as x&y are already in parent space.
		// FIXME: handle non-NSWindow non-Gtk things.
		// for example things might go weird with Cocoa widgets embedded inside GtkWidgets
		// or a GtkWidget embedded in a non-toplevel NSView 
	}

	/* Flip the y coords to match Cocoa */
	halfParentHeight = parentRect.height / 2.0;
	dy = (y + ownerRect.height) - halfParentHeight;

	y = (int)(halfParentHeight - dy);

	return CGRectMake (x, y, ownerRect.width, ownerRect.height);
}

- (BOOL)isAccessibilityEnabled
{
	GObject *owner;
	GtkWidget *ownerWidget;

	owner = ac_element_get_owner (_delegate);
	if (!GTK_IS_WIDGET (owner)) {
		return NO;
	}

	ownerWidget = GTK_WIDGET (owner);
	return gtk_widget_is_sensitive (ownerWidget);
}

- (BOOL)isAccessibilityFocused
{
	GObject *owner;
	GtkWidget *ownerWidget;

	owner = ac_element_get_owner (_delegate);
	if (!GTK_IS_WIDGET (owner)) {
		return NO;
	}

	ownerWidget = GTK_WIDGET (owner);

	return gtk_widget_has_focus (ownerWidget);
}

#pragma mark - Actions

- (NSArray *)accessibilityActionNames
{
	return ac_element_get_actions (_delegate);
}

- (void)accessibilityPerformAction:(NSString *)action
{
	if ([action isEqualTo:NSAccessibilityCancelAction]) {
		ac_element_perform_cancel (_delegate);
	} else if ([action isEqualTo:NSAccessibilityConfirmAction]) {
		ac_element_perform_confirm (_delegate);
	} else if ([action isEqualTo:NSAccessibilityDecrementAction]) {
		ac_element_perform_decrement (_delegate);
	} else if ([action isEqualTo:NSAccessibilityDeleteAction]) {
		ac_element_perform_delete (_delegate);
	} else if ([action isEqualTo:NSAccessibilityIncrementAction]) {
		ac_element_perform_increment (_delegate);
	} else if ([action isEqualTo:NSAccessibilityPickAction]) {
		ac_element_perform_pick (_delegate);
	} else if ([action isEqualTo:NSAccessibilityPressAction]) {
		ac_element_perform_press (_delegate);
	} else if ([action isEqualTo:NSAccessibilityRaiseAction]) {
		ac_element_perform_raise (_delegate);
	} else if ([action isEqualTo:NSAccessibilityShowAlternateUIAction]) {
		ac_element_perform_show_alternate_ui (_delegate);
	} else if ([action isEqualTo:NSAccessibilityShowDefaultUIAction]) {
		ac_element_perform_show_default_ui (_delegate);
	} else if ([action isEqualTo:NSAccessibilityShowMenuAction]) {
		ac_element_perform_show_menu (_delegate);
	}
}

@end
