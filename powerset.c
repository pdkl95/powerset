#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

/* compatability with missing stdbool.h */
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef __bool_true_false_are_defined
#  ifndef HAVE__BOOL
#   ifdef __cplusplus
typedef bool _Bool;
#   else
#    define _Bool signed char
#   endif
#  endif
#  define bool _Bool
#  define false 0
#  define true 1
#  define __bool_true_false_are_defined 1
# endif
#endif

/* constants */
#ifdef UINT_MAX
# if UINT_MAX >= 2147483648
#  define MAX_ITEM_LIMIT 31
# else
#  error "UINT_MAX < 2147483648"
# endif
#else
# define MAX_ITEM_LIMIT 31
#endif

/* options */
bool print_empty_set    = true;
bool print_array_format = false;
bool use_item_limit     = false;
bool use_sum_filter     = false;

/* global state */
int  max_items    = 0;
int  required_sum = 0;

char *progname = NULL;

enum item_type {
    ITEM_TYPE_STRING,
    ITEM_TYPE_INT
};
typedef enum item_type item_type_t;

item_type_t item_mode = ITEM_TYPE_STRING;

#define ITEM_IS_STRING (item_mode == ITEM_TYPE_STRING)
#define ITEM_IS_INT    (item_mode == ITEM_TYPE_INT)

/* out-of-memory checking wrapper for malloc() */
void *
MALLOC(
    size_t size
) {
    void *ptr = NULL;

    if (size > 0) {
        ptr = malloc(size);
        if (NULL == ptr) {
            abort();
        }
    }

    return ptr;
}

bool
string_is_integer(
    char *str
) {
    for (; *str; str++) {
        int c = str[0];

        if (!isdigit(c)) {
            return false;
        }
    }

    return true;
}

/*************************************************************************
 * One item in the set.
 *
 * These can be either strings or intgs.
 */

struct item_string {
    char *value;
    bool owner;
};
typedef struct item_string item_string_t;

struct item_int {
    int value;
};
typedef struct item_int item_int_t;

union item {
    item_string_t s;
    item_int_t    i;
};
typedef union item item_t;

#define ITEM_INT(item) (&((item)->i))
#define ITEM_STRING(item) (&((item)->s))

void
item_clear(
    item_t *item
) {
    if (ITEM_IS_STRING) {
        item->s.value = NULL;
        item->s.owner = false;
    } else {
        item->i.value = 0;
    }
}

static void
item_free_string(
    item_string_t *item
) {
    if (item) {
        if (item->value && item->owner) {
            free(item->value);
        }
    }
}

void
item_free(
    item_t *item
) {
    if (ITEM_IS_STRING) {
        item_free_string(ITEM_STRING(item));
    }
}

item_t *
create_item_array(
    int num_items
) {
    int i;
    item_t *items;

    if (num_items < 1) {
        printf("create_itemset(%d) is invalid (num_items must be > 0)", num_items);
        exit(EXIT_FAILURE);
    }

    items = MALLOC(sizeof(item_t) * num_items);

    for (i = 0; i < num_items; i++) {
        item_t *item = &(items[i]);
        item_clear(item);
    }

    return items;
}

void
destroy_item_array(
    item_t *items,
    int num_items
) {
    int i;

    if (items) {
        if (ITEM_IS_STRING) {
            for (i = 0; i < num_items; i++) {
                item_t *item = &(items[i]);
                item_free(item);
            }
        }

        free(items);
    }
}

static bool
item_string_ref_string(
    item_string_t *item,
    char *str
) {
    item_free_string(item);
    item->value = str;
    item->owner = false;
    return true;
}

static bool
item_string_set_from_string(
    item_string_t *item,
    char *str
) {
    item_free_string(item);
    item->value = strdup(str);
    item->owner = true;
    return true;
}

static bool
item_int_set_from_string(
    item_int_t *item,
    char *str
) {
    if (string_is_integer(str)) {
        item->value = atoi(str);
        return true;
    } else {
        return false;
    }
}

