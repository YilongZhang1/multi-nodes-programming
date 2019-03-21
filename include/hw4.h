#ifndef HW4_H__
#define HW4_H__

#include <exception>
#include <stdexcept>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#include <chrono>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/lexical_cast.hpp>

#include <mpi.h>



typedef std::vector<boost::filesystem::path> path_vec_type;
typedef std::map<int, std::string> book_type;

const int MASTER_TO_WORKER = 200;
const int WORKER_TO_MASTER = 2001;
const int VERSES_PER_CHAPTER = 10000;
const int CHAPTERS_PER_BOOK = 1000;

// stuff about file scanning within the given folder
std::map<std::string, path_vec_type> getFiles (boost::filesystem::path dir);
void sortBySize(path_vec_type& file_vector);	


// master related functions
/// work distribution by the master
/// file_vector: the files in inverse size sorted order
/// fileIdx: the sequential number of file in the alphabetic sorted squence
/// return: word --> chapterVerseIndex for all books
std::map<std::string, std::vector<int>> workDistribution(path_vec_type& file_vector, int* fileIdx);

/// merge all the books in a map with the format: word -> indices
/// books: books I have already merged up to now
/// fileSeq: the sequential number of file in the alphabetic sorted squence
/// currentFileResult: current file result in a string, format: word1@100010001@10001002100~word2@10002010301
/// return: none
void mergeBooks(std::map<std::string, std::vector<int>>& books, int fileSeq, std::string currentFileResult);

/// write the books into a file named myindex.txt in the same directory as the executable
/// books: all the books in a map format: word -> indices
/// fileVecAlpha: file sequence number in alphabetic order
/// return: none
void serialize(std::map<std::string, std::vector<int>>& books, path_vec_type& fileVecAlpha, std::string filename);


// worker related functions
/// activate worker
void startWork();
/// parse the current file into a data structure
/// filename: a complete filename given by the master
/// return: parsed file into book with a book_type
book_type parseFileToBook(const std::string& filename);
/// invert the current book and serialize into a string
/// book: parsed file in book format
/// return: a serialization of the given book
std::string bookAnalysis(const book_type& book);	

	
	
	
	
	
	
#endif





















