// master functions are here
#include <fstream>
#include "hw4.h"
std::map<std::string, std::vector<int>>  workDistribution( path_vec_type& file_vector, int* fileIdx)
{
	std::map<std::string, std::vector<int>> books;	// the return value
	// =========================
	// Master (thread 0) 
	// =========================
	int threadCount;
	MPI_Comm_size(MPI_COMM_WORLD, &threadCount);
	std::cout << "MASTER:  " << (threadCount-1) << " workers are available to process " << file_vector.size() << " books." << std::endl;
	
	std::chrono::high_resolution_clock c;
	std::chrono::high_resolution_clock::time_point start_parse = c.now();
	std::chrono::high_resolution_clock::time_point end_parse;
	double t_parse = 0.0;
	std::chrono::high_resolution_clock::time_point start_buildFile;
	std::chrono::high_resolution_clock::time_point end_buildFile;
	double t_buildFile = 0.0;

	// workerFileSeq means the current worker is working on file #, where # is the sequence of files in alphabetic order
	int* workerFileSeq = new int[threadCount - 1];
	// Start with 1, because the master is = 0
	int i = 0;
	for (int rank = 1; rank < threadCount && i < (int)file_vector.size(); ++rank, ++i) 
	{
		// work tag / index+1. Actually, useless for worker
		int tag =  fileIdx[i] + 1;	// here tag means sequential number of file in the files. +1 to avoid tag = 0, which causes transmission stop
		const std::string filename = file_vector[i].string();
		const size_t length = filename.size();
		char msg[ MASTER_TO_WORKER];
		filename.copy(msg, length);
		msg[length] = '\0';
		
		MPI_Send(msg,           	/* message buffer, nothing is sent */
				 MASTER_TO_WORKER,  /* buffer size */
				 MPI_CHAR,          /* data type */
				 rank,              /* destination process rank */
				 tag,  				/* user chosen message tag */
				 MPI_COMM_WORLD);   /* default communicator */  

		workerFileSeq[rank - 1] = tag - 1;
	}
		
	// Loop through the rest files until there is no more files to work on
	for ( ; i < (int)file_vector.size(); ++i) 
	{
		std::string currentFileResult;
		
		// Receive a packet from a worker
		char resultMsg[ WORKER_TO_MASTER];
  		MPI_Status status;

		// Receive a message from the worker
		MPI_Recv(resultMsg, 		/* message buffer */
 				 WORKER_TO_MASTER, 	/* buffer size */
				 MPI_CHAR, 			/* data type */
				 MPI_ANY_SOURCE,  	/* Receive from thread */
				 MPI_ANY_TAG,		/* tag */
				 MPI_COMM_WORLD, 	/* default communicator */
				 &status);
		     	
		int packetsLeft = status.MPI_TAG;
		const int sourceCaught = status.MPI_SOURCE;		
		currentFileResult = std::string(resultMsg);
		
		// continue receiving, until current serialized file is completed
		while(packetsLeft > 1)
		{
			MPI_Recv(resultMsg, 		/* message buffer */
					 WORKER_TO_MASTER, 	/* buffer size */
					 MPI_CHAR, 			/* data type */
					 sourceCaught,  	/* Receive from thread */
					 MPI_ANY_TAG,		/* tag */
					 MPI_COMM_WORLD, 	/* default communicator */
					 &status);	
			packetsLeft = status.MPI_TAG;
//			std::cout<<"In master, source = "<<sourceCaught<<"; packetsLeft = "<<packetsLeft<<std::endl;			
			currentFileResult += std::string(resultMsg);
		}
//		std::cout<<"MASTER:  worker "<<sourceCaught<<" finished job with file "<<workerFileSeq[sourceCaught - 1]<<std::endl;
		
		start_buildFile = c.now();
		// now, total serialized file is received. Merge it into books
		mergeBooks(books, workerFileSeq[sourceCaught - 1], currentFileResult);
		end_buildFile = c.now();
		t_buildFile += (double) std::chrono::duration_cast<std::chrono::microseconds>(end_buildFile - start_buildFile).count() / 1000000.0;
		
		// distribute new job
		// work tag / index+1. Actually, useless for worker
		int tag =  fileIdx[i] + 1; // sequential number of current file
		const std::string filename = file_vector.at(i).string();
		const size_t length = filename.size();
		char msg[ MASTER_TO_WORKER ];
		filename.copy(msg,length);
		msg[length] = '\0';
//		std::cout<<"current i = "<<i<<"; current tag = "<<tag-1<<std::endl;

		MPI_Send(msg,				/* message buffer */
				 MASTER_TO_WORKER,  /* buffer size */
				 MPI_CHAR,			/* data type */
				 sourceCaught,		/* destination process rank */
				 tag,				/* user chosen message tag */
				 MPI_COMM_WORLD);	/* default communicator */
				 
		// update the workerFileSeq
		workerFileSeq[sourceCaught - 1] = tag - 1;
	}	
	
	// There's no more work to be done, so receive all the outstanding
	// results from the workers

	for (int rank = 1; rank < threadCount; ++rank) 
	{
	    std::string currentFileResult;		
		char resultMsg[ WORKER_TO_MASTER ];
  		MPI_Status status;

		// Receive a message from the worker
		MPI_Recv(resultMsg, 		/* message buffer */
				 WORKER_TO_MASTER,	/* buffer size */
				 MPI_CHAR, 			/* data type */
				 rank,  			/* Receive from master */
				 MPI_ANY_TAG,		
				 MPI_COMM_WORLD,	/* default communicator */
		     	 &status);

		int packetsLeft = status.MPI_TAG;	
		currentFileResult = std::string(resultMsg);
		
		// continue receiving, until current serialized file is completed
		while(packetsLeft > 1)
		{
			MPI_Recv(resultMsg, 		/* message buffer */
					 WORKER_TO_MASTER, 	/* buffer size */
					 MPI_CHAR, 			/* data item is an integer */
					 rank,  			/* Receive from thread */
					 MPI_ANY_TAG,		/* tag */
					 MPI_COMM_WORLD, 	/* default communicator */
					 &status);	
			packetsLeft = status.MPI_TAG;
			currentFileResult += std::string(resultMsg);
		}
		
		start_buildFile = c.now();		
		// now, total serialized file is received. Merge it into books
		mergeBooks(books, workerFileSeq[rank - 1], currentFileResult);		
		end_buildFile = c.now();
		t_buildFile += (double) std::chrono::duration_cast<std::chrono::microseconds>(end_buildFile - start_buildFile).count() / 1000000.0;
	}

	delete[] workerFileSeq;
		
	end_parse = c.now();
	t_parse += (double) std::chrono::duration_cast<std::chrono::microseconds>(end_parse - start_parse).count() / 1000000.0;
	t_parse -= t_buildFile;
	
	std::cout<<"========================================================================\n";
	std::cout<<"\t\t\tTiming Statistics\n\n";
	std::cout<<"1. Total file read time is " << t_parse << " s" <<std::endl;
	std::cout<<"2. Total time building the in-memory inverted file is "<< t_buildFile <<" s"<<std::endl;
	
	
	// Tell all the workers to exit by sending an empty message with the TERMINATE tag (0)
	for (int rank = 1; rank < threadCount; ++rank)
	{
		MPI_Send(0, 0, MPI_INT, rank, 0, MPI_COMM_WORLD);
	}

	return books;
}


