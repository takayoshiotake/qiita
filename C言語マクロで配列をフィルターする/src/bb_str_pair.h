//
//  bb_str_pair.h
//  c-libbb
//
//  Created by OTAKE Takayoshi on 2017/09/24.
//  Copyright © 2017 OTAKE Takayoshi. All rights reserved.
//

#ifndef bb_str_pair_h
#define bb_str_pair_h

#include <stdint.h>

#include "bb_types.h"

typedef struct {
    char const* p0;
    char const* p1;
} bb_str_pair_t;

typedef bb_bool_t(*bb_str_pair_match_t)(bb_str_pair_t const*, void const*);

extern char const* bb_str_pair_get(bb_str_pair_t const* pair, int index);

extern bb_bool_t bb_str_pair_p0_equal(bb_str_pair_t const*, char const*);
extern bb_bool_t bb_str_pair_p1_equal(bb_str_pair_t const*, char const*);
extern bb_bool_t bb_str_pair_equal(bb_str_pair_t const*, bb_str_pair_t const*);

/**
 Specialization of BB_list_first for bb_str_pair_t.
 
 - returns:
     - An first object equals by `equal` with `rhs`.
     - NULL when the object not found.
 
 - parameters:
     - begin: The begin of the list of bb_str_pair_t.
     - end: The end of the list of bb_str_pair_t.
     - match: The match function for bb_str_pair_t. Arguments are one of the list and `arg`.
     - arg: The argument will be used by `match`.
 */
extern bb_str_pair_t const* bb_str_pair_list_first(bb_str_pair_t const* begin, bb_str_pair_t const* end, bb_str_pair_match_t match, void const* arg);

#endif /* bb_str_pair_h */
