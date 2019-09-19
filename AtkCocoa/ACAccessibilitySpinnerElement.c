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

@interface ACAccessibilityStepperButtonElement : NSAccessibilityElement <NSAccessibilityButton>

@property (readwrite) SEL action;
@property (readwrite, weak) id target;

@end

@implementation ACAccessibilityStepperButtonElement

@synthesize action;
@synthesize target;

- (BOOL)accessibilityPerformPress
{
    if (action == nil && target == nil) {
        return NO;
    }

    [target performSelector:action];
    return YES;
}

@end
@interface ACAccessibilityStepperElement : NSAccessibilityElement <NSAccessibilityStepper>
@end

@implementation ACAccessibilityStepperElement {
    GtkWidget *_spinButton;
    ACAccessibilityStepperButtonElement *_incrementButton;
    ACAccessibilityStepperButtonElement *_decrementButton;
}

- (instancetype)initWithSpinButton:(GtkSpinButton *)button
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _incrementButton = [[ACAccessibilityStepperButtonElement alloc] init];
    [_incrementButton setAccessibilityRole:NSAccessibilityButtonRole];
    [_incrementButton setAccessibilitySubrole:NSAccessibilityIncrementArrowSubrole];
    [_incrementButton setTarget:self];
    [_incrementButton setAction:@selector(accessibilityPerformIncrement)];
    [self setAccessibilityIncrementButton:_incrementButton];

    _decrementButton = [[ACAccessibilityStepperButtonElement alloc] init];
    [_decrementButton setAccessibilityRole:NSAccessibilityButtonRole];
    [_decrementButton setAccessibilitySubrole:NSAccessibilityDecrementArrowSubrole];
    [_decrementButton setTarget:self];
    [_decrementButton setAction:@selector(accessibilityPerformDecrement)];
    [self setAccessibilityDecrementButton:_decrementButton];

    _spinButton = (GtkWidget *) button;

    return self;
}

- (NSArray *)accessibilityChildren
{
    return @[_incrementButton, _decrementButton];
}

- (void)adjustValueInDirection:(GtkSpinType)direction
{
    gtk_spin_button_spin (GTK_SPIN_BUTTON (_spinButton), direction, 1);
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

#define STEPPER_WIDTH 16 // Don't think there's a way to get the stepper width, so hard code something so VO works
- (NSRect)accessibilityFrameInParentSpace
{
    return NSMakeRect(_spinButton->allocation.width - STEPPER_WIDTH, 0, STEPPER_WIDTH, _spinButton->allocation.height);
}

@end

@implementation ACAccessibilitySpinnerElement {
    ACAccessibilityStepperElement *_stepper;
    ACAccessibilityTextFieldElement *_textField;
}

- (instancetype)initWithDelegate:(AcElement *)delegate
{
    self = [super initWithDelegate:delegate];
    if (self == nil) {
        return nil;
    }

	[self setAccessibilityRole:NSAccessibilityGroupRole];

    _stepper = [[ACAccessibilityStepperElement alloc] initWithSpinButton:GTK_SPIN_BUTTON (ac_element_get_owner(delegate))];
    _textField = [[ACAccessibilityTextFieldElement alloc] initWithDelegate:delegate];

    return self;
}

- (BOOL)hasDynamicChildren
{
    return YES;
}

- (NSArray *)accessibilityChildren
{
    return @[_textField, _stepper];
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

@end
