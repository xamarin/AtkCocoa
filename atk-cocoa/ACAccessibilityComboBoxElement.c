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

#import "ACAccessibilityComboBoxElement.h"
#include "acelement.h"
#include "acdebug.h"

@implementation ACAccessibilityComboBoxElement

- (instancetype)initWithDelegate:(AcElement *)delegate
{
	return [super initWithDelegate:delegate];
}

- (BOOL)accessibilityPerformPress
{
    return ac_element_perform_press ([self delegate]);
}

- (BOOL)accessibilityPerformShowMenu
{
    return ac_element_perform_show_menu ([self delegate]);
}

- (NSString *)accessibilityLabel
{
    return [super accessibilityLabel];
}
@end