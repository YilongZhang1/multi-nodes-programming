#! /bin/bash
#-------------------------------------------------------------------------------
#  SBATCH CONFIG
#-------------------------------------------------------------------------------
## resources
#SBATCH --partition hpc0
#SBATCH --nodes=1
#SBATCH --ntasks=12  # used for MPI codes, otherwise leave at '1'
#DISABLED #SBATCH --ntasks-per-node=1  # don't trust SLURM to divide the cores evenly when you use more than one node
#SBATCH --cpus-per-task=1  # cores per task; set to one if using MPI
#DISABLED #SBATCH --exclusive  # using MPI with 90+% of the cores you should go exclusive
#SBATCH --mem-per-cpu=4G  # memory per core; default is 1GB/core
#SBATCH --time 0-15:00  # days-hours:minutes
#SBATCH --qos=normal
#SBATCH --account=general  # investors will replace this with their account name
#
## labels and outputs
#SBATCH --job-name=mpi_job
#SBATCH --output=timing_statistics-%j.out  # %j is the unique jobID
#SBATCH -e errors-%j.log # job error output

# Load your modules here:
module add openmpi/openmpi-3.0.0

# MPI flag for explicit saftey
export PSM_RANKS_PER_CONTEXT=2

cmake3 .
make
cmake3 .
make

mpirun -np 2 ./multi_nodes ./data/book/ invertedFile
mpirun -np 4 ./multi_nodes ./data/book/ invertedFile
mpirun -np 6 ./multi_nodes ./data/book/ invertedFile
mpirun -np 8 ./multi_nodes ./data/book/ invertedFile
mpirun -np 10 ./multi_nodes ./data/book/ invertedFile
mpirun -np 12 ./multi_nodes ./data/book/ invertedFile


















