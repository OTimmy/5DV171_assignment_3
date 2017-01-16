#include <stdio.h>
#include <stdlib.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

#define NR_THREADS 20
#define NR_FILES_PER_THREAD 50
#define NR_WRITES 100000

#define NR_EASY_WRITES 1000
#define NR_NORMAL_WRITES 7000
#define NR_HARD_WRITES 9000

#define EASY_WRITE_SIZE 90
#define NORMAL_WRITE_SIZE 100
#define HARD_WRITE_SIZE 150

#define DIR_PATH "testfiles/"

static const char FILE_PATH[] = "testfiles/";
pthread_mutex_t lock;

FILE *files[NR_THREADS][NR_FILES_PER_THREAD];
pthread_t threads[NR_THREADS];
 

time_t time_creating = 0;
time_t time_opening  = 0;
time_t time_writing  = 0;
time_t time_closing  = 0;

time_t time_easy_write   = 0;
time_t time_normal_write = 0;
time_t time_hard_write   = 0;

int run_tests(); 
void setup();
//void *runbench(void *i);
void add_statistic();
void print_time();

char* create_filepath(int i, int j); 
void create_file(char *filepath);

void easy_writes(FILE *file);
time_t normal_writes(FILE *file);
time_t hard_writes(FILE *file);
void add_statistic(time_t t_create,time_t t_opening,time_t t_easy,time_t t_normal,time_t t_hard, time_t t_closing);
char *gen_string(int size); 
void test_creatingfiles();
void test_openingfiles();
void test_closingfiles();
void test_writingfiles();
void test_closingfiles();


void *runbench_createfiles(void *arg);
void *runbench_openingfiles(void *arg);
void *runbench_easy_writingtofiles(void *arg);
void *runbench_normal_writigntofiles(void *arg);
void *runbench_hard_writigntofiles(void *arg);
void *runbench_closingfiles(void *arg);
void *runbench_hard_writigntofiles(void *arg);

