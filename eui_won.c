#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

int ew_getargs(char *cmd, char **argv);
void ew_exit();
void ew_cd(int argc, char* argv[]);
void ew_mkdir(int argc, char *argv[]);
void ew_rmdir(int argc, char *argv[]);
void ew_cp(int argc, char *argv[]);
void ew_rm(int argc, char *argv[]);
void ew_mv(int argc, char *argv[]);
void ew_cat(int argc, char *argv[]);

void main(){
    char buf[256];
    char *argv[50];
    int narg;
    pid_t pid;

    while(1){
        printf("shell> ");
        gets(buf);
        narg = ew_getargs(buf, argv);
        pid = fork();

        if(pid == 0)
            execvp(argv[0], argv);
        else if(pid > 0)
            wait((int *) 0);
        else
            perror("fork failed");

        if(!strcmp(buf, "exit"))
            ew_exit();
        else if(!strcmp(buf, "cd"))
            ew_cd(narg, argv);
        else if(!strcmp(buf, "mkdir"))
            ew_mkdir(narg, argv);
        else if(!strcmp(buf, "rmdir"))
            ew_rmdir(narg, argv);
        else if(!strcmp(buf, "cp"))
            ew_cp(narg, argv);
        else if(!strcmp(buf, "rm"))
            ew_rm(narg, argv);
        else if(!strcmp(buf, "mv"))
            ew_mv(narg, argv);
        else if(!strcmp(buf, "cat"))
            ew_cat(narg, argv);
    }
}

int ew_getargs(char *cmd, char **argv){
    int narg = 0;

    while(*cmd){
        if(*cmd == ' ' || *cmd == '\t')
            *cmd++ = '\0';
        else{
            argv[narg++] = cmd++;
            while(*cmd != '\0' && *cmd != ' ' && *cmd != '\t')
                cmd++;
        }
    }
    argv[narg] = NULL;

    return narg;
}

void ew_exit(){
    exit(1);
}

void ew_cd(int argc, char *argv[]){
    char *path;

   if(argc > 1)
      path = argv[1];
   else if((path = (char*)getenv("HOME")) == NULL)
      path = ".";

   chdir(path);
}

void ew_mkdir(int argc, char *argv[]){
    if(argc < 2){
      fprintf(stderr, "인자가 부족합니다.\n");
      return;
   }
   if(!(mkdir(argv[1], 0755)))
      fprintf(stderr, "디렉터리를 만들지 못했습니다.\n");
}

void ew_rmdir(int argc, char *argv[]){
    if(argc < 2){
      fprintf(stderr, "인자가 부족합니다.\n");
      return;
   }

   rmdir(argv);
}

void ew_cp(int argc, char *argv[]){
    FILE *fp1;
   FILE *fp2;
   char check;

   if(argc < 3){
      fprintf(stderr, "인자가 부족합니다.\n");
      return;
   }

   fp1 = fopen(argv[1], "r");
   fp2 = fopen(argv[2], "w");

   while(!feof(fp1)){
      check = (char)fgetc(fp1);

      if(check != EOF)
         fputc((int)check, fp2);
   }

   fclose(fp1);
   fclose(fp2);
}

void ew_rm(int argc, char *argv[]){
   if(argc < 2){
      fprintf(stderr, "인자가 부족합니다.\n");
      return;
   }

   unlink(argv[1]);
}

void ew_mv(int argc, char *argv[]){
    FILE *fp1;
   FILE *fp2;
   char check;

   if(argc < 3){
      fprintf(stderr, "인자가 부족합니다.\n");
      return;
   }

   fp1 = fopen(argv[1], "r");
   fp2 = fopen(argv[2], "w");

   while(!feof(fp1)){
      check = (char)fgetc(fp1);

      if(check != EOF)
         fputc((int)check, fp2);
   }

   fclose(fp1);
   fclose(fp2);

   unlink(argv[1]);
}

void ew_cat(int argc, char *argv[]){
   int check;
   FILE *fp;

   if(argc < 2){
      fprintf(stderr, "인자가 부족합니다.");
      return;
   }

   fp = fopen(argv[1], "r");

   while((check = getc(fp)) != EOF){
      putchar(check);
   }

   fclose(fp);
}
