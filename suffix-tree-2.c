#define MAX_CHAR 1000 
#define NO_DOCS 308 

#include <stdio.h> 
#include <string.h> 
#include <stdlib.h> 
#include <time.h>
#include <limits.h>


/* ------------------------------------------------Timing Function------------------------------------------------ */
double Exec_Time(struct timespec start, struct timespec end)
{
     double t;
     t = (end.tv_sec-start.tv_sec)+(end.tv_nsec - start.tv_nsec) * 0.000000001; //diff in nanoseconds
     return t;
}

/* ----------------------------Build Generalized Suffix Tree Using Ukkonen's Algorithm---------------------------- */   
struct SuffixTreeNode 
{ 
    struct SuffixTreeNode *children[MAX_CHAR]; 
    struct SuffixTreeNode *suffixLink; 
    int start; 
    int *end; 
    int suffixIndex; 
}; 

char **docHeads;
int *relevanceIndex;
int *offsets;   
typedef struct SuffixTreeNode Node; 

int wordCount = 0;
int *text;   
Node *root = NULL;  
   
Node *lastNewNode = NULL; 
Node *activeNode = NULL; 
   
int activeEdge = -1; 
int activeLength = 0; 
   
int remainingSuffixCount = 0; 
int leafEnd = -1; 
int *rootEnd = NULL; 
int *splitEnd = NULL; 
int size = -1; 
int noOfDocs = 0;
int textLen  = 0;
   
Node *newNode(int start, int *end) 
{ 
    Node *node =(Node*) malloc(sizeof(Node)); 
    int i; 
    for (i = 0; i < MAX_CHAR; i++) 
          node->children[i] = NULL; 
   
    node->suffixLink = root; 
    node->start = start; 
    node->end = end; 

    node->suffixIndex = -1; 
    return node; 
} 
   
int edgeLength(Node *n) 
{ 
    if(n == root)
    { 
      return 0;
    } 
    return *(n->end) - (n->start) + 1; 
} 
   
int walkDown(Node *currNode) 
{ 
    if (activeLength >= edgeLength(currNode)) 
    { 
        activeEdge += edgeLength(currNode); 
        activeLength -= edgeLength(currNode); 
        activeNode = currNode; 
        return 1; 
    } 
    return 0; 
} 
   
void extendSuffixTree(int pos)
{
  leafEnd = pos;
  remainingSuffixCount++;

  lastNewNode = NULL;

  while(remainingSuffixCount > 0) 
  {

    if (activeLength == 0)
    {
      activeEdge = pos;
    }

    if (activeNode->children[text[activeEdge]] == NULL)
    {

      activeNode->children[text[activeEdge]] = newNode(pos, &leafEnd);
      if (lastNewNode != NULL)
      {
        lastNewNode->suffixLink = activeNode;
        lastNewNode = NULL;
      }
    }
    else
    {
      Node *next = activeNode->children[(int)text[activeEdge]];
      if (walkDown(next))
      {
        continue;
      }
      if (text[next->start + activeLength] == text[pos])
      {
        if(lastNewNode != NULL && activeNode != root)
        {
          lastNewNode->suffixLink = activeNode;
          lastNewNode = NULL;
        }

        activeLength++;
        break;
      }
      splitEnd = (int*)malloc(sizeof(int));
      *splitEnd = next->start + activeLength - 1;

      Node *split = newNode(next->start, splitEnd);
      activeNode->children[(int)text[split->start]] = split;

      split->children[(int)text[pos]] = newNode(pos, &leafEnd);
      next->start += activeLength;
      split->children[(int)text[next->start]]= next;
      if (lastNewNode != NULL)
      {
        lastNewNode->suffixLink = split;
      }
      lastNewNode = split;
    }
    remainingSuffixCount--;
    if (activeNode == root && activeLength > 0)
    {
      activeLength--;
      activeEdge = pos - remainingSuffixCount + 1;
    }
    else if (activeNode != root)
    {
      activeNode = activeNode->suffixLink;
    }
  }
}

void print(int i, int j) 
{ 
    int k; 
    for (k=i; k<=j && text[k] < 256; k++) 
        printf("%c ", text[k]); 
    if(k<=j) 
        printf("%d ", text[k]); 
} 

