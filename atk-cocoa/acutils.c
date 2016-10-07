#include "acutils.h"
#import <Foundation/Foundation.h>

NSString *
nsstring_from_cstring (const char *cstring)
{
	if (cstring == NULL) {
		return nil;
	}
	return [[NSString alloc] initWithCString:cstring encoding:NSUTF8StringEncoding];
}