//
//  ACAccessibilityExpanderElement.m
//  AtkCocoa
//
//  Created by iain on 10/01/2019.
//  Copyright © 2019 Microsoft. All rights reserved.
//

#import "atk-cocoa/ACAccessibilityExpanderElement.h"

@implementation ACAccessibilityExpanderElement

- (NSString *)accessibilityRole
{
    return NSAccessibilityDisclosureTriangleRole;
}

- (NSString *)accessibilitySubrole
{
    return nil;
}

- (BOOL)isAccessibilityExpanded
{
    GtkExpander *expander = GTK_EXPANDER(ac_element_get_owner([self delegate]));
    if (expander == NULL) {
        return NO;
    }

    return gtk_expander_get_expanded(expander);
}

- (void)setAccessibilityExpanded:(BOOL)accessibilityExpanded
{
    GtkExpander *expander = GTK_EXPANDER(ac_element_get_owner([self delegate]));
    if (expander == NULL) {
        return;
    }

    gtk_expander_set_expanded(expander, accessibilityExpanded);
}

- (id)accessibilityValue
{
    return @([self isAccessibilityExpanded]);
}

- (BOOL)accessibilityPerformPress
{
    GtkExpander *expander = GTK_EXPANDER(ac_element_get_owner([self delegate]));
    gtk_expander_set_expanded(expander, !gtk_expander_get_expanded(expander));

    return YES;
}

- (id)accessibilityTitleUIElement
{
    GtkExpander *expander = GTK_EXPANDER(ac_element_get_owner([self delegate]));
    GtkWidget *label = gtk_expander_get_label_widget(expander);

    if (label == NULL) {
        return nil;
    }

    AcElement *labelElement = AC_ELEMENT (gtk_widget_get_accessible(label));
    return ac_element_get_accessibility_element(labelElement);
}
@end
