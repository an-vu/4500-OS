/*-----------------------------------------------------------------------*/
/* echo_me.c                                                             */
/*                                                                       */
/* This is an illustration program provided for the use of students in   */
/* CSCI 4500, Operating Systems, at the University of Nebraska at Omaha. */
/* Last modification: August 30, 2016.                                   */
/*-----------------------------------------------------------------------*/
#include <unistd.h>
#include <stdlib.h>

extern char **environ;

int main(int argc, char *argv[])
{
    /*--------------------------------------------------------------------*/
    /* The variable named 'argv' is initialized by the system when the    */
    /* program begins execution. If is an array of pointers to C-style    */
    /* character strings, followed by a null pointer. Each of the strings */
    /* contains one of the command line arguments, starting with the name */
    /* of the executable file. For example, if the command 'doit to it'   */
    /* was given to a command line interpreter (that is, a shell), and if */
    /* the program 'doit' exists, it will be executed and argv[0] will    */
    /* contain a pointer to the string "doit", argv[1] will contain a     */
    /* pointer to the string "to", and argv[2] will contain a pointer to  */
    /* the string "it". argv[3] will usually contain a null pointer.      */
    /* The integer 'argc' will contain the number of arguments. In the    */
    /* example just given, that would be 3.                               */
    /*--------------------------------------------------------------------*/

    /*--------------------------------------------------------------------*/
    /* The local variable 'args' is an array of pointers to the C-style   */
    /* character strings. These strings will represent the arguments to   */
    /* a program that will be executed by the process after the execve    */
    /* system call succeeds. The first string (pointed to by argv[0])     */
    /* identifies the command that will be executed. Also note that there */
    /* is a null pointer (with the value 0) following the last argument.  */
    /* Note that the args array is basically the same as the argv array   */
    /* in the new process after the execve system call succeeds.          */
    /*--------------------------------------------------------------------*/
    char *args[4];

    /*----------------------------------------------------------------------*/
    /* Setup the args array by storing pointers to the appropriate strings. */
    /* Remember that C-style string constants do three things: (1) they     */
    /* allocate sufficient (static) storage for the string, (2) they        */
    /* appropriately initialize the storage, and (3) they represent the     */
    /* address of the first character in the string to the program. So the  */
    /* first statement following will allocate 10 bytes of storage, store   */
    /* the character codes '/', 'b', 'i', 'n', '/', 'e', 'c', 'h', 'o' and  */
    /* the value 0 in those 10 bytes, and cause the address of the first    */
    /* '/' in the variable args[0]. The other statements are similar.       */
    /* The last statement assigns the value of the NULL pointer to args[3], */
    /* as expected by execve. The NULL pointer is just a pointer to memory  */
    /* location 0 (in most systems and languages).                          */
    /*----------------------------------------------------------------------*/
    args[0] = "/bin/echo";
    args[1] = "Hello,";
    args[2] = "world!";
    args[3] = 0;			/* this is a NULL pointer */

    /*--------------------------------------------------------------------*/
    /* The execve system call, if successful, will replace the code and   */
    /* data being used by the current process with the code and data from */
    /* the executable file specified by the string given as the first     */
    /* argument. In this case, that's the value of argv[0], which is the  */
    /* file named "/bin/echo". The array 'args' is also used to determine */
    /* the command line arguments passed to the main function of the      */
    /* program to be executed. The third argument points to an array of   */
    /* pointer to strings of the form 'NAME=value' which represent the    */
    /* value of the environment variables that the program may reference. */
    /*--------------------------------------------------------------------*/
    execve(args[0],args,environ);

    /*-----------------------------------------------------------------*/
    /* If execve returns, we know it failed. In that case, the program */
    /* just displays an appropriate error message and terminates.      */
    /*-----------------------------------------------------------------*/
    write(1,"execve failed.\n",15);
    _exit(1);		/* _exit: a system call */
			/* exit (without underscore): a library function */
			/* Use "man 3 exit" and "man 2 _exit" for details. */
}
