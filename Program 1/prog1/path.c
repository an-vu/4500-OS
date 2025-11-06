/*-----------------------------------------------------*/
/* Display, one per line, the names of the directories */
/* found in the PATH environment variable.             */
/*-----------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*----------------------------------------------------*/
/* WARNING: IN NORMAL PROGRAMS, YOU SHOULD NOT ALTER  */
/* THE STRING POINTED TO BY THE VARIABLE NAMED path,  */
/* SINCE THIS IS A POINTER TO THE ACTUAL ENVIRONMENT  */
/* STRING FOR THE PROCESS. IF YOU CHANGE THIS STRING  */
/* (FOR EXAMPLE, USE STRTOK ON IT), A LATER CALL of   */
/* getenv WILL RETURN A POINTER TO THE *MODIFIED*     */
/* COPY OF THE ENVIRONMENT, NOT THE ORIGINAL!         */
/*----------------------------------------------------*/
/* This program (i.e. the process created to execute  */
/* it) only uses the value returned by getenv("PATH") */
/* once, so it's okay to modify the value -- which    */
/* happens here when the strtok function is applied   */
/* to the string named path. You should also look at  */
/* the C source code in the files pathdup1.c and      */
/* pathdup2.c to see several ways a copy of the path  */
/* might be made and used.                            */
/*----------------------------------------------------*/
/* Note that this program only allocates storage for  */
/* two pointers to a character string. The string     */
/* itself is the actual storage used for the process' */
/* environment variables.                             */
/*----------------------------------------------------*/

int main(void)
{
    char *path, *dir;

    /*-------------------------------------------------*/
    /* Get the value of the PATH environment variable. */
    /* We assume here that this succeeds, but it is    */
    /* normally good practice to check the result.     */
    /*-------------------------------------------------*/
    path = getenv("PATH");

    /*---------------------------------------------------*/
    /* Parse the string and display the directory names. */
    /*---------------------------------------------------*/
    printf("Directories in the PATH environment variable:\n");
    dir = strtok(path,":");     /* get first entry -- modified path! */
    while (dir != NULL) {	    /* while we got an entry... */
        printf("%s\n",dir);         /* display the directory name */
        dir = strtok(NULL,":");     /* and get the next entry */
    }

    /*--------------------------------------------------*/
    /* To demonstrate the original was been changed, we */
    /* display a second list of directories in PATH.    */
    /*--------------------------------------------------*/
    path = getenv("PATH");
    printf("\nTo demonstrate we actually changed the process environment\n");
    printf("by \"fiddling\" with it using strtok...\n");
    printf("\nSecond list of directories:\n");
    dir = strtok(path,":");    
    while (dir != NULL) {           
        printf("%s\n",dir);         
        dir = strtok(NULL,":");     
    }

    exit(0);
}
