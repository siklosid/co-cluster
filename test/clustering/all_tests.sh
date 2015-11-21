#!/bin/bash

let num_test=0
let succ_test=0

run_test()
{
    if ./all_tests $1
    then
        let succ_test=succ_test+1
    fi
    let num_test=num_test+1
}


# Testing Readers and DataTypes
run_test "--gtest_filter=DataTest.*:ReaderTest.* --configfile=$CLUSTROOT/reader_data_test.conf --log_level=0"
run_test "--gtest_filter=L?SimilarityTest.*:-L?SimilarityTest.*Bi* --configfile=$CLUSTROOT/clustering.conf.onedata --do_bicluster=false --log_level=0"
run_test "--gtest_filter=L?SimilarityTest.*Bi* --configfile=$CLUSTROOT/clustering.conf.onedata --do_bicluster=true --log_level=0"
run_test "--gtest_filter=KLSimilarityTest.*:-KLSimilarityTest.*Bi* --configfile=$CLUSTROOT/clustering.conf.kl --do_bicluster=false --log_level=0"
run_test "--gtest_filter=KLSimilarityTest.*Bi* --configfile=$CLUSTROOT/clustering.conf.kl --do_bicluster=true --log_level=0"


echo "***************************************"
echo $succ_test"/"$num_test" test was succesfull"