int main(int args, char *argv[]) 
{
    printf("Creating %d files\n",NR_THREADS*NR_FILES_PER_THREAD);
    //*files = (FILE **)malloc(sizeof(FILE*)*NR_THREADS*NR_FILES_PER_THREAD);
    setup();
    printf("Running test: opening,writing,closing\n");
    if(run_tests() > 0) 
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


int run_tests() 
{
    pthread_t threads[NR_THREADS];
    
    if(pthread_mutex_init(&lock, NULL) != 0) 
    {
        printf("Mutex failed");
        return 1;
    }
      
    test_creatingfiles();
    test_openingfiles();
    test_writingfiles();    
    test_closingfiles();        
         
    return 0;
    
}


void test_creatingfiles()
{
    time_t t1 = time(NULL);
    for(int i = 0; i < NR_THREADS; i++) 
    {
       int *val = malloc(sizeof(int));
       *val = i;
       pthread_create(&threads[i],NULL, runbench_createfiles, (void*) val);  
    }
    
    for(int i = 0; i < NR_THREADS; i++) 
    {
      pthread_join(threads[i],NULL);
    }
    time_t t2 = time(NULL);
    
    time_creating = t2 - t1;
    
}

void test_openingfiles()
{

    time_t t1 = time(NULL);
    for(int i = 0; i < NR_THREADS; i++) 
    {
       int *val = malloc(sizeof(int));
       *val = i;
       pthread_create(&threads[i],NULL, runbench_openingfiles, (void*) val);
    }
    
    for(int i = 0; i < NR_THREADS; i++) 
    {
      pthread_join(threads[i],NULL);
    }
    time_t t2 = time(NULL);
    
    time_opening = t2 - t1;
}

void test_writingfiles()
{
    time_t t1 = time(NULL);
    for(int i = 0; i < NR_THREADS; i++) 
    {
       int *val = malloc(sizeof(int));
       *val = i;
       pthread_create(&threads[i],NULL, runbench_easy_writingtofiles, (void*) val);
      
    }
    
    for(int i = 0; i < NR_THREADS; i++) 
    {
      pthread_join(threads[i],NULL);
    }
    time_t t2 = time(NULL);
    time_easy_write = t2 - t1; 
    
    
    t1 = time(NULL);
    for(int i = 0; i < NR_THREADS; i++) 
    {
       int *val = malloc(sizeof(int));
       *val = i;
       pthread_create(&threads[i],NULL, runbench_normal_writigntofiles, (void*) val);
      
    }
    
    for(int i = 0; i < NR_THREADS; i++) 
    {
      pthread_join(threads[i],NULL);
    }
    
    t2 = time(NULL);
    time_normal_write = t2 - t1;
    
    t1 = time(NULL);
    for(int i = 0; i < NR_THREADS; i++) 
    {
       int *val = malloc(sizeof(int));
       *val = i;
       pthread_create(&threads[i],NULL, runbench_hard_writigntofiles, (void*) val);
      
    }
    
    for(int i = 0; i < NR_THREADS; i++) 
    {
      pthread_join(threads[i],NULL);
    }
    t2 = time(NULL);
    time_hard_write = t2 - t1;
}


void test_closingfiles()
{
    time_t t1 = time(NULL);
    for(int i = 0; i < NR_THREADS; i++) 
    { 
       int *val = malloc(sizeof(int));
       *val = i;
       pthread_create(&threads[i],NULL, runbench_closingfiles, (void*) val);     
    }
    
    for(int i = 0; i < NR_THREADS; i++) 
    {
      pthread_join(threads[i],NULL);
    }
    time_t t2 = time(NULL);
    
    time_closing = t2 - t1;
}


void *runbench_createfiles(void *arg)
{
    int i = *(int*) arg;
    for( int j = 0; j < NR_FILES_PER_THREAD; j++)
    {
        char *filepath = create_filepath(i,j );
        create_file(filepath);
    }
    free(arg);
    
}

void *runbench_openingfiles(void *arg)
{
    int i = *((int*) arg);
    for( int j = 0; j < NR_FILES_PER_THREAD; j++)
    {
        FILE *file;
        char *filepath = create_filepath(i,j);
        file = fopen(filepath,"w");
        
        files[i][j] = file;
        
        if(file == NULL)
        {
            printf("Couldn't open file :(");
        }   
    }
    free(arg);
}

void *runbench_easy_writingtofiles(void *arg)
{
    int i = *((int*) arg);
    for(int j = 0; j < NR_FILES_PER_THREAD; j++)
    {
        FILE *file = files[i][0];
        easy_writes(file);           
    }
    free(arg);
}


void *runbench_normal_writigntofiles(void *arg)
{
    int i = *(int*) arg;
    for(int j = 0; j < NR_FILES_PER_THREAD; j++) 
    {
        normal_writes(files[i][j]);
    }
    free(arg);
}


void *runbench_hard_writigntofiles(void *arg)
{
    int i = *(int*) arg;
    for(int j = 0; j < NR_FILES_PER_THREAD; j++) 
    {
        hard_writes(files[i][j]);
    }
    free(arg);
}

void *runbench_closingfiles(void *arg)
{
    int i = *(int*) arg;
    for(int j = 0; j < NR_FILES_PER_THREAD; j++)
    {
        fclose(files[i][j]);
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
 *
 */
void create_file(char *filepath)
{
    FILE *file;
    file = fopen(filepath,"ab+");
              
    if(file == NULL) 
    {
       printf("oops couldn't create file: %s\n",filepath);
    }
            
    fclose(file);
     
}
 
/**
 * Returns the time for writing to the file. 
 */
void easy_writes(FILE *file)
{
    char *chars = gen_string(EASY_WRITE_SIZE);
 
        
    for(int i = 0; i < NR_EASY_WRITES; i++)
    {
        
        fprintf(file,"%s",chars);
    } 
    

    free(chars);
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
    
    
    time_t t1 = time(NULL);
    for(int i = 0; i < NR_NORMAL_WRITES; i++)
    {
        fprintf(file,"%s",chars);        
    }
   
    time_t t2 = time(NULL);
    free(chars); 

    time_t ret = t2 -t1;     
    return ret;
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
    
    
    time_t t1 = time(NULL);
    for(int i = 0; i < NR_HARD_WRITES; i++)
    {
            fprintf(file,"%s",chars);
    }
    time_t t2 = time(NULL);
    free(chars);
    
    time_t ret = t2 -t1;
    return ret;
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
void add_statistic(time_t t_create,time_t t_opening,time_t t_easy,time_t t_normal,time_t t_hard, time_t t_closing) 
{
    pthread_mutex_lock(&lock);
    time_creating      += t_create; 
    time_opening       += t_opening;
    time_easy_write    += t_easy;
    time_normal_write  += t_normal;
    time_hard_write    += t_hard; 
    time_closing       += t_closing;
    pthread_mutex_unlock(&lock);
}

void print_time()
{
    printf("Time creating: %lo\n",time_creating);
    printf("Time opening: %lo\n",time_opening);
    printf("Time writing %d characters: %lo\n",EASY_WRITE_SIZE*NR_EASY_WRITES, time_easy_write);
    printf("Time writing %d characters: %lo\n",NORMAL_WRITE_SIZE*NR_NORMAL_WRITES, time_normal_write);
    printf("Time writing %d characters: %lo\n",HARD_WRITE_SIZE*NR_HARD_WRITES, time_hard_write);  
    printf("Time closing: %lo\n", time_closing);
}
