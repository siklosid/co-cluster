DEPENDENCIES:
libpthread
scons


********* BUILD: *********

1. go to directory src
2. scons
3. scons will create the binary to src/build/clustering/clustering


********* ENVIRONMENT:

To run the binary first you have to create a clustroot directory.
This directory stores the configuration file, inputs, outputs.
Example:
        test/clustroot/
                clustering.conf (configuration file)
                inputs/         (input directory)
                outputs/        (output directory)


********* PARAMETERS:

Every parameter can be set in two ways:
1. in the configuration file (see the above example)
2. in command line as follows: ./clustering --<param_name>=<value>
If a parameter appears in both then the program will use the command line value.

./clustering --help prints the list of available parameters

Important parameters:
        configfile - name of the configfile (default is clustering.conf)
        reader -
                simple - dense format (test/clustroot/kl_matrix.txt)
                sparse - sparse format (test/clustroot/kl_sparsematrix.txt)
        similarity - what kind of similarity you would like to use. (for Jensen-Shannon it's js)
        data_type - the inner representation of data. It can be dense or sparse
        num_row_cluster
        num_col_cluster
        num_iter - number of iterations
        do_bicluster - if set to false then it's going to be a simple one-dimensional clusterng
                       if set to true then it's going to be co-clustering
        do_output_sims - if set to true it will output all the distances between the elements and clusters


********* RUN:

export CLUSTROOT=<full path of the clustroot directory>
./clustering <parameters>


********* EXAMPLE:

1. sparse Jensen-Shannon
export CLUSTROOT=.../test/clustroot/
./clustering

You can read the outputs in $CLUSTROOT/outputs


2. dense Jensen-Shannon

export CLUSTROOT=.../test/clustroot/
./clustering --configfile=$CLUSTROOT/clustering.conf.kl
