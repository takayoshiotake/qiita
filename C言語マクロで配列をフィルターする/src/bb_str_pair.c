//
//  bb_str_pair.c
//  c-libbb
//
//  Created by OTAKE Takayoshi on 2017/09/24.
//  Copyright © 2017 OTAKE Takayoshi. All rights reserved.
//

#include "bb_str_pair.h"

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "bb_types.h"
#include "bb_algorithm.h"

char const* bb_str_pair_get(bb_str_pair_t const* pair, int index) {
    assert(pair);
    assert(index == 0 || index == 1);
    if (index == 0) {
        return pair->p0;
    }
    else if (index == 1) {
        return pair->p1;
    }
    return NULL;
}

bb_bool_t bb_str_pair_p0_equal(bb_str_pair_t const* lhs, char const* rhs) {
    assert(lhs);
    if (rhs == NULL) {
        return lhs->p0 == NULL;
    }
    return strcmp(lhs->p0, rhs) == 0 ? bb_true : bb_false;
}

bb_bool_t bb_str_pair_p1_equal(bb_str_pair_t const* lhs, char const* rhs) {
    assert(lhs);
    if (rhs == NULL) {
        return lhs->p0 == NULL;
    }
    return strcmp(lhs->p1, rhs) == 0 ? bb_true : bb_false;
}

bb_bool_t bb_str_pair_equal(bb_str_pair_t const* lhs, bb_str_pair_t const* rhs) {
    assert(lhs);
    assert(rhs);
    if (!bb_str_pair_p0_equal(lhs, rhs->p0)) {
        return bb_false;
    }
    if (!bb_str_pair_p1_equal(lhs, rhs->p1)) {
        return bb_false;
    }
    return bb_true;
}

bb_str_pair_t const* bb_str_pair_list_first(bb_str_pair_t const* begin, bb_str_pair_t const* end, bb_str_pair_match_t match, void const* arg) {
    bb_str_pair_t const* first = NULL;
    assert(begin);
    assert(end);
    assert(match);
    BB_list_first(bb_str_pair_t const, begin, end, &first, match, arg);
    return first;
}