void setSuffixIndexByDFS(Node *n, int labelHeight) 
{ 
    if (n == NULL)  return; 

    int leaf = 1; 
    int i; 
    for (i = 0; i < MAX_CHAR; i++) 
    { 
        if (n->children[i] != NULL) 
        {
            leaf = 0; 
            setSuffixIndexByDFS(n->children[i], labelHeight + edgeLength(n->children[i])); 
        } 
    } 
    if (leaf == 1) 
    { 
        for(i= n->start; i<= *(n->end); i++) 
        { 
            if(text[i] > 255) 
            { 
                n->end = (int*) malloc(sizeof(int)); 
                *(n->end) = i; 
            } 
        } 
        n->suffixIndex = size - labelHeight; 
    } 
} 
   
void freeSuffixTreeByPostOrder(Node *n) 
{ 
    if (n == NULL)
    { 
       return;
    }

    int i; 
    for (i = 0; i < MAX_CHAR; i++) 
    { 
        if (n->children[i] != NULL) 
        { 
            freeSuffixTreeByPostOrder(n->children[i]); 
        } 
    } 
    if (n->end != NULL)
    { 
        free(n->end);
        n->end = NULL;
    }
    free(n); 
} 
   
void buildSuffixTree() 
{ 
    size = textLen; 
    int i; 
    rootEnd = (int*) malloc(sizeof(int)); 
    *rootEnd = - 1; 
   
    root = newNode(-1, rootEnd); 
   
    activeNode = root;
    for (i=0; i<size; i++)
    { 
        extendSuffixTree(i); 
    }
    int labelHeight = 0; 
    setSuffixIndexByDFS(root, labelHeight); 
} 

/* -----------------------------------------------Searching the Tree----------------------------------------------- */

int traverseEdge(char *str, int idx, int start, int end)
{
  int k = 0; 
  for(k=start ;k<=end && str[idx] != '\0'; k++, idx++)
  {
    if(text[k] != (int)str[idx])
    {
      return -1;
    }
  }
  if(str[idx] == '\0')
  {
    return 1;
  }

  return 0;
}

int doTraversalToCountLeaf(Node *n, int factor)
{
  if(n == NULL)
  {
    return 0;
  }

  if(n->suffixIndex > -1)
  {
    int i = 0, r = 0; 
    for(i=1; i<noOfDocs; i++)
    {
       if((n->suffixIndex < offsets[i]) && (n->suffixIndex >= offsets[i-1]))
       {
          r = relevanceIndex[i-1]%1000;
          if((relevanceIndex[i-1]-r)/1000 > (n->suffixIndex - offsets[i-1]))
          {
               r += factor;
               relevanceIndex[i-1] = r + ((n->suffixIndex - offsets[i-1]) * 1000);
          }
          else
          {
               relevanceIndex[i-1] += factor;
          }
          return 1;
       }
    }
    i--;
    if(n->suffixIndex == offsets[i])
    {
       r = relevanceIndex[i-1]%1000;
       if((relevanceIndex[i-1]-r)/1000 > (n->suffixIndex - offsets[i-1]))
       {
               r += factor;
               relevanceIndex[i-1] = r + ((n->suffixIndex - offsets[i-1]) * 1000);
       }
       else
       {
               relevanceIndex[i-1] += factor;
       }
       return 1;
    }
    return 1;
  }

  int count = 0;
  int i = 0;
  for (i = 0; i < MAX_CHAR; i++)
  {
    if(n->children[i] != NULL)
    {
      count += doTraversalToCountLeaf(n->children[i], factor);
    }
  }
  return count;
}

int countLeaf(Node *n, int factor)
{
  if(n == NULL)
  {
    return 0;
  }

  return doTraversalToCountLeaf(n, factor);
}

