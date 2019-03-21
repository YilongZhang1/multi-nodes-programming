// worker functions are here
#include "hw4.h"
#include <fstream>

void  startWork()
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	std::cout << "WORKERS: worker["<< rank << "], starts working..." << std::endl;

	char msgIn[ MASTER_TO_WORKER];
	MPI_Status status;		
	while (1) 
	{
		// Receive a message from the master
		MPI_Recv(msgIn, 					/* message buffer */
				 MASTER_TO_WORKER, 			/* buffer size */
				 MPI_CHAR, 					/* data type */
				 0,  						/* Receive from master */
				 MPI_ANY_TAG,		
				 MPI_COMM_WORLD, 			/* default communicator */
				 &status);

		// Check if we have been terminated by the master (flag == 0)
		// exit from the worker loop
		if (status.MPI_TAG == 0) 
		{
			std::cout << "WORKERS: worker["<< rank << "], received terminate signal" << std::endl;
			return;
		}
		
		// Convert the message into a string for parse work
		std::string filename(msgIn);
		book_type parsedBook = parseFileToBook(filename);	// parsedBook in index --> story format
		std::string result = bookAnalysis(parsedBook);		// wow, the whole book now is a string
		// the serialized result maybe a very very long string, longer than a string capacity?
		// so we have to send in several packets rather than one.
		int packetCnt = ceil((double)result.length() / (WORKER_TO_MASTER-1));
		// send back packets one by one

		for(int i = 1; i <= packetCnt; i++)
		{
			// define tag as the sequence number of packets, 1 means the last one 
			int tag = packetCnt + 1 - i;
			std::string line;
		
			char msgOut[WORKER_TO_MASTER];
			if(i == packetCnt)
			{
				line = result.substr((i-1) * (WORKER_TO_MASTER-1), result.length() - (i-1) * (WORKER_TO_MASTER-1));
				line.copy(msgOut, result.length() - (i-1)*(WORKER_TO_MASTER-1) );
				msgOut[ result.length() - (i-1)*(WORKER_TO_MASTER-1) ] = '\0';
			}
			else
			{
				line = result.substr((i-1) * (WORKER_TO_MASTER-1),  WORKER_TO_MASTER-1);	
				line.copy(msgOut,  WORKER_TO_MASTER - 1);		
				msgOut[ WORKER_TO_MASTER - 1] = '\0';				
			}						
//			std::cout<<"In worker "<<rank<<"; i = "<<i<<"; message out = "<< std::string(msgOut,50)<<std::endl;
			MPI_Send(msgOut,      	     	/* message buffer */
				 WORKER_TO_MASTER,   /* buffer size */
				 MPI_CHAR,          		/* data type */
				 0,              			/* destination process rank */
				 tag,  						/* user chosen message tag */
				 MPI_COMM_WORLD);   		/* default communicator */
		}		
	} // end of an infinite loop
	return;
}


 book_type  parseFileToBook(const std::string& filename)
{
	// Hold my simple chapter-verse index pointing at the line of text
	 book_type book;

	// Do the basic IO prep
	// line with hold the read-in file line
	// assume filename is a std::string holding the file to process
	std::string line;
	std::ifstream datafile(filename.c_str()); // open the file for input

//	std::chrono::high_resolution_clock c;

	if (datafile.is_open()) // test for open failure
	{
		unsigned int linesRead = 0;
				
		// Process file into memory
//        std::chrono::high_resolution_clock::time_point start = c.now();
		while ( datafile.good() ) // read while the stream is good for reading
		{	
			// Lines are typically formatted as chapter:verse: Text
			std::getline(datafile,line); // see: http://www.cplusplus.com/reference/string/getline/		
			if (line.empty())
				continue; // do not try to parse an empty line
		
			// Break / tokenize / split the line string into a 
			// vector of strings using boost and the ':'
			std::vector<std::string> chapterVerseStory;
			boost::split(chapterVerseStory, line, boost::is_any_of(":"));
	
			if (chapterVerseStory.size() < 3)
			{
				// we don't need the book names, so just continue
				// No matter what, we continue to next iteration because this is a non-verse
				continue;
			}
			// Trim. Chop out the blanks
			const std::string chapter(boost::algorithm::trim_copy(chapterVerseStory[0]));
			const std::string verse = boost::algorithm::trim_copy(chapterVerseStory[1]);
			const std::string text = boost::algorithm::trim_copy(chapterVerseStory[2]);
			// Create a line index	
			int chapterVerseIndex = boost::lexical_cast<int>(chapter) * VERSES_PER_CHAPTER +  boost::lexical_cast<int>(verse);

			if (book.find(chapterVerseIndex) != book.end())
			{
				std::cerr << "Problem with reading and parsing line {" << line << "}" << std::endl;
				std::cerr << "Parsed [" << chapter << ":" << verse << "], index = " << chapterVerseIndex << std::endl;
				std::cerr << "However, book currently has (" << book[chapterVerseIndex] << ") at index = " << chapterVerseIndex << std::endl;
				exit(1);
			}
			book[chapterVerseIndex] = text;		
			++linesRead;
		}
		datafile.close();

		// Log the file load
//		double sumTime = 0.0;
//		std::chrono::high_resolution_clock::time_point stop = c.now();
//		double splitTime = (double) std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count() / 1000000. ;
//		std::cout << "WORKERS: Read " << linesRead << " lines from [" << filename << "] in " << splitTime << " s " << std::endl;
	}
	else
	{
		std::stringstream msg;
		msg << "Unable to open file '" << filename << "'";
		throw std::runtime_error(msg.str());
	}
	return book;
}


std::string  bookAnalysis(const  book_type& book)
{
	// Split on any punctuation or space
	// build into maps with word => index
	std::map<std::string, std::vector<int>> word2index;

	for ( book_type::const_iterator curLine = book.begin(); curLine!=book.end(); ++curLine)
	{
		std::vector<std::string> words;
		boost::split(words, curLine->second, boost::is_any_of(": ,.;!?)("));
		for(std::vector<std::string>::const_iterator w = words.begin(); w != words.end(); ++w)
		{
			const std::string term(boost::algorithm::to_lower_copy(*w));			
			if (boost::algorithm::trim_copy(term).empty())
				continue;	// ignore empty strings
			
			// to avoid duplicates
			if(word2index[term].empty() || curLine->first > word2index[term].back())
				word2index[term].push_back(curLine->first);			
					
			if(word2index[term].size() >= VERSES_PER_CHAPTER)
			{
				std::cout<<"Some word occurs too frequently. Increase VERSES_PER_CHAPTER in the header file.\n";
				std::cout<<"specifically, the word is: "<<term<<std::endl;
				exit(0);
			}
		}		
	}		

	// now the map word2index is built, serialize it for transmission
	std::stringstream result;
	// the serialized result is in the format (note the delimiters @ and ~): 
	// a@1001@1005@1010~c@1001@1050@2005~
	for (std::map<std::string, std::vector<int>>::const_iterator t = word2index.begin(); t != word2index.end(); ++t)
	{
		const std::string term(t->first);
		result << term;
		for(std::vector<int>::const_iterator idx = t->second.begin(); idx != t->second.end(); ++idx)
			result << "@" << *idx;
		result << "~";
	}
//	std::cout<<"in child size = "<<result.str().size()<<"\n";
//	std::cout<<"in child max = \n"<<result.str().max_size()<<"\n";
//	std::cout<<"in child = \n"<<result.str()<<"\n";
	return result.str();
}
















