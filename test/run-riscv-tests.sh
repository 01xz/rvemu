#!/bin/bash

TARGET="riscv64-unknown-elf"
RISCV_TESTS="$RISCV/$TARGET/share/riscv-tests"

RVEMU="../build/rvemu"

passed_count=0
failed_count=0
failed_tests=""

test_pattern=${1:-"rv64ui-p"}

for isa_test in $RISCV_TESTS/isa/$test_pattern-*; do
    if [[ "$isa_test" != *.dump ]]; then
        ./$RVEMU "$isa_test"
        result=$?
        if [ $result -eq 0 ]; then
            passed_count=$((passed_count + 1))
        else
            failed_count=$((failed_count + 1))
            failed_tests="${failed_tests}$(basename "$isa_test")"$'\n'
        fi
    fi
done

if [ $failed_count -eq 0 ]; then
    echo "All tests passed. Total passed: $passed_count"
else
    echo "Total passed: $passed_count"
    echo "Total failed: $failed_count"
    echo -e "Failed tests:\n$failed_tests"
fi
