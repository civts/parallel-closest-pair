#! /usr/bin/env bash

# Run this script from the root of the project to test the code

cd test
cmake -B build -S .
cmake --build build
cd build
make test
TEST_RESULT=$?

if [[ $TEST_RESULT -ne 0 ]]; then
  echo -e "\n\n"
  TESTS_LOG_FILE=./Testing/Temporary/LastTest.log

  GREEN='\x1B[32m'
  RED='\x1B[0;31m'
  RESET='\x1B[0m'

  COLORIZE_GREEN_PASSED="s/\(PASSED\|OK\)/${GREEN}\1${RESET}/g"
  sed -i $COLORIZE_GREEN_PASSED $TESTS_LOG_FILE

  COLORIZE_RED_FAILED="s/\(FAILED\)/${RED}\1${RESET}/g"
  sed -i $COLORIZE_RED_FAILED $TESTS_LOG_FILE

  echo "$(<$TESTS_LOG_FILE)"

  exit 1
else
  echo "Succeeded"
fi
