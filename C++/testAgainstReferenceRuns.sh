#!/usr/bin/env bash

tmp_main_cpp=FixedSeed.cpp
failed_runs=()
for i in {1..5}
do
    cp GameRunner.cpp $tmp_main_cpp
    sed -i "s/time(NULL)/$i/g" $tmp_main_cpp
    make -j4 buildFixedSeed
    make run > /tmp/game_make_run_result
    tail -n +2 /tmp/game_make_run_result > /tmp/game_result
    diff /tmp/game_result reference/seed_$i
    if [[ $? -ne 0 ]]
    then
        failed_runs+=$i
    fi
done
rm $tmp_main_cpp
rm /tmp/game_make_run_result
rm /tmp/game_result

echo
echo "=============================="
if [[ ${#failed_runs[@]} -ne 0 ]]
then
    echo "Failed tests ${failed_runs[*]}"
else
    echo "All tests passed!"
fi
echo "=============================="
