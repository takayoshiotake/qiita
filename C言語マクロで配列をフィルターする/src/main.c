//
//  main.c
//  c-libbb
//
//  Created by OTAKE Takayoshi on 2017/09/23.
//  Copyright © 2017 OTAKE Takayoshi. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "bb_types.h"
#include "bb_algorithm.h"
#include "bb_str_pair.h"

static int test_first(int* lhs, void const* arg) {
    return *lhs >= *(int*)arg ? 1 : 0;
}

static int test_apple_filter(char const** lhs, void const* arg) {
    return strstr(*lhs, "apple") != NULL ? 1 : 0;
}

static bb_str_pair_t const fruits_colors_list_[] = {
    {"apple", "red"},
    {"banana", "yellow"},
    {"cherry", "#DE3163"},
    {"dekopon", "orange"},
    {"eggplant", "dark purple"},
};
static bb_str_pair_t const* fruits_colors_list_begin_ = BB_beginof(bb_str_pair_t, fruits_colors_list_);
static bb_str_pair_t const* fruits_colors_list_end_ = BB_endof(bb_str_pair_t, fruits_colors_list_);

static bb_bool_t test_copy_filter(bb_str_pair_t const* str_pair, void const* arg) {
    return str_pair->p1[0] != '#' ? bb_true : bb_false;
}

static bb_bool_t test_ptr_filter1(bb_str_pair_t const** str_pair_ptr, void const* arg) {
    return (*str_pair_ptr)->p1[0] != '#' ? bb_true : bb_false;
}

static bb_bool_t test_ptr_filter2(bb_str_pair_t const** str_pair_ptr, void const* arg) {
    return strlen((*str_pair_ptr)->p0) <= 7 ? bb_true : bb_false;
}

int main(int argc, const char * argv[]) {
    printf("Test BB_list_first:\n");
    {
        int list[5] = { 2, 3, 5, 7, 11 };
        int* first = NULL;
        int arg = 5;
        BB_list_first(int, &list[0], &list[sizeof(list)/sizeof(*list)], &first, test_first, &arg);
        if (first) {
            printf("  index: %d\n", (int)(first - list));
        }
    }
    
    printf("Test BB_list_filter:\n");
    {
        char const* list[4] = { "I have a pen", "I have an apple", "ugh", "apple pen" };
        char const* filtered[4];
        char const** filtered_end = filtered;
        BB_list_filter(char const*, &list[0], &list[sizeof(list)/sizeof(*list)], filtered_end, test_apple_filter, NULL);
        {
            char const** ptr;
            for (ptr = filtered; ptr < filtered_end; ++ptr) {
                printf("  %s\n", *ptr);
            }
        }
    }
    
    printf("Test bb_str_pair_lis_first:\n");
    {
        int index = 0;
        char const* key = "cherry";
        bb_str_pair_t const* found;
        
        assert(index == 0 || index == 1);
        
        found = bb_str_pair_list_first(fruits_colors_list_begin_, fruits_colors_list_end_, (bb_str_pair_match_t)(index == 0 ? bb_str_pair_p0_equal : bb_str_pair_p1_equal), key);
        if (found) {
            printf("  %s => %s\n", key, bb_str_pair_get(found, 1 - index));
        }
        else {
            printf("Not found\n");
        }
    }
    
    printf("Test BB_list_filter (copy):\n");
    {
        bb_str_pair_t filtered[fruits_colors_list_end_ - fruits_colors_list_begin_];
        bb_str_pair_t* filtered_end = filtered;
        BB_list_filter(bb_str_pair_t const, fruits_colors_list_begin_, fruits_colors_list_end_, filtered_end, test_copy_filter, NULL);
        {
            bb_str_pair_t const* ptr;
            for (ptr = filtered; ptr < filtered_end; ++ptr) {
                printf("  {%s, %s}\n", ptr->p0, ptr->p1);
            }
        }
    }
    
    printf("Test BB_list_filter (ptr):\n");
    {
        bb_str_pair_t const* ptr_list[fruits_colors_list_end_ - fruits_colors_list_begin_];
        bb_str_pair_t const** ptr_list_begin = BB_beginof(bb_str_pair_t const*, ptr_list);
        bb_str_pair_t const** ptr_list_end = BB_endof(bb_str_pair_t const*, ptr_list);
        
        bb_str_pair_t const** filtered_end_1 = ptr_list;
        bb_str_pair_t const** filtered_end_2 = ptr_list;
        bb_str_pair_t const** filtered_end_3 = ptr_list;
        
        // Initialize `ptr_list`; `ptr_list` includes references to all items of `fruits_colors_list_` at this point.
        {
            bb_str_pair_t const** ptr;
            bb_str_pair_t const* ptr_src;
            for (ptr = ptr_list_begin, ptr_src = fruits_colors_list_begin_; ptr < ptr_list_end; ++ptr, ++ptr_src) {
                *ptr = ptr_src;
            }
        }
        
        BB_list_filter(bb_str_pair_t const*, ptr_list_begin, ptr_list_end, filtered_end_1, test_ptr_filter1, NULL);
        printf("  filterd1:\n");
        {
            bb_str_pair_t const** ptr;
            for (ptr = ptr_list; ptr < filtered_end_1; ++ptr) {
                printf("    {%s, %s}\n", (*ptr)->p0, (*ptr)->p1);
            }
        }
        
        BB_list_filter(bb_str_pair_t const*, ptr_list_begin, filtered_end_1, filtered_end_2, test_ptr_filter2, NULL);
        printf("  filterd2:\n");
        {
            bb_str_pair_t const** ptr;
            for (ptr = ptr_list; ptr < filtered_end_2; ++ptr) {
                printf("    {%s, %s}\n", (*ptr)->p0, (*ptr)->p1);
            }
        }
        
        // HACK: Manualy filter
        {
            bb_str_pair_t const** ptr;
            for (ptr = ptr_list_begin; ptr < filtered_end_2; ++ptr) { \
                if (strcmp((*ptr)->p0, "apple") != 0) {
                    *filtered_end_3 = *ptr;
                    ++filtered_end_3;
                }
            }
        }
        printf("  filterd3:\n");
        {
            bb_str_pair_t const** ptr;
            for (ptr = ptr_list; ptr < filtered_end_3; ++ptr) {
                printf("    {%s, %s}\n", (*ptr)->p0, (*ptr)->p1);
            }
        }
    }
    
    return 0;
}