int doTraversal(Node *n, char* str, int idx, int factor)
{
  if(n == NULL)
  {
    return -1;
  }
  int res = -1;
  if(n->start != -1)
  {
    res = traverseEdge(str, idx, n->start, *(n->end));
    if(res == -1)
    {
      return -1;
    }
    if(res == 1)
    {
       if(n->suffixIndex > -1)
       {
          wordCount++;
          int i = 0, r = 0; 
          for(i=1; i<noOfDocs; i++)
          {
            if((n->suffixIndex < offsets[i]) && (n->suffixIndex >= offsets[i-1]))
            {
               r = relevanceIndex[i-1]%1000;
               if((relevanceIndex[i-1]-r)/1000 > (n->suffixIndex - offsets[i-1]))
               {
                    r += factor;
                    relevanceIndex[i-1] = r + ((n->suffixIndex - offsets[i-1]) * 1000);
               }
               else
               {
                    relevanceIndex[i-1] += factor;
               }
               return 1;
            }
          }
          i--;
          if(n->suffixIndex == offsets[i])
          {
            r = relevanceIndex[i-1]%1000;
            if((relevanceIndex[i-1]-r)/1000 > (n->suffixIndex - offsets[i-1]))
            {
                    r += factor;
                    relevanceIndex[i-1] = r + ((n->suffixIndex - offsets[i-1]) * 1000);
            }
            else
            {
                    relevanceIndex[i-1] += factor;
            }
            return 1;
          }
          return 1;
       }
      
       else      
       { 
          wordCount+= countLeaf(n, factor);
       }
       return 1;
    }
  }

  idx = idx + edgeLength(n);
  if(strlen(str)==1)
  {
    return doTraversal(n->children[(int)str[0]], str, 0, factor);
  }
  else if(n->children[(int)str[idx]] != NULL)
  {
    return doTraversal(n->children[(int)str[idx]], str, idx, factor);
  }
  else
  {
    return -1;
  }
}

/* ---------------------------------------------Reading the Input Files--------------------------------------------- */

void addDoc(char *input)
{
  int len = strlen(input);
  offsets[noOfDocs] = textLen;
  if(noOfDocs == 0)
  {
    text = (int*)calloc(len+1, sizeof(int));
    textLen += len+1;
    for(int i=0; i<len; i++)
    {
      text[i] = (int)input[i];
    }
    text[textLen-1] = 256;
    noOfDocs++;
  }
  else
  {
    text = (int*)realloc(text, sizeof(int)*(textLen+len+1));
    for(int i=textLen; i<textLen+len; i++)
    {
      text[i] = (int)input[i-textLen];
    }
    textLen += len+1;
    text[textLen-1] = 256 + noOfDocs;
    noOfDocs++;
  } 
}

void readDocs()
{
     int flag;
     FILE *file = fopen("AesopTales.txt", "r");
     fseek(file, 0, SEEK_END);
     long int fsize = ftell(file);
     fseek(file, 0, SEEK_SET);
     char *allStories = (char*)malloc(sizeof(char)*(fsize+1));
     fread(allStories, fsize, 1, file);
     fclose(file);
     
     int i=0;
     while(i<fsize)
     {
          if(i+2<fsize)
          {
               if((allStories[i]=='\n' || allStories[i]==' ') & (allStories[i+1]=='\n') & (allStories[i+2]=='\n'))
               {
                    allStories[i] = '$';
                    allStories[i+1] = '$';
                    allStories[i+2]= '$';
               }
          }
          i++;
     }

     const char* delim = "$";
     char *token = strtok(allStories, delim);
     while(token!=NULL)
     {
          
          if(strlen(token)>70)
          {
               i = 0;
               flag=0;
               while((token[i]!= '\n'))
               {
                    i++;
               }

               if(i<2)
               {
                    token[i] = ' ';
                    i++;
                    flag=1;
                    while((token[i]!= '\n'))
                    {
                         i++;
                    }
               }
               docHeads[noOfDocs] = (char*)calloc(i+1, sizeof(char));
               memset(docHeads[noOfDocs], '\0', i+1);
               char *X = token;
               while(X[0]==' ')
               {
                    X++;
               }
               strncpy(docHeads[noOfDocs], X, i-flag-1);
               addDoc(token);
               //printf("Token%s\n", token);
          }


          token = strtok(NULL, delim);
     }
     printf("No Of Stories: %d\n", noOfDocs);
     free(allStories);
} 


/* ------------------------------------------------------Relevance------------------------------------------------- */

char compareBasedOnRelavance(int i, int j)
{
     return ((relevanceIndex[i]%1000) > (relevanceIndex[j])%1000);
}

void sortBasefOnRelavance(int n)
{
     int *relevantDocs = (int*)calloc(NO_DOCS, sizeof(int));
     for(int i =0; i<NO_DOCS; i++)
     {
          relevantDocs[i] = i;
     }

     sort(relevantDocs, relevantDocs+NO_DOCS, compareBasedOnRelavance);
     for(int i=0; i<n; i++)
     {
     	if((relevanceIndex[relevantDocs[i]]%1000) > 0)
     	{
          printf("%s, %d\n", docHeads[relevantDocs[i]], relevanceIndex[relevantDocs[i]]%1000);
     	}
     }
     free(relevantDocs);
}

