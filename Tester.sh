#!/bin/bash

gcc MultiProc_server.c -o server
gcc client.c -o client
./server &
./client "InputFiles/test_case_1 2.csv" "GeneratedOutputs/output_1 2.csv" &
./client "InputFiles/test_case_1.csv" "GeneratedOutputs/output_1.csv" &
./client "InputFiles/test_case_2 1.csv" "GeneratedOutputs/output_2 1.csv" &
./client "InputFiles/test_case_3_1.csv" "GeneratedOutputs/output_3 1.csv" &

