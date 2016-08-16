default: wordStatistics_ASP
wordStatistics_ASP:
	gcc -o wordStatistics_ASP wordStatistics_ASP.c -lpthread

clean:	
	rm -rf *.o wordStatistics_ASP wordCount.Txt letterCount.txt
