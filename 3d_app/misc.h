#pragma once

#ifndef NO_CC_ASSIGN
#define NO_CC_ASSIGN(type_name) \
    type_name(const type_name&);    \
    type_name& operator=(const type_name&)
#endif