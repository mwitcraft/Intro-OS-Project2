#define _XOPEN_SOURCE 500
   #include <ftw.h>
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include <stdint.h>
   #include <dirent.h>

  int display_info(const char *fpath, const struct stat *sb,
   int tflag,struct FTW *ftwbuf)
   {
     // Given code from man7 page
    // printf("%-3s %2d %7jd   %-40s %d %s\n",
    // (tflag == FTW_D) ?   "d"   : (tflag == FTW_DNR) ? "dnr" :
    // (tflag == FTW_DP) ?  "dp"  : (tflag == FTW_F) ?   "f" :
    // (tflag == FTW_NS) ?  "ns"  : (tflag == FTW_SL) ?  "sl" :
    // (tflag == FTW_SLN) ? "sln" : "???",
    // ftwbuf->level, (intmax_t) sb->st_size,
    // fpath, ftwbuf->base, fpath + ftwbuf->base);
    // return 0;           /* To tell nftw() to continue */


    // My code
    int isDir = 0;
    if(tflag == FTW_D)
      isDir = 1;
    if(isDir){
      printf("%s\tDir\n", fpath);
    }else{
      printf("%s\tNotDir\n", fpath);
    }
    return 0;
    }

  int isDirectoryEmpty(char *dirname) {
  int n = 0;
  struct dirent *d;
  DIR *dir = opendir(dirname);
  if (dir == NULL) //Not a directory or doesn't exist
    return 1;
  while ((d = readdir(dir)) != NULL) {
    ++n;
    if(n > 2)
      break;
  }
  closedir(dir);
  if (n <= 2) //Directory Empty
    return 1;
  else
    return 0;
}

   int main(int argc, char *argv[])
   {
   int flags = 0;

   if(isDirectoryEmpty(argv[1])){
     printf("Empty Directory\n");
     exit(0);
   }

  if (nftw(argv[1], display_info, 20, flags) == 0) {
          printf("Tree exhausted\n");
   }
   exit(EXIT_SUCCESS);
  }
