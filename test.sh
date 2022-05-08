#!/bin/bash
in_cnt=$(ls in -U | wc -l)
out_cnt=$(ls out -U | wc -l)

if [ $in_cnt != $out_cnt ] ; then
  echo "File numbers not matching."
  exit
fi

for i in `seq $in_cnt`
do
  ./compiler "in/${i}.txt" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"
  expected="$(cat out/${i}.txt)"
  if [ $actual = $expected ] ; then
      echo "got $actual, as expected."
  else
      echo "$expected expected, but got $actual."
      exit 1
  fi
  echo -e ""
done

echo OK