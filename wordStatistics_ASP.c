
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#define Mapper_Size 7
#define Reducer_Size 7
#define Summarizer_Size 7
#define LetterCount_Size 7


/* Buffer For Each Pool */

char MapperPool_Buffer[Mapper_Size][1000],ReducerPool_Buffer[Reducer_Size][1000],SummarizerPool_Buffer[Summarizer_Size][1000];
char LetterCountTable[LetterCount_Size][1000];

/* To Track In and Out of each Item in Pool */
int MapperPool_In,MapperPool_out,MapperPool_Item;
int ReducerPool_In,ReducerPool_Out,ReducerPool_Item;
int SummarizerPool_In,SummarizerPool_Out,SummarizerPool_Item=0;
int letterCountTable_In,letterCountTable_Out,letterCountTable_Item=0;
int WordCount_In,WordCount_Out,WordCount_Item=0;
int i,j;
int flag=0;

/*Mutex & Conditional Variables */
pthread_mutex_t MapperPool_Mutex,ReducerPool_Mutex,SummarizerPool_Mutex,LetterCountTable_Mutex,WordCount_Mutex;
pthread_cond_t MapperPool_Empty, MapperPool_Full,ReducerPool_Empty, ReducerPool_Full,SummarizerPool_Empty, SummarizerPool_Full;
pthread_cond_t LetterCountTable_Empty, LetterCountTable_Full,WordCount_Empty,WordCount_Full;
  
struct ListNode
{
	char *word;
	int count;
	struct ListNode* next;
};


struct ListNode* insert_list(struct ListNode* head,char *value){
 	struct ListNode* head_node;
 	char *new_val;
 	new_val=(char *)malloc(sizeof(char)*strlen(value));
 	strcpy(new_val,value);
 	head_node=head;
 	if(head==NULL)
 	{
 	struct ListNode* new_node=(struct ListNode*)malloc(sizeof(struct ListNode));
 	new_node->word=(char *)malloc(sizeof(char)*strlen(value));
 	strcpy(new_node->word,value);
 	new_node->count=1;
 	new_node->next=NULL;
 	head=new_node;
 	return head;
    }
 	else
 	{
 			 head=head_node;
 			 while(strcmp(head_node->word,new_val)!=0&& head_node->next!=NULL)
 			 {
 			 head_node=head_node->next;
			 }
 			 if(head_node->next==NULL)
 			 {
 			 	if(strcmp(head_node->word,new_val)==0)
 			 	{
 			 	head_node->count=head_node->count+1;	
				}
 			 	else{
				  
 			 		struct ListNode* new_node=(struct ListNode*)malloc(sizeof(struct ListNode));
                	new_node->word=(char *)malloc(sizeof(char)*strlen(value));
 	                strcpy(new_node->word,value);
                	new_node->count=1;
 	                new_node->next=NULL;
                    head_node->next=new_node;
                   }
			 }
 			 else
 			 {
 			 	head_node->count=head_node->count+1;
			 }
	   }
	return head;
}
	
/* thread function */
void *MapperPoolUpdater(void *arg) {
  const char *filename = (char *)arg;
  char str[1000],str1[1000];
  char first_str;
  memset(&str1[0],0,strlen(str1));
  FILE *file1; 
  int a=0;          
  file1 = fopen(filename , "r");
  if (file1)
   {
   	pthread_mutex_lock(&MapperPool_Mutex);	
    while (fscanf(file1,"%s", str)!=EOF)
    {    while (MapperPool_Item == Mapper_Size)			
         pthread_cond_wait(&MapperPool_Full, &MapperPool_Mutex);
    	 if(str[0]==first_str||a==0)
    	 {   
    		strcat(str1,str);
    		strcat(str1," ");
    		first_str=str[0];
    		a=1;
		 }
		 else
		 {   
		    first_str=str[0];
		 	memcpy(&MapperPool_Buffer[MapperPool_In],&str1,sizeof(str1));
		 	MapperPool_In = (MapperPool_In+1)%Mapper_Size;
		 	MapperPool_Item++;	/* release the buffer */
            memset(&str1[0],0,strlen(str1));
            memcpy(&str1[0],str,sizeof(str));
            strcat(str1," ");
		    pthread_cond_signal(&MapperPool_Empty);	/* wake up consumer */
            pthread_mutex_unlock(&MapperPool_Mutex);
         }
	}
        while (MapperPool_Item == Mapper_Size)			
        pthread_cond_wait(&MapperPool_Full, &MapperPool_Mutex);
    	memcpy(&MapperPool_Buffer[MapperPool_In],&str1,sizeof(str1));
		MapperPool_In = (MapperPool_In+1)%Mapper_Size;
		MapperPool_Item++;	/* release the buffer */
        memset(&str1[0],0,strlen(str1));
        memcpy(&str1[0],str,sizeof(str));
        strcat(str1," ");
		pthread_cond_signal(&MapperPool_Empty);	/* wake up consumer */
        pthread_mutex_unlock(&MapperPool_Mutex);
   }
   
  fclose(file1);
  pthread_exit(NULL);

}


