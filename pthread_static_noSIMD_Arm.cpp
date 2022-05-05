#include<iostream>
using namespace std;
#include<pthread.h>
#include <sys/time.h>
#include<semaphore.h>

typedef struct{
   int t_id;
   float** A;
   int n;

}threadParam_t;

const int NUM_THREADS=7;

//信号量
sem_t sem_leader;
sem_t sem_Division[NUM_THREADS-1];
sem_t sem_Elimination[NUM_THREADS-1];

//线程函数定义
void *threadFunc(void *param){
    threadParam_t *p = (threadParam_t*)param;
    int t_id = p->t_id;
    float** A=p->A;
    int n=p->n;

    for(int k=0;k<n;k++){

        if(t_id == 0){
            //先由0号线程统一完成除法工作
            for(int j=k+1;j<n;j++){
                A[k][j]=A[k][j]/A[k][k];
            }
            A[k][k]=1.0;
        }
        else{
            sem_wait(&sem_Division[t_id-1]);
        }


        //t_id=0的线程唤醒其他工作线程，进行消去
        if(t_id==0){
            for(int i=0;i<NUM_THREADS-1;i++){
                sem_post(&sem_Division[i]);
            }
        }

        //划分循环任务 消去工作
        for(int i=k+1+t_id;i<n;i=i+NUM_THREADS){
            for(int j=k+1;j<n;j++){
                A[i][j]=A[i][j]-A[i][j]*A[k][j];
            }
            A[i][k]=0.0;
        }

        //所有线程一起进入下一轮
        if(t_id==0){

            for(int i=0;i<NUM_THREADS-1;i++)
                sem_wait(&sem_leader);

            for(int i=0;i<NUM_THREADS-1;i++)
                sem_post(&sem_Elimination[i]);
            }
        else{
                sem_post(&sem_leader);
                sem_wait(&sem_Elimination[t_id-1]);
        }
        }
        pthread_exit(NULL);


}


void display(float **A, int n){

        for(int i = 0;i<n; i++){
	          for(int j=0; j<n; j++)
		           {cout<<A[i][j]<<' ';}
                          cout<<endl;}

}

int main(){

    //初始化矩阵
   int n=4;
   for(n=4;n<=1024;n=n*2){
   cout<<n<<' ';

   struct timeval start;
   struct timeval end;
   gettimeofday(&start,NULL);

   for(int cy=0;cy<10;cy++){
   float** A = new float* [n];
   for(int i=0;i<n;i++)
         A[i]= new float[n];

   for(int i=0;i<n;i++)
      { for(int j=0;j<i;j++)
                 A[i][j]=0;
                 A[i][i]=1.0;
        for(int j=i+1;j<n;j++)
                  A[i][j]=rand();
       }
   for(int k=0;k<n;k++)
       for(int i=k+1;i<n;i++)
              for(int j=0;j<n;j++)
                        A[i][j]+=A[k][j];



    //初始化信号量
   sem_init(&sem_leader,0,0);
   for(int i=0;i<NUM_THREADS-1;i++){
   sem_init(&sem_Division[i],0,0);
   sem_init(&sem_Elimination[i],0,0);
   }

   //创建静态线程
   pthread_t handles[NUM_THREADS];//创建对应的handle参数
   threadParam_t param[NUM_THREADS]; //创建对应的线程数

   for(int t_id=0;t_id<NUM_THREADS;t_id++){
    param[t_id].t_id=t_id;
    param[t_id].A=A;
    param[t_id].n=n;
    pthread_create(&handles[t_id],NULL,threadFunc,(void *)&param[t_id]);
   }

   for(int t_id=0;t_id<NUM_THREADS;t_id++){
    pthread_join(handles[t_id], NULL);
   }

   sem_destroy(&sem_leader);
   for(int i=0;i<NUM_THREADS-1;i++){
   sem_destroy(&sem_Division[i]);
   sem_destroy(&sem_Elimination[i]);
   }
   }
   gettimeofday(&end,NULL);
   cout<<((long long)end.tv_sec-(long long)start.tv_sec)*1000000+((long long)end.tv_usec-(long long)start.tv_usec)<<endl;//微秒

   }
 return 0;}