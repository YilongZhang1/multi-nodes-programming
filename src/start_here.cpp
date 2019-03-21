#include "hw4.h"


int main (int argc, char * argv[])
{
	// Initialize MPI
	// Find out my identity in the default communicator
	MPI_Init(&argc, &argv);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// Logic branch
	if (rank == 0) 
	{
		// ++++++++++++++++++++++++++++++
		// Master process
		// ++++++++++++++++++++++++++++++	
		if (argc != 3)
		{
			std::cout << "Usage: mpirun -np [NUM_NODES] " << argv[0] << " [directory] [index_filename] " << std::endl;
			return 1;
		}
		std::cout<<"MASTER:  sort books by size from the largest to the smallest\n";
		// Define a template type, and its iterator	
		typedef std::map<std::string,  path_vec_type> content_type;
		typedef content_type::const_iterator content_type_citr;
		
		// time the entire program
		std::chrono::high_resolution_clock c;
		std::chrono::high_resolution_clock::time_point start_entire = c.now();
		// Get the file list from the directory
		content_type directoryContents =  getFiles(argv[1]);

		// For each type of file found in the directory, list all files of that type
		path_vec_type file_vector;
		int* fileIdx;	// fileIdx[3] = 10 means 4th inverse size sorted file is placed at 11th when use alphabetic order
		path_vec_type fileVecAlpha;
		for (content_type_citr f = directoryContents.begin(); f != directoryContents.end(); ++f)
		{
			file_vector = f->second;			
			// sort the files in current folder from the largest size to smallest
			if(f->first == "REGULAR")
			{
				 fileIdx = new int[file_vector.size()];
				 fileVecAlpha = file_vector;
				std::sort( fileVecAlpha.begin(),  fileVecAlpha.end());
				 sortBySize(file_vector);
				for(int i = 0; i < (int)file_vector.size(); i++)
					for(int j = 0; j < (int)file_vector.size(); j++)
						if(file_vector.at(i) ==  fileVecAlpha.at(j))
							 fileIdx[i] = j;
				// I am not interested in other types of directories, so break
				break;				
			}
/*			// make sure the file_vector and fileVecAlpha are correct
			std::cout << "sorted by size from largest to smallest" << std::endl;
			for ( path_vec_type::const_iterator i = file_vector.begin(); i != file_vector.end(); ++i)
			{
				// boost::filesystem::path file_path(boost::filesystem::system_complete(*i));
				boost::filesystem::path file_path(*i);
				std::cout << "\t" << file_path.string() << std::endl;
				std::cout << "\t size = " << boost::filesystem::file_size(file_path) << std::endl;
			}
			std::cout << "sorted by alphabetics" << std::endl;
			for ( path_vec_type::const_iterator i = fileVecAlpha.begin(); i != fileVecAlpha.end(); ++i)
			{
				// boost::filesystem::path file_path(boost::filesystem::system_complete(*i));
				boost::filesystem::path file_path(*i);
				std::cout << "\t" << file_path.string() << std::endl;
				std::cout << "\t size = " << boost::filesystem::file_size(file_path) << std::endl;
			}
*/
		}

		// now the master has the list of sorted files, and it should be ready to distribute jobs
		std::map<std::string, std::vector<int>> books =  workDistribution(file_vector, fileIdx);
		MPI_Barrier(MPI_COMM_WORLD);

		std::chrono::high_resolution_clock::time_point start_writeOut = c.now();
		serialize(books, fileVecAlpha, argv[2]);		
		std::chrono::high_resolution_clock::time_point end_writeOut = c.now();
		double t_writeOut = (double) std::chrono::duration_cast<std::chrono::microseconds>(end_writeOut - start_writeOut).count() / 1000000.0;	
		
		std::cout << "3. Total time writing the output file " << t_writeOut << " s" << std::endl;		

		delete[]  fileIdx;
		
		std::chrono::high_resolution_clock::time_point stop_entire = c.now();
		const double t_entire = (double) std::chrono::duration_cast<std::chrono::microseconds>(stop_entire - start_entire).count() / 1000000.0;
		std::cout << "4. The entire program completed in " << t_entire << " s" << std::endl
			<< "========================================================================" << std::endl; 
		std::cout << "Done! Check the inverted file " << argv[2] << std::endl;			
	}
	else 
	{	
		// ++++++++++++++++++++++++++++++
		// Workers
		// ++++++++++++++++++++++++++++++
		startWork();
		MPI_Barrier(MPI_COMM_WORLD); // this is linked to the above barrier
	}
	
	// Shut down MPI
	MPI_Finalize();
	
	return 0;
}


// barrier of master and worker
// when will worker jumps out of while loop

