void *Mapper(void *arg) {
   	char str[1000],str1[1000],temp[1000];
	memset(&str[0],0,strlen(str));
	memset(&str1[0],0,strlen(str1));
	memset(&temp[0],0,strlen(temp));
	char *pch;
 while(1)
{  
  
  pthread_mutex_lock(&MapperPool_Mutex);	/* protect buffer */
  while (MapperPool_Item == 0)			
  pthread_cond_wait(&MapperPool_Empty, &MapperPool_Mutex);
  memset(&str[0],0,strlen(str)); 
  MapperPool_Item--;
  memcpy(&str1[0],&MapperPool_Buffer[MapperPool_out],sizeof(MapperPool_Buffer[MapperPool_out]));
  MapperPool_out = (MapperPool_out + 1) % Mapper_Size; 
  pthread_cond_signal(&MapperPool_Full);
  pthread_mutex_lock(&ReducerPool_Mutex);	/* protect buffer */
  while (ReducerPool_Item == Reducer_Size)			
    pthread_cond_wait(&ReducerPool_Full, &ReducerPool_Mutex);
  pch = strtok (str1," ");
  while (pch != NULL)
    {
        strcat(temp,"(");
        strcat(temp,pch);
        strcat(temp,",1) ");
        pch = strtok (NULL, " ,.-");
    }
	memcpy(&ReducerPool_Buffer[ReducerPool_In],&temp,sizeof(temp));
	ReducerPool_In = (ReducerPool_In+1)%Reducer_Size;
	ReducerPool_Item++;
	memset(&str1[0],0,strlen(str1));
	memset(&temp[0],0,strlen(temp));
	pthread_cond_signal(&ReducerPool_Empty);	/* wake up consumer */
	pthread_mutex_unlock(&MapperPool_Mutex);
    pthread_mutex_unlock(&ReducerPool_Mutex);	/* release the buffer */
}
pthread_exit(NULL);
}


void *Reducer(void *arg) {
	char str[1000],str1[1000],temp2[1000],dest[1000];
	char *pch;
	memset(&str[0],0,strlen(str));
	memset(&str1[0],0,strlen(str1));
	memset(&temp2[0],0,strlen(temp2));
	memset(&dest[0],0,strlen(dest));
	char str3[15];
    memset(&str3[0],0,strlen(str3));
	struct ListNode* head=NULL;
    while(1)
    {    
      pthread_mutex_lock(&ReducerPool_Mutex);	/* protect buffer */
      while (ReducerPool_Item == 0)			
      pthread_cond_wait(&ReducerPool_Empty, &ReducerPool_Mutex);
      memcpy(&str,&ReducerPool_Buffer[ReducerPool_Out],sizeof(ReducerPool_Buffer[ReducerPool_Out]));
      ReducerPool_Out = (ReducerPool_Out + 1) % Mapper_Size;  
      ReducerPool_Item--;   
      memcpy(&str1,&str,sizeof(str));
      memset(&str[0],0,strlen(str));
      pthread_cond_signal(&ReducerPool_Full);
      pthread_mutex_lock(&SummarizerPool_Mutex);	/* protect MapperPool_Buffer */
      while (SummarizerPool_Item == Summarizer_Size)			
      pthread_cond_wait(&SummarizerPool_Full, &SummarizerPool_Mutex);    
      i=0,j=0;
      memset(&temp2[0],0,strlen(temp2));
      char str4[1000];
	  while(str1[i]!='\0')
        { 
            if(str1[i]==')'||str1[i]=='('||str1[i]==','||str1[i]=='1')
				i++;
			else
				{
        	        str4[j]=str1[i];
        	        i++;
        	        j++;
				}
		}
	    str4[j]='\0';
		pch = strtok (str4," ");
        while (pch != NULL)
            {
             strcpy(dest,pch);     
             head=insert_list(head,dest);
             pch = strtok (NULL, " ,.-");
            }
        memset(&dest[0],0,strlen(dest));
        while(head!=NULL)
            {
            	strcat(dest,"(");
            	strcat(dest,head->word);
            	sprintf(str3, "%d", head->count);
            	strcat(dest,",");
            	strcat(dest,str3);
            	strcat(dest,") ");
    	        head=head->next;
			}
		memset(&str3[0],0,strlen(str3));
		memcpy(&SummarizerPool_Buffer[SummarizerPool_In],&dest,sizeof(dest));
		memset(&dest[0],0,strlen(dest));
		SummarizerPool_In = (SummarizerPool_In+1)%Summarizer_Size;
		SummarizerPool_Item++;
		memset(&str1[0],0,strlen(str1));
		memset(&temp2[0],0,strlen(temp2));
		pthread_cond_signal(&SummarizerPool_Empty);
		pthread_mutex_unlock(&ReducerPool_Mutex);
		pthread_mutex_unlock(&SummarizerPool_Mutex);	/* wake up consumer */
}
pthread_exit(0);
}


