#!/bin/bash
assert() {
    expected="$1"
    input="$2"
    
    ./compiler "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"
    echo $actual
    echo $expected
    if [ $actual = $expected ] ; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual."
        exit 1
    fi
}

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 12 "6 + 3 * 2;"
assert 47 "5+6*7;"
assert 9 "3 * (9 - 6);"
assert 25 "4 + (3 - 2) * 9 + 3 * 4;"
assert 10 "-10 + 20;"
assert 1 "10 == 10;"
assert 0 "10 == 11;"
assert 1 "10 < 11;"
assert 0 "10 < 9;"
assert 1 "10 > 9;"
assert 0 "10 > 11;"
assert 1 "10 >= 9;"
assert 1 "10 >= 10;"
assert 0 "10 >= 11;"
assert 0 "10 <= 9;"
assert 1 "10 <= 10;"
assert 1 "10 <= 11;"
assert 1 "a=3;a==3;"
assert 0 "a=3;a!=3;"
assert 3 "a=3;a;"
assert 3 "ab=3;ab;"
assert 3 "_=3;_;"
assert 3 "_x=3;_x;"
assert 3 "_09090=3;_09090;"
assert 3 "a=3;b=2;return 3;"
assert 3 "a=4;return a-1"


echo OK
