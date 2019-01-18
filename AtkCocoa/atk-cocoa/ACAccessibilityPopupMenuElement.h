//
//  ACAccessibilityPopupMenuElement.h
//  AtkCocoa
//
//  Created by iain on 16/01/2019.
//  Copyright © 2019 Microsoft. All rights reserved.
//

#import "ACAccessibilityElement.h"

NS_ASSUME_NONNULL_BEGIN

@interface ACAccessibilityPopupMenuElement : ACAccessibilityElement <NSAccessibilityButton>

- (instancetype)initWithDelegate:(AcElement *)delegate;

@end

NS_ASSUME_NONNULL_END
