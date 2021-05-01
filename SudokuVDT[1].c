/**************************
 Program: Soduko validator 

 Author: Abdirahman Hassan

******************************/

#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>

#define NUMBEROFTHREADS 10 // total number of threads needed to check the soduko

/*The example Given in the assigment*/
int valid_sudoku[9][9]={
                {6,2,4,5,3,9,1,8,7},
                {5,1,9,7,2,8,6,3,4}, 
                {8,3,7,6,1,4,2,9,5},
                {1,4,3,8,6,5,7,2,9},
                {9,5,8,2,4,7,3,6,1},
                {7,6,2,3,9,1,4,5,8},
                {3,7,1,9,5,6,8,4,2},
                {4,9,6,1,8,2,5,7,3},
                {2,8,5,4,7,3,9,1,6}
             };

/*example of invalid soduko for testing*/
int invalid_sudoku[9][9]={
                {6,2,4,5,3,9,1,8,7},
                {5,1,4,7,2,8,6,3,4}, 
                {8,3,7,6,1,4,2,9,5},
                {1,4,3,8,6,5,7,2,9},
                {9,5,8,2,4,7,3,6,1},
                {7,6,2,3,9,1,4,5,8},
                {3,7,1,9,5,6,8,4,2},
                {4,9,6,1,8,2,1,7,3},
                {2,8,5,4,7,3,9,1,6}
             };
//********
//struct for passing data to pthreds
typedef struct {
    int row; // row
    int col; //colum
}parameters;

int result_holder[9]; // holds result for each thread
int thread_result_Iterator=0; // iterator to update valus in result_holder array 


/***************
checks if the row of the soduko is valid 
if the row is valid, the result_holder array at thread_result_iterator will be updated to 1
else 0
****************/
void *checkRows(void *s){
    parameters *info =(parameters *)s;
    
    int row=info->row; // row to checked
    int col=info->col; //  the colum
    for(int i= row;i<9;i++){ // loop for the row
        int validNumbers[10]={0}; // valid numbers holder 1 to 9    
             for(int j= col;j<9;j++){ // each colums 
                 
                 int solution= valid_sudoku[i][j]; // solution at row- colum
                 if(solution < 1 || solution > 9  || validNumbers[solution]==1){ //if the number got is greater 9 or less 1 or if  we have seen the number before
                        result_holder[thread_result_Iterator++]=0; //rows is not valid
                        pthread_exit(NULL);   
                    } 
                    else{ // if not seen, update the value
                        validNumbers[solution]=1;
                    }

                 }
             }
             result_holder[thread_result_Iterator++]=1; //rows valid
             pthread_exit(NULL);  

    

}

/***************
checks if the Column of the soduko is valid 
if the column is valid, the result_holder array at thread_result_iterator will be updated to 1
else 0
****************/
void *checkColumns(void *s){
    parameters *info =(parameters *)s;
    
    int row=info->row; // row to checked
    int col=info->col; //  the under colum
    for(int i= row;i<9;i++){ // loop for the row
        int validNumbers[10]={0}; // valid numbers holder   
             for(int j= col;j<9;j++){ // each colums 
                 
                 int solution= valid_sudoku[j][i]; // solution at column, row
                 if(solution <1 || solution > 9  || validNumbers[solution]==1){ //if the number got is greater 9 or less 1 or if  we have seen the number before
                        result_holder[thread_result_Iterator++]=0; //column not valid 
                        pthread_exit(NULL); 
                    } 
                    else{ // if not seen, update the value
                        validNumbers[solution]=1;
                    }

                 }
             }
            result_holder[thread_result_Iterator++]=1; //column valid 
            pthread_exit(NULL);  

}

/***************
checks if the subgrid of the soduko is valid 
if the subgrid is valid, the result_holder array at thread_result_iterator will be updated to 1
else 0
****************/
void *isSubGridsValid(void *s){
    parameters *info =(parameters *)s;
    
    int row=info->row; // row to checked
    int col=info->col; //  the under colum
    int validNumbers[10]={0}; // valid numbers holder
    for(int i= row;i<row+3;i++){ // loop for the row 
              
             for(int j= col;j<col+3;j++){ // each colums 
                 
                 int solution= valid_sudoku[i][j]; // solution at row- colum
                 if(solution <1 || solution > 9  || validNumbers[solution]!=0){ //if the number got is greater 9 or less 1 or if  we have seen the number before
                        result_holder[thread_result_Iterator++]=0;
                        pthread_exit(NULL); //rows valid  
                    } 
                    else{ // if not seen, update the value
                        validNumbers[solution]=1;
                    }

                 }
             }
             result_holder[thread_result_Iterator++]=1;
             pthread_exit(NULL); //subgrid valid

}
/*******************
Main

************************/

int main(){
    
    pthread_t allThreads[NUMBEROFTHREADS]; // array for all threads 
    
    int thread=0; // thread incremetor 
    
    //check all rows 
    parameters *rowInfo = (parameters *) malloc(sizeof(parameters));	
	rowInfo->row = 0;		
	rowInfo->col = 0;
	pthread_create(&allThreads[thread++], NULL,checkRows,(void *)rowInfo); // thread for the rows
    
    //check all colunms 
    parameters *colInfo = (parameters *) malloc(sizeof(parameters));	
	colInfo->row = 0;		
	colInfo->col = 0;
	pthread_create(&allThreads[thread++], NULL, checkColumns,(void *) colInfo); // column threads

    
    // checking the subgrids of the sudoku 
    for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
            if (i%3 == 0 && j%3 == 0) { //thread to validate subgrids
				parameters *subInfo = (parameters *) malloc(sizeof(parameters));	
				subInfo->row = i;		
				subInfo->col = j;
				pthread_create(&allThreads[thread++], NULL, isSubGridsValid, (void *)subInfo); // 3x3 subsection threads
			}
		}
	}

    //wait for threads to finish 
    for(int i=0;i<11;i++){
        pthread_join(allThreads[i],NULL);
    }
    
    // check for solution in the result holder
    for(int i=0;i<NUMBEROFTHREADS;i++){
        if(result_holder[i]==0){ // fails the test
            printf("The solution is not correct! \n");
            return 1;
        }
    }
    //passes all the test 
    printf("The solution is correct! \n");
    return 0;


}

