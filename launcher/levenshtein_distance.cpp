#include <stdafx.h>
#include "LevhensteinDistance.h"

static int minimum(int a,int b,int c)
/*Gets the minimum of three values*/
{
  int min=a;
  if(b<min)
    min=b;
  if(c<min)
    min=c;
  return min;
}

static int *d=0;

int levenshtein_distance(const char *s,const char*t)
/*Compute levenshtein distance between s and t*/
{
  //Step 1
  int k,i,j,n,m,cost,distance;
  n=strlen(s); 
  m=strlen(t);
  if(n>100) n=100;
  if(m>100) m=100;
  if(n!=0&&m!=0)
  {
    if(!d)
        d=(int*)malloc(100*100);

    //d=(int*)malloc((sizeof(int))*(m+1)*(n+1));
    m++;
    n++;
    //Step 2	
    for(k=0;k<n;k++)
	d[k]=k;
    for(k=0;k<m;k++)
      d[k*n]=k;
    //Step 3 and 4	
    for(i=1;i<n;i++)
      for(j=1;j<m;j++)
	{
        //Step 5
        if(s[i-1]==t[j-1])
          cost=0;
        else
          cost=1;
        //Step 6			 
        d[j*n+i]=minimum(d[(j-1)*n+i]+1,        // deletion
                         d[j*n+i-1]+1,          // insertion
                         d[(j-1)*n+i-1]+cost);  // substiturion
      }
    distance=d[n*m-1];
    //free(d);
    return distance;
  }
  else 
    return -1; //a negative return value means that one or both strings are empty.
}