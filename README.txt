Project 2
Mason Witcraft - mason.witcraft@ou.edu  
10/22/2018

My program acts as a shell, with some custom behavior on certain keywords. I tokenize the inputs, and compare the first token to a list of words. If the token matches any of the predefined words, then it behaves how I have defined, if the token does not match any of the predefined words, then it gets passed straight to the system for it to handle.

Notable changes with regard to normal shell functionality:
    help - prints contents of this README
    chdir (with no arguments) = pwd
Other functions simply behave like other system commands, listed below:
    wipe = clear
    filez = ls -1 (now allows user to specify other flags relevant to 'ls')
    mimic = cp (now allows recursive copy)
    erase = rm 
    morph = mv (now allows recursive move)
    chdir (with argument) = cd

Notable changes with regard to Project 1:
    -All system calls were changes from 'system()' to fork/exec calls so command was run in child process
        -Called 'execlp()' on wipe because I know the exact number of arguments
        -Called 'execvp()' on everything else because I did not know the number of arguments
            -Both are appended with 'p' so that it will automatically search the path
    -Morph and mimic can now copy or move entire directories if '-r' is supplied
        -Implemented this by using the 'nftw' function 
    -Also added custom 'mkdirz' and 'rmdirz' functions.
        -'mkdirz' takes the permissions of the source directory if there is one and applies it to the new directory
    -Prefixed the prompt with the current working directory
    -Added support for IO redirection for calls to commands that are not handled in my code so get passed on to system (except for 'filez' which does support IO redirection).
    

Compile by running 'make' or 'make all'
If issues arise, run 'make clean' then 'make all'
Executable can be run a few ways:
    1. Without piping in or supplying as an argument a batch file
        -This will run a shell that accepts user input and acts accordingly
    2. With supplying a batch file as an argument (./project2 batch)
        -This will run the shell with the commands outlined in batch, printing the input after the prompt and the output on line beneath prompt. Program exits on end of batch file.
        
Current bugs and issues/concerns:
    -Pretty sure piping in a file as stdin in while calling other commands will not work, but piping stdout to a file does.
    
Sources:
https://oudalab.github.io/cs3113fa18/projects/project1
    Used for miscellaneous information pertaining to this project and the project before
https://oudalab.github.io/cs3113fa18/projects/project2
    Used for miscellaneous information pertaining to this project
https://stackoverflow.com/questions/2218290/concatenate-char-array-in-c
    Used to learn how to concatenate char pointers in C
https://stackoverflow.com/questions/2180079/how-can-i-copy-a-file-on-unix-using-c
    Used to find out how to copy files using C
http://man7.org/linux/man-pages/man2/sendfile.2.html
    Used for information on sendfile function
https://www.geeksforgeeks.org/c-program-delete-file/
    Used to learn how to delete files in C
http://man7.org/linux/man-pages/man3/remove.3.html
    Used for information on remove function
https://stackoverflow.com/questions/7180293/how-to-extract-filename-from-path/19200767
    Used to learn how to get file name from path in C
http://man7.org/linux/man-pages/man3/basename.3.html
    Used for information on basename function
https://stackoverflow.com/questions/17438493/moving-a-file-on-linux-in-c
    Used to learn how to move a file in C
http://man7.org/linux/man-pages/man2/rename.2.html
    Used for information on rename function
https://stackoverflow.com/questions/1293660/is-there-any-way-to-change-directory-using-c-language
    Used to learn how to change working directory in C
http://man7.org/linux/man-pages/man2/chdir.2.html
    Used for information on chdir function
http://man7.org/linux/man-pages/man3/freopen.3p.html
    Used for information on freopen function
http://man7.org/linux/man-pages/man3/ftw.3.html
    Used for information on nftw function
https://stackoverflow.com/questions/5467725/how-to-delete-a-directory-and-its-contents-in-posix-c
    Used to learn how to use nftw to delete an entire directory
https://stackoverflow.com/questions/6383584/check-if-a-directory-is-empty-using-c-on-linux
    Used to learn how to check if a directory is empty in C
