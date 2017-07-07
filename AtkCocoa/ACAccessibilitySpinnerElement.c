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

#import "atk-cocoa/ACAccessibilitySpinnerElement.h"
#include "atk-cocoa/acelement.h"
#include "atk-cocoa/acdebug.h"

@implementation ACAccessibilitySpinnerElement {
}

- (instancetype)initWithDelegate:(AcElement *)delegate
{
	[self setAccessibilityRole:NSAccessibilityIncrementorRole];
	return [super initWithDelegate:delegate];
}

- (GtkSpinButton *)owner
{
	GObject *owner;

	owner = ac_element_get_owner ([self delegate]);
	if (!GTK_IS_SPIN_BUTTON (owner)) {
		return NULL;
	}

	return GTK_SPIN_BUTTON (owner);
}

- (NSRect)accessibilityFrame
{
	return [super accessibilityFrame];
}

- (NSString *)accessibilityLabel
{
	return [super accessibilityLabel];
}

- (id<NSAccessibility>)accessibilityParent
{
	return [super accessibilityParent];
}

- (NSNumber *)accessibilityValue
{
	GtkSpinButton *owner = [self owner];
	if (owner == NULL) {
		return nil;
	}

	return [NSNumber numberWithDouble:gtk_spin_button_get_value (owner)];
}

- (void)setAccessibilityValue:(NSNumber *)value
{
	GtkSpinButton *owner = [self owner];
	if (owner == NULL) {
		return;
	}

	gtk_spin_button_set_value (owner, [value doubleValue]);
}

- (void)adjustValueInDirection:(GtkSpinType)direction
{
	GtkSpinButton *owner = [self owner];
	if (owner == NULL) {
		return;
	}
	
	gtk_spin_button_spin (GTK_SPIN_BUTTON (owner), direction, 1);
}

- (BOOL)accessibilityPerformDecrement
{
	[self adjustValueInDirection:GTK_SPIN_STEP_BACKWARD];
	return YES;
}

- (BOOL)accessibilityPerformIncrement
{
	[self adjustValueInDirection:GTK_SPIN_STEP_FORWARD];
	return YES;
}
@end
