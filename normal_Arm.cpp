#include<iostream>
using namespace std;
#include <sys/time.h>
#include<time.h>


//动态线程版本


void display(float **A, int n){

        for(int i = 0;i<n; i++){
	          for(int j=0; j<n; j++)
		           {cout<<A[i][j]<<' ';}
                          cout<<endl;}

}
float** normal(float **A, int n){
	//串行算法主体部分
for (int k=0;k<n;k++){

      //将第k行第k个元素全部变为1
      for(int j=k+1;j<n;j++)
           {
			A[k][j] = A[k][j]/A[k][k];
	       }
      A[k][k]=1.0;


      //对k+1到第n-1行进行高斯消元
      for(int i=k+1;i<n;i++){
           for(int j=k+1;j<n;j++)
                 A[i][j] = A[i][j]-A[i][k]*A[k][j];
       A[i][k]=0;
           }
         }

     return A;
}


int main(){

   //矩阵初始化
   int n ;
   for(n=4;n<=1024;n=n*2){
   cout<<n<<' ';
   struct timeval start;
   struct timeval end;
   gettimeofday(&start,NULL);

   for(int i=0;i<100;i++){
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


   //串行算法求解

   float** AA= normal(A,n);
    }
      gettimeofday(&end,NULL);
      cout<<((long long)end.tv_sec-(long long)start.tv_sec)*1000000+((long long)end.tv_usec-(long long)start.tv_usec)<<endl;//微秒
   }

return 0;
}