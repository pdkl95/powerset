#!/bin/sh

########################
###  Default Values  ###
########################

max_items=
required_sum=
skip_final_sort=false
skip_empty_set=false


###################################################
###  Command line processing and documentation  ###
###################################################

show_help()
{
    echo "usage: $0 [options] <item1> [<item2> [...]]"
    echo
    echo "OPTIONS"
    echo "    -s, --sum <integer>     Only output subsets that sum to this value"
    echo "    -m, --max <maxitems>    Only output subsets with at MOST this many items"
    echo
    echo "    --skip-sort             Skip the final call to sort(1)"
    echo "    --skip-empty-set        Suppress the blank line of the empty set"
    echo
    echo "EXAMPLE"
    echo "    # basic usage, slow"
    echo "    $0 --sum 8 \$(seq 11)"
    echo
    echo "    # note that this gives the same output but is"
    echo "    # much faster as many subsets are filtered early"
    echo "    $0 --sum 8 --max 3 \$(seq 11)"
    echo
    echo "    # this can be even faster with smaller limits"
    echo "    $0 --sum 8 --max 2 \$(seq 11)"
}

if command -v getopt 2>&1 >/dev/null ; then
    # have GNU getopt (allows nicer options)
    SOPT="hm:s:"
    LOPT="help,skip-final-sort,skip-empty-setmax:,sum:"
    OPTIONS=$(getopt -o "$SOPT" --long "$LOPT" -n "$SELFNAME" -- "$@") || exit 1
    eval set -- "$OPTIONS"
fi

while true ; do
    case "$1" in
        -h | --help)       show_help             ;  exit 0 ;;
        -m | --max)        max_items="$2"        ; shift 2 ;;
        -s | --sum)        required_sum="$2"     ; shift 2 ;;
        --skip-final-sort) skip_final_sort=true  ; shift   ;;
        --skip-empty-set)  skip_empty_set=true   ; shift   ;;
        --) shift ; break ;;
        -*) echo "bad option: $1" ; exit 1 ;;
        *)  break ;;
    esac
done


################################
###  Power Set search space  ###
################################

basic_powerset()
{
    if [ $# -eq 0 ] ; then
        echo
    else
        (   shift
            basic_powerset "$@"
        ) | while read r; do
            echo "$1 $r"
            echo "$r"
        done
    fi
}

powerset_without_empty_set()
{
    basic_powerset "$@" | egrep -v "^$"
}

powerset()
{
    if $skip_empty_set ; then
        powerset_without_empty_set "$@"
    else
        basic_powerset "$@"
    fi
}

###################################
###  Filter - Maximum Set Size  ###
###################################

set_leq_max_items()
{
    [ $# -le $max_items ]
}

filter_long_sets()
{
    if [ -z "$max_items" ] ; then
        cat
    else
        while read line ; do
            if set_leq_max_items $line ; then
                echo "$line"
            fi
        done
    fi
}


#################################
###  Filter - Arithmetic Sum  ###
#################################

# prints the sum of it's arguments
sum()
{
    x=0

    while [ $# -gt 0 ] ; do
        i=$1
        shift

        x=$((x+i))
    done

    echo $x
}

# tests if a set's sum matches the required sum
set_matches_sum()
{
    cur=$(sum "$@")
    [ $cur -eq $required_sum ]
}

# filters sets on STDIN, printing only 
# the sets the match the required sum
filter_by_sum()
{
    if [ -z "$required_sum" ] ; then
        cat
    else
        while read line ; do
            if set_matches_sum $line ; then
                echo "$line"
            fi
        done
    fi
}

#############################################
###  Building and formatting the resutls  ###
#############################################

unsorted_matching_sets()
{
    powerset "$@" | filter_long_sets | filter_by_sum
}

sorted_matching_sets()
{
    unsorted_matching_sets "$@" | sort -n
}

if $skip_final_sort ; then
    unsorted_matching_sets "$@"
else
    sorted_matching_sets "$@"
fi


