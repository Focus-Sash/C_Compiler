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

assert 0 0
assert 42 42
assert 21 "5+20-4"

echo OK