void *Summarizer(void *arg) {
	char s[1000],s1[1000],temp[1000],dest[1000];
	char *pch;
	memset(&s[0],0,strlen(s));
	memset(&s1[0],0,strlen(s1));
	memset(&temp[0],0,strlen(temp));
	memset(&dest[0],0,strlen(dest));          
    while(1)
    {    
      pthread_mutex_lock(&SummarizerPool_Mutex);	/* protect MapperPool_Buffer */
      while (SummarizerPool_Item == 0)			
      pthread_cond_wait(&SummarizerPool_Empty, &SummarizerPool_Mutex);
      memcpy(&s,&SummarizerPool_Buffer[SummarizerPool_Out],sizeof(SummarizerPool_Buffer[SummarizerPool_Out]));
      WordCount_Item++;
      pthread_cond_signal(&WordCount_Empty);
      SummarizerPool_Out = (SummarizerPool_Out + 1) % Mapper_Size;    
      memcpy(&s1,&s,sizeof(s));
      memset(&s[0],0,strlen(s));
      memcpy(&temp[0],&s1,sizeof(s1));
      SummarizerPool_Item--;
      while(WordCount_Item!=0)
   pthread_cond_wait(&WordCount_Full,&WordCount_Mutex);
  pthread_cond_signal(&SummarizerPool_Full);
  pthread_mutex_lock(&LetterCountTable_Mutex);	/* protect MapperPool_Buffer */
  while (letterCountTable_Item == LetterCount_Size)			
  pthread_cond_wait(&LetterCountTable_Full, &LetterCountTable_Mutex);
  int i=0;j=0;
  char te[1000]={0};
  memset(&te[0],0,strlen(te));
  char s2[1000]={0};
  while(temp[i]!='\0')
        { 
            if(temp[i]==')'||temp[i]=='('||temp[i]==',')
				{ 
					if(temp[i]==',')
				    {
				        s2[j]=' ';
                        j++;				    	  	
					}
					i++;
				}
				else
				   {
        	        s2[j]=temp[i];
        	        i++;
        	        j++;
				   }
		}
  s2[j]='\0';
  char c;
  pch = strtok (s2," ");
  int k=0,sum=0;
  while (pch != NULL)
    {
            	if(k%2==1)
            	{
				strcpy(dest,pch);
				sum=sum+dest[0]-'0';
				}
				if(k==0)
				  c=pch[0];
				k++;
             pch = strtok (NULL, " ");
    }
		 	memset(&s1[0],0,strlen(s1));
		    char te1[1000]={0};
            memset(&s2[0],0,strlen(s2));
		    memset(&te1[0],0,sizeof(te1));
            memset(&te[0],0,sizeof(te));
            sprintf(te1, "%d",sum);
            strcat(te,"(");
            te[strlen(te)]=c;
            strcat(te,",");
            strcat(te,te1);
            strcat(te,")");
            memcpy(&LetterCountTable[letterCountTable_In],&te,sizeof(te));
		 	letterCountTable_In = (letterCountTable_In+1)%LetterCount_Size;
		 	letterCountTable_Item++;
		 	memset(&te[0],0,sizeof(te));
		 	pthread_cond_signal(&LetterCountTable_Empty);
		 	pthread_mutex_unlock(&WordCount_Mutex);
            pthread_mutex_unlock(&SummarizerPool_Mutex);
			pthread_mutex_unlock(&LetterCountTable_Mutex);	/* wake up consumer */
    }
pthread_exit(0);
}

