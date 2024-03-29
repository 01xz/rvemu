#!/bin/bash

TARGET="riscv64-unknown-elf"
RISCV_TESTS="$RISCV/$TARGET/share/riscv-tests"

RISCV_ISA_TESTS="$RISCV_TESTS/isa"

RVEMU="../build/rvemu"

passed_count=0
failed_count=0
failed_tests=""

test_pattern=${1:-"rv64ui-p"}

for isa_test in $RISCV_ISA_TESTS/$test_pattern-*; do
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
    echo -e "\e[32mAll tests passed. Total passed: $passed_count\e[0m"
    exit 0
else
    echo -e "\e[31mTotal passed: $passed_count\e[0m"
    echo -e "\e[31mTotal failed: $failed_count\e[0m"
    echo -e "\e[31mFailed tests:\e[0m\n$failed_tests"
    exit 1
fi
