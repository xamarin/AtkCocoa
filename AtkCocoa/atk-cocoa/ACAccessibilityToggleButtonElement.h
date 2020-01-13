//
//  ACAccessibilityToggleButtonElement.h
//  AtkCocoa
//
//  Created by iain on 09/01/2020.
//  Copyright © 2020 Microsoft. All rights reserved.
//

#import "ACAccessibilityElement.h"

NS_ASSUME_NONNULL_BEGIN

@interface ACAccessibilityToggleButtonElement : ACAccessibilityElement

@property (readwrite, weak) id<NSAccessibility> comboBox;

@end

NS_ASSUME_NONNULL_END