void mergeBooks(std::map<std::string, std::vector<int>>& books, int fileSeq, std::string currentFileResult)
{
//	std::cout<<"fileSeq = "<<fileSeq<<"\n";
//	std::cout<<"in master, currentFileResult size = "<<currentFileResult.size()<<"\n";
	std::vector<std::string> lines;
	boost::split(lines, currentFileResult, boost::is_any_of("~"));  

	// If there is no data, early return
	if (lines.size() < 1) return;

	for(std::vector<std::string>::const_iterator l = lines.begin(); l != lines.end(); ++l)
	{
		// Parse out current line
		const std::string line(*l);
		if (boost::algorithm::trim_copy(line).empty())
			continue;	// ignore empty strings

		std::vector<std::string> terms;
		boost::split(terms, line, boost::is_any_of("@"));
		
		if (terms.size() < 1) 
			continue;	// should not happen
		
		for(int i = 1; i < (int)terms.size(); i++)
		{
			const std::string term(terms[i]);
			if (boost::algorithm::trim_copy(term).empty()) 
				continue;	// should not happen
//			std::cout<<"terms["<<i<<"] = "<<terms[i]<<std::endl;
			books[ terms[0] ].push_back(fileSeq * VERSES_PER_CHAPTER * CHAPTERS_PER_BOOK + std::stoi(terms[i]));
		}
	}	
}


void serialize(std::map<std::string, std::vector<int>>& books, path_vec_type& fileVecAlpha, std::string filename)
{
	std::ofstream out(filename);
	for(std::map<std::string, std::vector<int>>::const_iterator b = books.begin(); b != books.end(); ++b)
	{
		// first, sort the books within one key
		std::string word = b->first;
		out << word << "=";
		std::vector<int> fileChapVerseIndex = b->second;
		std::sort(fileChapVerseIndex.begin(), fileChapVerseIndex.end());
		
		std::vector<int>::iterator v;
		for(v = fileChapVerseIndex.begin(); v != fileChapVerseIndex.end()-1; v++)
		{
			int fileID = (*v) / VERSES_PER_CHAPTER / CHAPTERS_PER_BOOK;
			int chapIdx = ((*v) % (VERSES_PER_CHAPTER * CHAPTERS_PER_BOOK)) / VERSES_PER_CHAPTER;
			int verseIdx = (*v) % VERSES_PER_CHAPTER;
			out<< fileVecAlpha[fileID].filename().string() << "," << chapIdx << ":" << verseIdx << "; ";
		}
		
		// no semicolon for the last term
		int fileID = (*v) / (VERSES_PER_CHAPTER * CHAPTERS_PER_BOOK);
		int chapIdx = ((*v) % (VERSES_PER_CHAPTER * CHAPTERS_PER_BOOK)) / VERSES_PER_CHAPTER;
		int verseIdx = (*v) % VERSES_PER_CHAPTER;
		out<< fileVecAlpha[fileID].filename().string() << "," << chapIdx << ":" << verseIdx;		
		out << "\n";	
	}
}
















