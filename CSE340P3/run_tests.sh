#!/bin/bash  
if [ -z "$1" ]
  then

    for test_file in $(find ./test_cases -type f -name "*.txt" | sort); do
        name=`basename ${test_file} .txt`
        ./a.out < ${test_file} > ./outputs/${name}.output
        echo "${name} completed"
    done

    echo "All test cases completed."
else
    name="test_case$1"
    ./a.out < ./test_cases/${name}.txt > ./outputs/${name}.output
    echo "${name} completed"
fi
