#!/bin/sh
# Monga
# Author: Gabriel de Quadros Ligneul

test_folder=$1
test_bin=$2

if [[ $3 == "-v" ]]; then
    verbose=true
fi

echo "Test: $test_folder"

for test_in in $( find $test_folder -name "*.in" | sort ); do
    test_out=$( echo $test_in | sed -e "s/in$/out/" )
    test_exp=$( echo $test_in | sed -e "s/in$/exp/" )
    if [ ! -e $test_exp ]; then
        continue
    fi
    if [ "$verbose" = true ]; then
        echo "$test_in"
    fi
    
    ./$test_bin < $test_in &> $test_out
    if ! cmp --silent $test_exp $test_out; then
        echo "----------------------------------------"
        echo "Test failed: $test_in"
        echo "diff -u $test_exp $test_out"
        diff -u $test_exp $test_out
        echo "----------------------------------------"
        exit 1
    fi
    rm $test_out
done
echo "Test succeeded!"