bool
item_set_from_string(
    item_t *item,
    char *str
) {
    if (ITEM_IS_STRING) {
        return item_string_set_from_string(ITEM_STRING(item), str);
    } else {
        return item_int_set_from_string(ITEM_INT(item), str);
    }

    return true;
}

static void
item_string_ref_copy_item(
    item_string_t *dst,
    item_string_t *src
) {
    item_string_ref_string(dst, src->value);
}

static void
item_int_ref_copy_item(
    item_int_t *dst,
    item_int_t *src
) {
    dst->value = src->value;
}

void
item_ref_copy_item(
    item_t *dst,
    item_t *src
) {
    if (ITEM_IS_STRING) {
        item_string_ref_copy_item(ITEM_STRING(dst),
                                  ITEM_STRING(src));
    } else {
        item_int_ref_copy_item(ITEM_INT(dst),
                               ITEM_INT(src));
    }
}

static void
print_item_string(
    item_string_t *item
) {
    if (item->value) {
        if (print_array_format) {
            printf("\"%s\"", item->value);
        } else {
            printf("%s", item->value);
        }
    } else {
        if (print_array_format) {
            printf("NULL");
        } else {
            printf("(null)");
        }
    }
}


static void
print_item_int(
    item_int_t *item
) {
    printf("%d", item->value);
}

void
print_item(
    item_t *item
) {
    if (item) {
        if (ITEM_IS_STRING) {
            print_item_string(ITEM_STRING(item));
        } else {
            print_item_int(ITEM_INT(item));
        }
    }
}

/*************************************************************************
 * a set of items
 */
struct itemset {
    int num_items;
    item_t *items;
};
typedef struct itemset itemset_t;

itemset_t *
create_itemset(
    int num_items
) {
    itemset_t *iset;

    if (num_items < 1) {
        printf("create_itemset(%d) is invalid (num_items must be > 0)", num_items);
        exit(EXIT_FAILURE);
    }

    iset = MALLOC(sizeof(itemset_t));

    iset->num_items = num_items;
    iset->items     = create_item_array(num_items);

    return iset;
}

void
destroy_itemset(
    itemset_t *iset
) {
    if (iset) {
        if (iset->items) {
            destroy_item_array(iset->items, iset->num_items);
            iset->items = NULL;
        }

        free(iset);
    }
}

itemset_t *
create_itemset_argv(
    int argc,
    char *argv[]
) {
    int i;
    itemset_t *iset = NULL;

    iset = create_itemset(argc);

    for (i=0; i < iset->num_items; i++) {
        char   *arg  = argv[i];
        item_t *item = &(iset->items[i]);

        if (!item_set_from_string(item, arg)) {
            printf("argument %d \"%s\" is not an integer", i, arg);
            destroy_itemset(iset);
            return NULL;
        }
    }
    
    return iset;
}

void
print_itemset(
    itemset_t *iset
) {
    int i;

    if (print_array_format) {
        printf("[");
    }

    for (i = 0; i < iset->num_items; i++) {
        if (i > 0) {
            if (print_array_format) {
                printf(", ");
            } else {
                printf(" ");
            }
        }

        print_item(&(iset->items[i]));
    }
    
    if (print_array_format) {
        printf("]");
    }
}

void
print_itemset_nl(
    itemset_t *iset
) {
    print_itemset(iset);
    printf("\n");
}


/*************************************************************************
 * power sets
 */

bool
map_bits_to_itemset(
    unsigned int bits,
    itemset_t *src,
    itemset_t *dst
) {
    int src_i = 0;
    int dst_i = 0;

    int sum = 0;

    while (bits) {
        if (bits & 1) {
            item_t *d = &(dst->items[dst_i]);
            item_t *s = &(src->items[src_i]);

            item_ref_copy_item(d, s);

            if (use_sum_filter) {
                sum += d->i.value;
            }

            dst_i++;
        }

        bits = bits >> 1;

        src_i++;
    }

    dst->num_items = dst_i;

    if (use_sum_filter && (sum != required_sum)) {
        return false;
    } else {
        return true;
    }
}

