#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

int num_items;
int *items;
int print_array_format = 0;

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
int

string_is_integer(
    char *str
) {
    int rv = 1;

    for (; *str; str++) {
        int c = str[0];
        rv = rv && isdigit(c);
    }

    return rv;
}


struct intset {
    int num_items;
    int *items;
};
typedef struct intset intset_t;

intset_t *
create_intset(int num_items)
{
    int i;
    intset_t *iset;

    if (num_items < 1) {
        printf("create_intset(%d) is invalid (num_items must be > 0)", num_items);
        exit(EXIT_FAILURE);
    }

    iset = MALLOC(sizeof(intset_t));

    iset->num_items = num_items;
    iset->items     = MALLOC(sizeof(int) * num_items);

    for (i=0; i < iset->num_items; i++) {
        iset->items[i] = 0;
    }

    return iset;
}

void
destroy_intset(
    intset_t *iset
) {
    if (iset) {
        if (iset->items) {
            free(iset->items);
            iset->items = NULL;
        }

        free(iset);
    }
}

intset_t *
create_intset_argv(
    int argc,
    char *argv[]
) {
    int i;
    intset_t *iset = NULL;

    /* move past the program name */
    argc--;
    argv++;

    iset = create_intset(argc);

    for (i=0; i < iset->num_items; i++) {
        char *arg = argv[i];

        if (!string_is_integer(arg)) {
            printf("argument %d \"%s\" is not an integer", i, arg);
            destroy_intset(iset);
            return NULL;
        }
        
        iset->items[i] = atoi(argv[i]);
    }
    
    return iset;
}

void
print_intset(
    intset_t *iset
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
        printf("%d", iset->items[i]);
    }
    
    if (print_array_format) {
        printf("]");
    }
}

void
print_intset_nl(
    intset_t *iset
) {
    print_intset(iset);
    printf("\n");
}

void
map_bits_to_intset(
    unsigned int bits,
    intset_t *src,
    intset_t *dst
) {
    int src_i = 0;
    int dst_i = 0;

    while (bits) {
        if (bits & 1) {
            dst->items[dst_i] = src->items[src_i];
            dst_i++;
        }

        bits = bits >> 1;
        src_i++;
    }

    dst->num_items = dst_i;
}

void
print_powerset(
    intset_t *iset
) {
    intset_t *subset = create_intset(iset->num_items);

    unsigned int max = 1 << iset->num_items;
    unsigned int bits;

    for (bits=0; bits < max; bits++) {
        map_bits_to_intset(bits, iset, subset);
        print_intset_nl(subset);
    }

    destroy_intset(subset);
}


int
main(
    int argc,
    char *argv[]
) {
    intset_t *iset = create_intset_argv(argc, argv);
    if (NULL == iset) {
        return EXIT_FAILURE;
    }

    /* printf("Initial Array: "); */
    /* print_intset_nl(iset); */

    print_powerset(iset);

    return EXIT_SUCCESS;
}
