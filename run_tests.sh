#!/bin/sh
# PUC-Rio
# INF1715 Compiladores
# Gabriel de Quadros Ligneul 1212560

if [[ $1 == "-v" ]]; then
    verbose=true
fi

run_test() {
    for f in $( find $4 -name "*.$5" | sort ); do
        out=$( echo $f | sed -e "s/$5$/out/" )
        exp=$( echo $f | sed -e "s/$5$/exp/" )
        if [ ! -e $exp ]; then
            continue
        fi
        if [ "$verbose" = true ]; then
            echo "$f"
        fi
        if [ "$3" = true ]; then
            $2 < $f &> $out
        else
            $2 $f &> $out
        fi
        if ! cmp --silent $exp $out; then
            echo "Test failed: $f"
            printf "%-64s%s\n" "expected" "output"
            diff -y -W 120 $exp $out
            exit 1
        fi
        rm $out
    done
    echo "Test succeeded: $1"
}

# Units tests
for t in $( ls tests ); do
    exe=$( printf "./bin/%s_test" $t )
    run_test $t $exe true "tests/$t" "in"
done

# Compiler tests
run_test "monga" "./monga" false "examples" "mng"

