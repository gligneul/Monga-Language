#!/bin/sh

out=$(find tests -name "*.out")
exp=$(echo $out | sed 's/\.out/.exp/')
echo mv $out $exp
mv $out $exp

