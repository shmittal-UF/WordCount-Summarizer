Readme For Running this Program

1> Run the make File
2> Run the output(wordStatistics_ASP) with 4 arguments (arg1 arg2 arg3 arg4)
   here arg1 is input file name
        arg2 is Number of mapper threads
        arg3 is Number of reducer threads
        arg4 is number of summarizer threads.
   example
   ./wordStatistics_ASP 'home/inputx.txt' 2 1 2

3> Go to the problem folder location, there you can see two new text files(wordCount.txt) 
   and (letterCount.txt) as output files;
   
4> For Re-Run run 'make clean' followed by 'make' command.


Module Functionality

It has 4 Buffers and size of each Buffer is variable namedly
1>Reducerpool_Buffer
2>MapperPool_Buffer
3>SummarizerPool_Buffer
4>LetterCountTable

I am also using 5 mutex, each per thread to maintain syncronization.

I am storing in,out,item count for each buffer to provide syncronization.

I am using condition variables(empty,full) for each pool buffer to alert the threads about the empty and full status 
of Pool buffer.

It has 6 threads namedly:-

1>MapperPoolUpdater-> Gets a string with input with starting with same letter and put it into 
                      Reducerpool_Buffer as one entry and awakes mapper thread until EOF is not encountered
2>Mapper->Each thread creates tuple of form (word,1) for each entry of Reducerpool_Buffer and puts into reducer pool and 
         awakes reducer thread and look for next entry from mapperpool to perform same action
3>Reducer->Each thread reduces the tuples into (word,count) from each entry of reducer pool  using linked list and awakes 
           Summarizer Thread and look for next entry from reducerpool to perform same action
4>Summarizer-> Each thread reads an entry from the summarizer pool and alert the wordcounter thread to consume the same and put the 
               same to LetterCount Table and look for next entry from summarizer_pool to perform same action
5>WordCountWriter-> Reads an entry from the summarizer pool and print in the file wordCount.txt and look for next entry 
                    from the summarizer pool.
6>LetterCountWriter-> Reads from the lettercount table and print in the file letterCount.txt and look for next entry from
                     the letterCount table.