void *WordCountWriter(void *arg) {
  const char *filename = (char *)arg;
  char s[1000],s1[1000],temp[1000],dest[1000];
	char *pch;
	memset(&s[0],0,strlen(s));
	memset(&s1[0],0,strlen(s1));
	memset(&temp[0],0,strlen(temp));
	memset(&dest[0],0,strlen(dest));
	FILE *file1;           
    while(1)
    {    
      
      pthread_mutex_lock(&WordCount_Mutex);	/* protect MapperPool_Buffer */
      while (WordCount_Item == 0)		
	  	pthread_cond_wait(&WordCount_Empty, &WordCount_Mutex);
	  file1 = fopen(filename , "a");
      memcpy(&s,&SummarizerPool_Buffer[WordCount_Out],sizeof(SummarizerPool_Buffer[WordCount_Out]));
      WordCount_Out = (WordCount_Out + 1) % Mapper_Size;       
      memcpy(&s1,&s,sizeof(s));
      memset(&s[0],0,strlen(s));
      memcpy(&temp[0],&s1,sizeof(s1));
      pch = strtok (s1," ");
      while (pch != NULL)
            {
             fputs(pch,file1);
             fputs("\n",file1);
             pch=strtok(NULL," ");
            }
    fclose(file1);
   WordCount_Item--; 
  pthread_cond_signal(&WordCount_Full);
  pthread_mutex_unlock(&WordCount_Mutex);
  
}
pthread_exit(NULL);
}
void *LetterCountWriter(void *arg) {
  const char *filename = (char *)arg;
  char s[1000],s1[1000],temp[1000],dest[1000];
  char *pch;
  memset(&s[0],0,strlen(s));
  memset(&s1[0],0,strlen(s1));
  FILE *file1;           
  
  while(1)
  {   
  pthread_mutex_lock(&LetterCountTable_Mutex);
  while (letterCountTable_Item == 0)			
  pthread_cond_wait(&LetterCountTable_Empty, &LetterCountTable_Mutex);
  file1 = fopen(filename , "a");
  memcpy(&s,&LetterCountTable[letterCountTable_Out],sizeof(LetterCountTable[letterCountTable_Out]));
  letterCountTable_Out = (letterCountTable_Out + 1) % LetterCount_Size;  
  letterCountTable_Item--;      
  memcpy(&s1,&s,sizeof(s));
  memset(&s[0],0,strlen(s));
  pch = strtok (s1," ");
            while (pch != NULL)
            {
             fputs(pch,file1);
             fputs("\n",file1);
             pch=strtok(NULL," ");
            }
  memset(&s1[0],0,strlen(s1));
  fclose(file1);
  pthread_cond_signal(&LetterCountTable_Full);
  pthread_mutex_unlock(&LetterCountTable_Mutex);
}
pthread_exit(NULL);
}


/* Main Function */

int main(int argc, char **argv)
{
  if(argc!=5)
  {
  	printf(" Please Provide 4 arguments for this file \n");
  	exit(0);
  }
  int i = atoi(argv[2]);
  int j= atoi(argv[3]);
  int k= atoi(argv[4]);
  char WorCount_File[20]="wordCount.txt";
  char LetterCount_File[20]="letterCount.txt";
  FILE* file1 = fopen("wordCount.txt", "w+");
  fclose(file1);
  FILE* file2 = fopen("letterCount.txt", "w+");
  fclose(file2);
  pthread_mutex_init(&MapperPool_Mutex, NULL);	
  pthread_cond_init(&MapperPool_Empty, NULL);		
  pthread_cond_init(&MapperPool_Full, NULL);		
  pthread_mutex_init(&ReducerPool_Mutex, NULL);	
  pthread_cond_init(&ReducerPool_Empty, NULL);		
  pthread_cond_init(&ReducerPool_Full, NULL);	
  pthread_mutex_init(&SummarizerPool_Mutex, NULL);	
  pthread_cond_init(&SummarizerPool_Empty, NULL);		
  pthread_cond_init(&SummarizerPool_Full, NULL);	
  pthread_mutex_init(&LetterCountTable_Mutex, NULL);	
  pthread_cond_init(&LetterCountTable_Empty, NULL);		
  pthread_cond_init(&LetterCountTable_Full, NULL);	
  pthread_mutex_init(&WordCount_Mutex, NULL);	
  pthread_cond_init(&WordCount_Empty, NULL);		
  pthread_cond_init(&WordCount_Full, NULL);	
  pthread_t MapperPoolUpdater_thread,Mapper_thread[i],Reducer_thread[j],Summarizer_thread[k],LetterCountWriter_thread,WordCountWriter_thread;
  int rc = pthread_create(&MapperPoolUpdater_thread, NULL,MapperPoolUpdater,(void *)argv[1]);
  for(i;i>0;i--)
  	rc = pthread_create(&Mapper_thread[i],NULL,Mapper,NULL);
  for(j;j>0;j--)
  	rc = pthread_create(&Reducer_thread[j],NULL,Reducer,NULL);
  for(k;k>0;k--)
  	rc = pthread_create(&Summarizer_thread[k],NULL,Summarizer,NULL);
  rc = pthread_create(&WordCountWriter_thread,NULL,WordCountWriter,(void *)WorCount_File);
  rc = pthread_create(&LetterCountWriter_thread,NULL,LetterCountWriter,(void *)LetterCount_File);
  i = atoi(argv[2]);
  j= atoi(argv[3]);
  k= atoi(argv[4]);
  pthread_join(MapperPoolUpdater_thread,NULL);
  sleep(1);
  exit(0);
}

