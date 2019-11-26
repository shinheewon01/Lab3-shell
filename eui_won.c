#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

int ew_getargs(char *cmd, char **argv);
int checkTaskOption(char *argv);
void ew_exit();
void ew_ctrlc(int sig);
void ew_ctrlz(int sig, int flag);
void ew_pwd();
void pwd_print();
void ew_cd(int argc, char* argv[]);
void ew_mkdir(int argc, char *argv[]);
void ew_rmdir(int argc, char *argv[]);
void ew_ln(char *src, char *target);
void ew_cp(int argc, char *argv[]);
void ew_rm(int argc, char *argv[]);
void ew_mv(int argc, char *argv[]);
void ew_cat(int argc, char *argv[]);
void run(int i, int t_opt, int argc, char **argv);
void run_pipe(int i, char **argv);

void main(){
    char buf[256];
    char *argv[50];
    int narg;
    struct sigaction ctrlc_act;
    struct sigaction ctrlz_act;
    ctrlc_act.sa_handler = ew_ctrlc;
    ctrlz_act.sa_handler = ew_ctrlz;
    sigaction(SIGINT, &ctrlc_act, NULL);
    sigaction(SIGTSTP, &ctrlz_act, NULL);
    pid_t pid;

    while(1){
        printf("[eui_won shell] ");
        pwd_print();
        gets(buf);
        narg = ew_getargs(buf, argv);   // 들어온 인자 개수
        pid = fork();

        if(pid == 0)
            execvp(argv[0], argv);
        else if(pid > 0)
            wait((int *) 0);
        else
            perror("fork failed");

        if(!strcmp(buf, "exit"))
            ew_exit();
        else if(!strcmp(buf, "pwd"))
            ew_pwd(narg, argv);
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

        for (int i = 0; i < narg; i++) {
            // -1 = '&', 1 = '|', 2 = '<', 3 = '>'
            int t_opt = checkTaskOption(argv[i + 1]);

            if(t_opt == 1){
                run_pipe(i, argv);
                i += 2;
            }
            else
                run(i, t_opt, narg, argv);

            if(t_opt > 1)
                i += 2;
        }
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

//실행 인자 포함 여부 확인. 0 = 없음, -1 = '&', 1 = '|', 2 = '<', 3 = '>'
int checkTaskOption(char *argv){
    int opt = 0;
    if(argv == NULL)
        return opt;

    for(int i=0; argv[i] != NULL; i++){
        if (argv[i] == '&'){
            opt = -1;
            return opt;
        }
        if (argv[i] == '|'){
            opt = 1;
            return opt;
        }
        if (argv[i] == '<'){
            opt = 2;
            return opt;
        }
        if (argv[i] == '>'){
            opt = 3;
            return opt;
        }
    }

    return opt;
}

void ew_exit(){
    exit(1);
}

void ew_ctrlc(int sig){
    signal(sig, SIG_IGN);
    printf("  ctrl_C 입력 - 쉘 종료\n");
    exit(1);
}

void ew_ctrlz(int sig, int flag){
    signal(sig, SIG_IGN);
    printf(" 쉘 일시정지..\n");
    printf(" fg 명령으로 재개 가능..\n");
    raise(SIGSTOP);
    printf(" 쉘 재개..\n");
    signal(sig, ew_ctrlz);
}

void ew_pwd(){
    char buf[1024];
    getcwd(buf, 1024);
    printf("%s\n", buf);
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
   if(!(mkdir(argv[1], 0755)))
      fprintf(stderr, "디렉터리를 만들지 못했습니다.\n");
}

void ew_rmdir(int argc, char *argv[]){
   rmdir(argv);
}

void ew_ln(char *src, char *target){
    if (link(src, target) <0){
        perror("link");
    }
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

   fp = fopen(argv[1], "r");

   while((check = getc(fp)) != EOF){
      putchar(check);
   }
   fclose(fp);

}

void run(int i, int t_opt, int argc, char **argv){
    pid_t pid;
    int fd; /* file descriptor */
    char *buf[1024];
    int flags = O_RDWR | O_CREAT;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; /* == 0644 */
    memset(buf, 0, 1024);
    pid = fork();
    if (pid == 0){  //child
        //-1 = &, 1 = pipe, 2 = <, 3 = >
        if(t_opt == -1){
            printf("%s가 백그라운드로 실행...\n", argv[i]);
            selectCmd(i, argv);
            exit(0);
        }
        else if(t_opt == 2){
            if ((fd = open(argv[i + 2], flags, mode)) == -1) {
                perror("open"); /* errno에 대응하는 메시지 출력됨*/
                exit(1);
            }
            if (dup2(fd, STDIN_FILENO) == -1) {
                perror("dup2"); /* errno에 대응하는 메시지 출력됨 */
                exit(1);
            }
            if (close(fd) == -1) {
                perror("close"); /* errno에 대응하는 메시지 출력됨*/
                exit(1);
            }
            ew_cat(argc, argv[i+2]);
            selectCmd(i, argv);
            exit(0);
        }
        else if(t_opt == 3){
            if ((fd = open(argv[i+2], flags, mode)) == -1) {
                perror("open"); /* errno에 대응하는 메시지 출력됨*/
                exit(1);
            }
            if (dup2(fd, STDOUT_FILENO) == -1) {
                perror("dup2"); /* errno에 대응하는 메시지 출력됨 */
                exit(1);
            }
            if (close(fd) == -1) {
                perror("close"); /* errno에 대응하는 메시지 출력됨*/
                exit(1);
            }
            selectCmd(i, argv);
            exit(0);
        }
    }
    else if (pid > 0){  //parent - 백그라운드 아닐 때만 기다림
        if(t_opt >= 0){ //백그라운드가 아닐 때
            wait(pid);
       }
        if(!strcmp(argv[i], "cd")){
            ew_cd(argc, argv[i+1]);
        }
    }
    else{
        perror("fork failed");
    }
}

void run_pipe(int i, char **argv){
    char buf[1024];
    int p[2];
    int pid;

    /* open pipe */
    if (pipe(p) == -1) {
        perror ("pipe call failed");
        exit(1);
    }

    pid = fork();

    if (pid == 0) { /* child process */
        close(p[0]);
        if (dup2(p[1], STDOUT_FILENO) == -1) {
            perror("dup2"); /* errno에 대응하는 메시지 출력됨 */
            exit(1);
        }
        close(p[1]);
        selectCmd(i, argv);
        exit(0);
    }
    else if (pid > 0) {
        wait(pid);
        char *arg[1024];
        close(p[1]);
        sprintf(buf, "%d", p[0]);
        arg[0] = argv[i + 2];
        arg[1] = buf;
        selectCmd(0, arg);
    }
    else
        perror ("fork failed");
}

void selectCmd(int i, char **argv){
    //argv 판별해서 맞는 명령 실행
    if(!strcmp(argv[i], "cat")){
        if(argv[i+1] == NULL){
            fprintf(stderr, "A few argument..!\n");
        }
        ew_cat(i+1, argv[i+1]);
    }
    else if(!strcmp(argv[i], "pwd"))
        ew_pwd();
    else if(!strcmp(argv[i], "mkdir"))
        ew_mkdir(i, argv[i+1]);
    else if(!strcmp(argv[i], "rmdir"))
        ew_rmdir(i, argv[i+1]);
    else if(!strcmp(argv[i], "ln")){
        if(argv[i+1] == NULL || argv[i+2] == NULL){
            fprintf(stderr, "A few argument..!\n");
        }
        else{
            ew_ln(argv[i+1], argv[i+2]);
        }
    }
    else if(!strcmp(argv[i], "cp")){
        if(argv[i+1] == NULL || argv[i+2] == NULL){
            fprintf(stderr, "A few argument..!\n");
        }
        else{
            my_cp(argv[i+1], argv[i+2]);
        }
    }
    else if(!strcmp(argv[i], "rm")){
        if(argv[i+1] == NULL){
            fprintf(stderr, "A few argument..!\n");
        }
        else{
            ew_rm(i, argv[i+1]);
        }
    }
    else if(!strcmp(argv[i], "mv")){
        if(argv[i+1] == NULL || argv[i+2] == NULL){
            fprintf(stderr, "A few argument..!\n");
        }
        else{
            my_mv(argv[i+1], argv[i+2]);
        }
    }
    else{}
}

void pwd_print(){
    char buf[1024];
    getcwd(buf, 1024);
    printf("%s > ", buf);
}

void my_cp(char *src, char *target){
    int src_fd; /* source file descriptor */
    int dst_fd; /* destination file descriptor */
    char buf[10];
    ssize_t rcnt; /* read count */
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; /* == 0644 */
    if ((src_fd = open(src, O_RDONLY)) == -1 ){
        perror("src open"); /* errno에 대응하는 메시지 출력됨 */
        exit(1);
    }
    if ( (dst_fd = creat(target, mode)) == -1 ){
        perror("dst open"); /* errno에 대응하는 메시지 출력됨 */
        exit(1);
    }
    while ( (rcnt = read(src_fd, buf, 10)) > 0){
        write(dst_fd, buf, rcnt);
    }
    if (rcnt < 0) {
        perror("read");
        exit(1);
    }
    close(src_fd);
    close(dst_fd);
}

void my_rm(char *target){
    remove(target);
}

void my_mv(char *file, char *path){
    my_cp(file, path);
    my_rm(file);
}