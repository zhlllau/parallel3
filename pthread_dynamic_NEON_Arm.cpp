#include<iostream>
using namespace std;
#include<pthread.h>
#include <sys/time.h>
#include<time.h>
#include<arm_neon.h> 

//动态线程版本

//定义线程数据结构
typedef struct{
    int k; //消去的轮次
    int t_id; //线程id
    int act_row;
    int n;
    float** A;
}threadParam_t;



//定义线程函数
void *threadFunc(void *param){
    threadParam_t *p = (threadParam_t*)param;
    int n = p->n;
    float** A = p->A;
    int k = p->k;
    int t_id = p->t_id;
    int i = p->act_row;
    //int i = k+t_id+1; 获取自己的计算任务

    //NEON-消去部分并行化
    float32x4_t t1 ;
    float32x4_t t2 ;
    float32x4_t t3 ;
    float32x4_t t4 ;

    /*
    for(int j=k+1; j<n; j++){

        A[i][j] = A[i][j] - A[i][k] * A[k][j];
           }
    A[i][k] = 0;*/

    float tmp[4] = {A[i][k], A[i][k], A[i][k], A[i][k]};
    t1=vld1q_f32(tmp); 

    int j;
    for(j=n-4;j>=k;j=j-4){

             t2 = vld1q_f32(A[i] + j);
             t3 = vld1q_f32(A[k] + j);
             t4 = vsubq_f32(t2, vmulq_f32(t1, t3));
             vst1q_f32(A[i]+j, t4);
             }
    if((j+4)!=k)
      {
          for(int s=k; s<(j+4);s++)
              A[i][s] = A[i][s]-A[i][k]*A[k][s];

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

   //矩阵初始化
   int n ;
for(n=4;n<=1024;n=n*2){
   cout<<n<<' ';
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

    //动态线程版本
   struct timeval start;
   struct timeval end;
   gettimeofday(&start,NULL);

   for(int k=0; k<n; k++){
      for(int j=k+1; j<n; j++)
          //主线程做除法操作
          A[k][j] = A[k][j] / A[k][k];

      A[k][k]=1.0;
      //创建工作线程，进行消去操作
      int worker_count = n-1-k;

      if(worker_count>7){worker_count = 7;} //工作的线程数不超过8个
      int cowork_cycle;
      if(worker_count>0){
             cowork_cycle = (n-1-k)/worker_count;
      }
      int remain = n-1-k - cowork_cycle*worker_count;


      for(int c=0;c<cowork_cycle;c++){
          pthread_t* handles = new pthread_t[worker_count];//创建对应的handle参数
          threadParam_t *param = new threadParam_t[worker_count]; //创建对应的线程数据结构
          //分配任务
          for(int t_id = 0;t_id<worker_count; t_id++){
            param[t_id].k = k;
            param[t_id].t_id = t_id;
            param[t_id].A = A;
            param[t_id].n = n;
            param[t_id].act_row = k + c*worker_count + t_id + 1;
                                }
      //创建线程(核心函数)
      for(int t_id=0;t_id<worker_count;t_id++)
         pthread_create(&handles[t_id],NULL,threadFunc,&param[t_id]);
      for(int t_id = 0; t_id<worker_count;t_id++)
        //主线程挂起等待所有工作线程完成
        pthread_join(handles[t_id], NULL);

        }

      //处理剩余的remain行
      pthread_t* handles = new pthread_t[remain];//创建对应的handle参数
      threadParam_t *param = new threadParam_t[remain]; //创建对应的线程数据结构

      for(int t_id = 0;t_id<remain; t_id++){
        param[t_id].k = k ;
        param[t_id].t_id = t_id;
        param[t_id].A = A;
        param[t_id].n = n;
        param[t_id].act_row = cowork_cycle*worker_count + k + 1 + t_id;
     }
      //创建线程(核心函数)
      for(int t_id=0;t_id<remain;t_id++)
         pthread_create(&handles[t_id],NULL,threadFunc,&param[t_id]);
      for(int t_id = 0; t_id<remain;t_id++)
        //主线程挂起等待所有工作线程完成
        pthread_join(handles[t_id], NULL);

      }


      gettimeofday(&end,NULL);
      cout<<((long long)end.tv_sec-(long long)start.tv_sec)*1000000+((long long)end.tv_usec-(long long)start.tv_usec)<<endl;//微秒

      }

    return 0;}