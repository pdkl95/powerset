#!/bin/sh

max=
sum=
dosort=true

show_help()
{
    echo "usage: $0 [options] <item1> [<item2> [...]]"
    echo
    echo "OPTIONS"
    echo "    -s, --sum <integer>     Only output subsets that sum to this value"
    echo "    -m, --max <maxitems>    Only output subsets with at MOST this many items"
    echo
    echo "    -n, --nosort            Skip the final call to sort(1)"
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
    SOPT="hnm:s:"
    LOPT="help,nosort,max:,sum:"
    OPTIONS=$(getopt -o "$SOPT" --long "$LOPT" -n "$SELFNAME" -- "$@") || exit 1
    eval set -- "$OPTIONS"
fi

while true ; do
    case "$1" in
        -h | --help)   show_help    ;  exit 0 ;;
        -m | --max)    max="$2"     ; shift 2 ;;
        -s | --sum)    sum="$2"     ; shift 2 ;;
        -n | --nosort) dosort=false ; shift   ;;
        --) shift ; break ;;
        -*) echo "bad option: $1" ; exit 1 ;;
        *)  break ;;
    esac
done


powerset()
{
    if [ $# -eq 0 ] ; then
        echo
    else
        (   shift
            powerset "$@"
        ) | while read r; do
            echo "$1 $r"
            echo "$r"
        done
    fi
}

powerset_nonempty()
{
    powerset "$@" | egrep -v "^$"
}

set_leq_max_items()
{
    [ $# -le $max ]
}

filter_long_sets()
{
    if [ -z "$max" ] ; then
        cat
    else
        while read line ; do
            if set_leq_max_items $line ; then
                echo "$line"
            fi
        done
    fi
}

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

set_matches_sum()
{
    cur=$(sum "$@")
    [ $cur -eq $sum ]
}

filter_by_sum()
{
    if [ -z "$sum" ] ; then
        cat
    else
        while read line ; do
            if set_matches_sum $line ; then
                echo "$line"
            fi
        done
    fi
}


filtered_powerset()
{
    powerset_nonempty "$@" | filter_long_sets | filter_by_sum
}

unsorted_output()
{
    filtered_powerset "$@"
}

sorted_output()
{
    unsorted_output "$@" | sort -n
}

if $dosort ; then
    sorted_output "$@"
else
    unsorted_output "$@"
fi