/*
 * From:
 * https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
 */
int
count_bits(
    unsigned int v
) {
    unsigned int c;
    static const unsigned int B[] = {
        0x55555555,   // 01010101 01010101 01010101 01010101
        0x33333333,   // 00110011 00110011 00110011 00110011
        0x0F0F0F0F,   // 00001111 00001111 00001111 00001111
        0x00FF00FF,   // 00000000 11111111 00000000 11111111
        0x0000FFFF    // 00000000 00000000 11111111 11111111
    };

    c = v - ((v >> 1) & B[0]);
    c = ((c >>  2) & B[1]) + (c & B[1]);
    c = ((c >>  4) + c) & B[2];
    c = ((c >>  8) + c) & B[3];
    c = ((c >> 16) + c) & B[4];

    return (int)c;
}

void
print_powerset(
    itemset_t *iset
) {
    itemset_t *subset = create_itemset(iset->num_items);

    unsigned int bits_start = print_empty_set ? 0 : 1;
    unsigned int bits_stop  = 1 << iset->num_items;
    unsigned int bits;

    for (bits = bits_start; bits < bits_stop; bits++) {
        if (use_item_limit) {
            if (count_bits(bits) > max_items) {
                continue;
            }
        }

        if (map_bits_to_itemset(bits, iset, subset)) {
            print_itemset_nl(subset);
        }
    }

    destroy_itemset(subset);
}


/*************************************************************************
 * main program / option parsing
 */

#define NEXT_ARG \
    do {         \
        argc--;  \
        argv++;  \
    } while(0)

#define REQUIRE_ADDITIONAL_ARG(argname)                  \
    if (argc < 1) {                                      \
        printf("option \"-%s\" requires a parameter\n",  \
               argname);                                 \
        return false;                                    \
    }

#define REQUIRE_ADDITIONAL_INT_ARG(argname)              \
    REQUIRE_ADDITIONAL_ARG(argname);                     \
    if (!string_is_integer(*argv)) {                     \
        printf("option \"-%s\" expects an integer\n",    \
               argname);                                 \
        return false;                                    \
    }

bool
parse_options(
    int   *argcp,
    char **argvp[]
) {
    int    argc = *argcp;
    char **argv = *argvp;

    /* move past the program name */
    NEXT_ARG;

    while ((argc > 0)) {
        char *arg = *argv;

        /* printf("argc = %d, *arg = \"%s\"\n", argc, arg); */

        if ('-' == arg[0]) {
            NEXT_ARG;

            switch (arg[1]) {
            case 'a':
                print_array_format = true;
                break;

            case 'E':
                print_empty_set = false;
                break;

            case 'm':
                REQUIRE_ADDITIONAL_INT_ARG("m");
                use_item_limit = true;
                max_items = atoi(*argv);
                if (max_items > MAX_ITEM_LIMIT) {
                    printf("Only item limits up to %d are supported",
                           MAX_ITEM_LIMIT);
                    return false;
                }
                NEXT_ARG;
                break;

            case 's':
                REQUIRE_ADDITIONAL_INT_ARG("s");
                use_sum_filter = true;
                required_sum = atoi(*argv);
                item_mode = ITEM_TYPE_INT;
                NEXT_ARG;
                break;

            default:
                printf("bad arg: \"%s\"\n", arg);
                return false;
            }
        } else {
            break;
        }
    }

    *argcp = argc;
    *argvp = argv;

    return true;
}

int
main(
    int argc,
    char *argv[]
) {
    itemset_t *iset = NULL;

    progname = argv[0];

    if (!parse_options(&argc, &argv)) {
        printf("error parsing options");
        return EXIT_FAILURE;
    }

    iset = create_itemset_argv(argc, argv);
    if (NULL == iset) {
        return EXIT_FAILURE;
    }

    /* printf("Initial Array: "); */
    /* print_itemset_nl(iset); */

    print_powerset(iset);

    return EXIT_SUCCESS;
}