int isLetter(char c)
{
     if ((c>='a'&& c<='z') || (c>='A'&& c<='Z'))
     {
        return 1;
     }
     return 0;
}

void removePunc(char *searchString)
{
     int i, j=0, len = strlen(searchString);
     for (i =0; i<len; i++)
     {
        if (isLetter(searchString[i]))
        {
            searchString[j] = searchString[i];
            j++;
        }
        else if (!isLetter(searchString[i]) && (i+1<len))
        {
          if(searchString[i+1]!=' ')
          {
            searchString[j] = ' ';
            j++;
          }
        }       
     }
     
     if(j < len)
     {
          searchString[j] = '\0';
     }
}

static int *computePrefix(char *p, int *Pi)
{
     int m = strlen(p);
     int k = -1, q=0;
     Pi[0]=-1;
     for(q=1; q<m; q++)
     {
          while((k>-1) && (p[k+1]!=p[q]))
          {
               k=Pi[k];
          }
          if(p[k+1]==p[q])
          {
               k=k+1;
          }
          Pi[q]=k;
     }
     return Pi;
}

int KMP(char *text, char *pattern)
{
     int L_Text = strlen(text);
     int L_Pattern = strlen(pattern);
     int *Pi = (int*)malloc(L_Pattern*sizeof(int));
     computePrefix(pattern, Pi);
     int q = -1, i, n=0;
     for(i=0; i<L_Text; i++)
     {
          while((q>-1) && (pattern[q+1]!=text[i]))
          {
               q = Pi[q];
          }
          if(pattern[q+1]==text[i])
          {
               q=q+1;
          }
          if(q==L_Pattern-1)
          {
               return 1;
          }
     }
     free(Pi);
     Pi = NULL;
     return n;
}

int checkForSpace(char *X)
{
   for(int i=0; X[i]!= '\0'; i++)
   {   
     if (X[i] == ' ')
     {
        return 1;
     }
   }
   return 0;
}

void checkRelevanceOfDocs(char *str, int n)
{
     int limit = INT_MAX - INT_MAX%1000;
     char *orig = (char*)malloc((strlen(str)+1)*sizeof(char));
     strcpy(orig, str);
     int len = strlen(str);
     removePunc(str);
     doTraversal(root, str, 0, len*2);

     if(checkForSpace(str))
     {
          char *token = strtok(str, " ");
          doTraversal(root, token, 0, strlen(token));
          while(token!=NULL)
          {
               
               if(token!=NULL)
               {
                    doTraversal(root, token, 0, strlen(token));
               }
               token = strtok(NULL, " ");
          }

     }
     doTraversal(root, orig, 0, len*2+1);
     printf("\n-------------------------First Occurance-------------------------\n");
     for(int i=0; i<noOfDocs; i++)
     {
          if(KMP(docHeads[i], str))
          {
               relevanceIndex[i] += len/2;
          }
          if(relevanceIndex[i]<limit)
          {
               printf("(%s, %d)\n", docHeads[i], relevanceIndex[i]/1000);
          }
     }
     printf("\n\n----------------------------Relevance----------------------------\n");
     sortBasefOnRelavance(n);
     free(orig);
}


/* --------------------------------------------------------Main----------------------------------------------------- */ 

int main(int argc, char *argv[]) 
{ 
    int limit = INT_MAX - INT_MAX%1000;
    //printf("Limit: %d\n", limit); 
    docHeads = (char**)calloc(NO_DOCS, sizeof(char*));
    offsets = (int*)calloc(NO_DOCS, sizeof(int));
    relevanceIndex = (int*)calloc(NO_DOCS, sizeof(int));
    
    for(int i=0; i< NO_DOCS; i++)
    {
      relevanceIndex[i] = limit;
    }
    struct timespec start, end;

    clock_gettime(CLOCK_REALTIME, &start); 
    readDocs();
    buildSuffixTree();
    char *input = (char*)malloc(100);
 	int n = NO_DOCS;
 	scanf("%[^\n]", input);
 	
 	checkRelevanceOfDocs(input, n);
    clock_gettime(CLOCK_REALTIME, &end);
    printf("\n------------------------Time Taken For Exec------------------------\n");
    double time=Exec_Time(start, end);
    printf("%lf\n", time);
    
    freeSuffixTreeByPostOrder(root);
    free(text);
    for(int i=0; i<noOfDocs; i++)
    {
       free(docHeads[i]);
    }
    
    free(docHeads);
    free(offsets);
    free(relevanceIndex);
    free(input);
    return 0;     
} 