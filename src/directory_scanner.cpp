/* 
  Sample code that reads a directory that was 
  specified and lists the text files that exists
  
  Purpose: Demonstrate use of Boost:Filesystem and directory scanning for files.
  
  Build: g++ directory_scanner.cpp -o directory_scanner -lboost_filesystem-mt -lboost_system-mt
*/
#include "hw4.h"
#include <exception>
#include <stdexcept>

// +++++++++++++++++++++++++++++++++++++++++++++++++++

// See the boost documentation for the filesystem
// Especially: http://www.boost.org/doc/libs/1_53_0/libs/filesystem/doc/reference.html#Path-decomposition-table
// Link against boost_filesystem-mt (for multithreaded) or boost_filesystem

// +++++++++++++++++++++++++++++++++++++++++++++++++++


// ===== IMPLEMENTATIONS =======
std::map<std::string,  path_vec_type>  getFiles (boost::filesystem::path dir)
{
	// Define my map keys
	const std::string regular_file("REGULAR");
	const std::string directory_file("DIRECTORY");
	const std::string other_file("OTHER");
	
	// This is a return object
	// REGULAR -> {file1r,file2r,...,fileNr}
	// DIRECTORY -> {file1d,file2d,...,fileNd}
	// ...
	std::map<std::string, path_vec_type> directoryContents;
	
	// Change to the absolute system path, instead of relative
	// boost::filesystem::path dirPath(boost::filesystem::system_complete(dir));
	boost::filesystem::path dirPath(dir);
	
	// Verify existence and directory status
	if ( !boost::filesystem::exists( dirPath ) ) 
	{
		std::stringstream msg;
		msg << "Error: " << dirPath.string() << " does not exist " << std::endl;
		throw std::runtime_error(msg.str());
	}
	
	if ( !boost::filesystem::is_directory( dirPath ) ) 
	{
		std::stringstream msg;
		msg << "Error: " << dirPath.string() << " is not a directory " << std::endl;
		throw std::runtime_error(msg.str());
	}

#ifdef GJS_DEBUG_PRINT				
	std::cout << "Processing directory: " << dirPath.directory_string() << std::endl;
#endif	

	// A directory iterator... is just that, 
	// an iterator through a directory... crazy!
	boost::filesystem::directory_iterator end_iter;
	for ( boost::filesystem::directory_iterator dir_itr( dirPath ); dir_itr != end_iter; ++dir_itr )
	{
		// Attempt to test file type and push into correct vector
		try
		{
			if ( boost::filesystem::is_directory( dir_itr->status() ) )
			{
				// Note, for path the "/" operator is overloaded to append to the path
				directoryContents[directory_file].push_back(dir_itr->path());
#ifdef GJS_DEBUG_PRINT				
				std::cout << dir_itr->path().filename() << " [directory]" << std::endl;
#endif
			}
			else if ( boost::filesystem::is_regular_file( dir_itr->status() ) )
			{
				directoryContents[regular_file].push_back(dir_itr->path());
#ifdef GJS_DEBUG_PRINT				
				std::cout << "Found regular file: " << dir_itr->path().filename() << std::endl;
#endif
			}
			else
			{
				directoryContents[other_file].push_back(dir_itr->path());
#ifdef GJS_DEBUG_PRINT				
				std::cout << dir_itr->path().filename() << " [other]" << std::endl;
#endif
			}

		}
		catch ( const std::exception & ex )
		{
			std::cerr << dir_itr->path().filename() << " " << ex.what() << std::endl;
		}
	}	
	return directoryContents;
}


// bubble sorting is OK since number of elements is not large
void  sortBySize( path_vec_type& file_vector)
{
	for(unsigned i = 0; i < file_vector.size()-1; i++)
		for(unsigned j = i+1; j < file_vector.size(); j++)
			if( boost::filesystem::file_size(file_vector.at(i)) < boost::filesystem::file_size(file_vector.at(j)) )
			{
				boost::filesystem::path temp = file_vector.at(i);
				file_vector.at(i) = file_vector.at(j);
				file_vector.at(j) = temp;
			}
}







