This code implements cluster computing following the map reduce fashion.


1. to run the executable in command line
    1) module add openmpi/openmpi-3.0.0
    2) cmake3 .
    3) make
    Sometimes error occurs in make. Do step 4 and 5 will solve this problem.
    4) cmake3 .
    5) make
    Now everything should be good. Run the executable with
    6) mpirun -np [#_Nodes] ./multi_nodes [Inverted_File]
    There should be an inverted output file in the path you provided in command line.

2. to run the executable with SLURM
    1) sbatch analysis.sh
    invertedFile is the output inverted file for the books
    and timing_statistics-PID.out records timings in each module of the executable with different number of nodes
        
3. some timing statistics
                                                node = 2      node = 4      node = 6      node = 8      node = 10      node = 12
--------------------------------------------------------------------------------------------------------------------------------------
Total file read time                              3.2766        0.2536      0.1584        0.1498        0.1277          0.1425
Total time building in-memory inverted file       1.4929        1.4994      1.4997        1.4979        1.5168          1.6722
Total time writing the output file                0.4473        0.4458      0.4535        0.4549        0.4679          0.4396
Total time for the entire program                 5.2462        2.2243      2.1414        2.1278        2.1398          2.3008

