Power Set generator
===================

A utility to generate [power sets](https://en.wikipedia.org/wiki/Power_set).

Filters
-------

  * Max Items

    Restricts the output to only the subsets that have
    at most a specified number of items.

  * Required Sum

    Restricts the output to only the subsets that sum
    to a specified value. *WARNING:* when this filter
    is used, each item in the set *must* be an inteter!

Options
-------

  * `-m <INTEGER>`

    Sets the maximum number of items allowed in any subset.

  * `-s <INTEGER>`

    Sets the sum filter's required value. WARNING: when this
    option is used, only integer items are supported.

  * `-a`

    Outputs subsets in an arrayh format (`[1,2,3]`) instead
    of a simple space-separated list (`1 2 3`).

  * `-E`

    Skips the empty set, which is normally output as a blank line.

Example
-------

```sh
$ ./powerset $(seq 3)

1
2
1 2
3
1 3
2 3
1 2 3
$ ./powerset -s 10 $(seq 5)
1 2 3 4
2 3 5
1 4 5
$ ./powerset -s 10 -m 3 $(seq 5)
2 3 5
1 4 5
```

Shell Script
------------

The file `powerset.sh` is a mostly-equivalent implementation
that relies only on the POSIX shell. It should run in most
environments, though it is consderably slower and wastes significantly
more memory than the C implementaion.


Copyright
---------

Copyright 2015 Brent Sanders

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
