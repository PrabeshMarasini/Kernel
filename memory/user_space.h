#ifndef USER_SPACE_H
#define USER_SPACE_H

#include "common.h"

VirtualMemory allocate_user_space();
VirtualMemory* allocate_multiple_user_spaces(size_t count);

#endif // USER_SPACE_H
