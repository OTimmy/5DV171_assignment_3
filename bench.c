#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>

#define NUMB_THREADS 30
#define NUMB_FILES_PER_THREAD 20
#define NUMB_WRITES 100000


static const char FILE_PATH[] = "testfiles/";
pthread_mutex_t lock;

time_t time_opening = 0;
time_t time_writing = 0;
time_t time_closing = 0;

int runtest();
void setup();
void clean();
void *runbench(void *i);
void add_statistic();
void print_time();
char* createFilePath(int i, int j);
void add_statistic(time_t t_opening,time_t t_writing,time_t t_closing);


int main(char argv[], int args) 
{
    printf("Creating %d files\n",NUMB_THREADS*NUMB_FILES_PER_THREAD);
    setup();
    printf("Done creating files\n");
    printf("Running test: opening,writing,closing\n");
    if(runtest() > 0) 
    {
        return 1;
    }
    
    print_time();
    clean(); 
}

int runtest() 
{
    pthread_t threads[NUMB_THREADS];
    
    if(pthread_mutex_init(&lock, NULL) != 0) 
    {
        printf("Mutex failed");
        return 1;
    }
    
    int status = 0;
    for(int i = 0; i < NUMB_THREADS; i++) 
    {
       int *val = malloc(sizeof(int));
       *val = i;
       pthread_create(&threads[i],NULL, runbench, (void*) val);  
    }
       
       
    for(int i = 0; i < NUMB_THREADS; i++) 
    {
      pthread_join(threads[i],NULL);
    }
}

/**
 * Creates files for each thread. 
 */
void setup()
{
    struct stat st = {0};
    
    if(stat(FILE_PATH,&st) == -1)
    {
        mkdir(FILE_PATH,0700);
    }

    FILE *file;
    for(int i = 0; i < NUMB_THREADS; i++)
    {
        for(int j = 0; j < NUMB_FILES_PER_THREAD; j++) 
        {
            char *filepath = createFilePath(i,j);
            file = fopen(filepath,"ab+");
            
            if(file == NULL) 
            {
                printf("oops couldn't create file: %s\n",filepath);
            }
            
            fclose(file); 
        }
    }   
    
}

/**
 * Remove all files. 
 */
void clean()
{
    for(int i = 0; i < NUMB_THREADS; i++) 
    {
        for(int j = 0; j < NUMB_FILES_PER_THREAD; j++)
        {
            //remove files
        }
    }
}

/**
 * open file, writes to file and 
 * add the time for each operation
 */
void *runbench(void *arg)
{
    int i = *(int*) arg;

    for( int j = 0; j < NUMB_FILES_PER_THREAD; j++) 
    {
        FILE *file;
  
        time_t t1_opening = time(0);
        
        char *filepath = createFilePath(i,j);
        file = fopen(filepath,"w");        
        if(file == NULL )
        {
          printf("Failed to open file: %s", filepath);
        }
        time_t t2_opening = time(0);
        
        time_t t1_writing = time(0);
        for(int k = 0; k < NUMB_WRITES; k++ ) 
        {
            fprintf(file,"writing stuffwriting stuffwriting stuffwriting stuff");        
        }
        time_t t2_writing = time(0);
        time_t t1_closing = time(0);
        fclose(file);
        time_t t2_closing = time(0);
                
        add_statistic((t2_opening-t1_opening),(t2_writing-t1_writing),(t2_closing-t1_closing));
    }   
    
    free(arg);
}

/**
 *
 */
char* createFilePath(int i, int j) 
{
    char a = (char) (i+'0');
    char b = (char) (j+'0');
            
    char name[3];
            
    name[0] = a;
    name[1] = b;
    name[2] = '\0';
            
    char *filepath = malloc(sizeof(char)*30);
            
    for(int i = 0; i < 30; i ++) {
        filepath[i] = '\0';
    }
            
    strcat(filepath,FILE_PATH);
    strcat(filepath,name);
    
    return filepath;
}

/**
 * Store time
 */  
void add_statistic(time_t t_opening,time_t t_writing, time_t t_closing) 
{
    pthread_mutex_lock(&lock);
    time_opening = time_opening + t_opening;
    time_writing = time_writing + t_writing;
    time_closing = time_closing + t_closing;
    pthread_mutex_unlock(&lock);
}

void print_time()
{
    printf("Time opening: %lo\n",time_opening);
    printf("Time writing: %lo\n",time_writing);
    printf("Time closing: %lo\n", time_closing);
}
