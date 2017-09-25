//
//  bb_algorithm.h
//  c-libbb
//
//  Created by OTAKE Takayoshi on 2017/09/25.
//  Copyright © 2017 OTAKE Takayoshi. All rights reserved.
//

#ifndef bb_algorithm_h
#define bb_algorithm_h

#include "bb_types.h"

// array

#define BB_countof(_x) \
sizeof(_x)/sizeof(*(_x))

#define BB_beginof(_Type, _x) \
((_Type*)_x)

#define BB_endof(_Type, _x) \
((_Type*)_x + BB_countof(_x))

// list

#define BB_list_first(_Type, _begin, _end, _first, _Match, _arg) \
do { \
    _Type* _ptr; \
    for (_ptr = _begin; _ptr < _end; ++_ptr) { \
        if (_Match(_ptr, _arg)) { \
            *_first = _ptr; \
            break; \
        } \
    } \
} while (0)

#define BB_list_filter(_Type, _begin, _end, _filtered, _Match, _arg) \
do { \
    _Type* _ptr; \
    for (_ptr = _begin; _ptr < _end; ++_ptr) { \
        if (_Match(_ptr, _arg)) { \
            *_filtered = *_ptr; \
            ++_filtered; \
        } \
    } \
} while (0)

#endif /* bb_algorithm_h */
