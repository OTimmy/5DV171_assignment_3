#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>

#define NR_THREADS 30
#define NR_FILES_PER_THREAD 20
#define NR_WRITES 100000

#define NR_EASY_WRITES 1000
#define NR_NORMAL_WRITES 7000
#define NR_HARD_WRITES 9000

#define EASY_WRITE_SIZE 50
#define NORMAL_WRITE_SIZE 100
#define HARD_WRITE_SIZE 150


static const char FILE_PATH[] = "testfiles/";
pthread_mutex_t lock;

time_t time_creating = 0;
time_t time_opening  = 0;
time_t time_writing  = 0;
time_t time_closing  = 0;

time_t time_easy_write   = 0;
time_t time_normal_write = 0;
time_t time_hard_write   = 0;

int runtest();
void setup();
void *runbench(void *i);
void add_statistic();
void print_time();

char* create_filepath(int i, int j); 
time_t create_file(char *filepath); 

time_t easy_writes(FILE *file);
time_t normal_writes(FILE *file);
time_t hard_writes(FILE *file);
void add_statistic(time_t t_opening,time_t t_easy,time_t t_normal,time_t t_hard, time_t t_closing);
char *gen_string(int size); 

int main(char argv[], int args) 
{
    printf("Creating %d files\n",NR_THREADS*NR_FILES_PER_THREAD);
    setup();
    printf("Running test: opening,writing,closing\n");
    if(runtest() > 0) 
    {
        return 1;
    }
    
    print_time();
}

/**
 * Creates folder for the test files. . 
 */
void setup()
{
    struct stat st = {0};
    
    if(stat(FILE_PATH,&st) == -1)
    {
        mkdir(FILE_PATH,0700);
    }
}


int runtest() 
{
    pthread_t threads[NR_THREADS];
    
    if(pthread_mutex_init(&lock, NULL) != 0) 
    {
        printf("Mutex failed");
        return 1;
    }
    
    int status = 0;
    for(int i = 0; i < NR_THREADS; i++) 
    {
       int *val = malloc(sizeof(int));
       *val = i;
       pthread_create(&threads[i],NULL, runbench, (void*) val);  
    }
       
       
    for(int i = 0; i < NR_THREADS; i++) 
    {
      pthread_join(threads[i],NULL);
    }
}


/**
 * open file, writes to file and 
 * add the time for each operation
 */
void *runbench(void *arg)
{
    int i = *(int*) arg;

    for( int j = 0; j < NR_FILES_PER_THREAD; j++) 
    {
        FILE *file;
        char *filepath = create_filepath(i,j);
        time_t t_create = create_file(filepath);
        
        time_t t1_open = time(0);
        file = fopen(filepath,"w");        
        time_t t2_open = time(0);
        time_t t_opening = t2_open - t1_open;
      
        
        free(filepath); 
      
        
        time_t t_easy   = easy_writes(file);
        time_t t_normal = normal_writes(file);
        time_t t_hard   = hard_writes(file);
       

       
        
        if( file == NULL)
        {
            printf("Couldn't open file\n");
            return NULL;
        } 

        time_t t1_close  = time(0);
        fclose(file);
        time_t t2_close  = time(0);
        
        time_t t_closing = t2_close - t1_close;        

        add_statistic(t_opening,t_easy,t_normal,t_hard,t_closing);    
        

    }   
    
    free(arg);
}

/**
 *
 */
char* create_filepath(int i, int j) 
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
 * Creates file at testfiles/
 * returns time for creating file. 
 */
time_t create_file(char *filepath)
{
    FILE *file;
     time_t t1 = time(0);
     file = fopen(filepath,"ab+");
     time_t t2 = time(0);
                 
     if(file == NULL) 
     {
        printf("oops couldn't create file: %s\n",filepath);
     }
            
     fclose(file);
     
     return t2 - t1;
}
 
/**
 * Returns the time for writing to the file. 
 */
time_t easy_writes(FILE *file)
{
    char *chars = gen_string(EASY_WRITE_SIZE);
    if(chars == NULL) 
    {
        return -1;
    }
    
    
    time_t t1 = time(0);
    
    for(int i = 0; i < NR_EASY_WRITES; i++)
    {
        fprintf(file,"%s",chars);
    } 
    
    time_t t2 = time(0);
    
    free(chars);
    return t2 -t1;
}

/**
 * Returns the time for writing to the file. 
 */
time_t normal_writes(FILE *file)
{
    char *chars = gen_string(NORMAL_WRITE_SIZE);
    
    if(chars == NULL) 
    {
        return -1;
    }
    
    
    time_t t1 = time(0);
    for(int i = 0; i < NR_NORMAL_WRITES; i++)
    {
        fprintf(file,"%s",chars);        
    }
   
    time_t t2 = time(0);
    free(chars); 
     
    return t2 - t1;
}

/**
 * Returns the time for writing to the file. 
 */
time_t hard_writes(FILE *file)
{
    char *chars = gen_string(HARD_WRITE_SIZE);
    
    if(chars == NULL) 
    {
        return -1;
    }
    
    
    time_t t1 = time(0);
    for(int i = 0; i < NR_HARD_WRITES; i++)
    {
            fprintf(file,"%s",chars);
    }
    time_t t2 = time(0);
    free(chars);
    return t2 - t1;
}

/**
 * Returns a string with given size. 
 */
char *gen_string(int size) 
{
    char *ret = malloc(sizeof(char)*size);
    
    if((ret = malloc(sizeof(char)*size)) == NULL)
    {
        return NULL;
    }
   
    for(int i = 0; i < size; i++) 
    {
        ret[i] = '\0';
    }
    
    for(int i = 0; i < size; i++) 
    {
        strcat(ret,"t");
    }
    
    return ret;
}


/**
 * Store time
 */  
void add_statistic(time_t t_opening,time_t t_easy,time_t t_normal,time_t t_hard, time_t t_closing) 
{
    pthread_mutex_lock(&lock);
    time_opening = time_opening + t_opening;
    time_easy_write    = time_easy_write   + t_easy;
    time_normal_write  = time_normal_write + t_normal;
    time_hard_write    = time_hard_write   + t_hard; 
    time_closing       = time_closing      + t_closing;
    pthread_mutex_unlock(&lock);
}

void print_time()
{
    printf("Time opening: %lo\n",time_opening);
    printf("Time writing %d characters: %lo\n",EASY_WRITE_SIZE*NR_EASY_WRITES, time_easy_write);
    printf("Time writing %d characters: %lo\n",NORMAL_WRITE_SIZE*NR_NORMAL_WRITES, time_normal_write);
    printf("Time writing %d characters: %lo\n",HARD_WRITE_SIZE*NR_HARD_WRITES, time_hard_write);  
    printf("Time closing: %lo\n", time_closing);
}
