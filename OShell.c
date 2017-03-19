#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#define MAX 1<<7
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <dirent.h>
#include <ctype.h>
#include <signal.h>

char *line[255];
char *line2[255];
char *command[255];
//char *lines[255][255];
int noOfLineArgs;
bool pipeFlag=false;
int noOfLines;
int noOfCommandLines;

//Global flags for cp command
bool interactiveFlag=false;
bool recursiveFlag=false;
bool targetDirFlag=false;
bool verboseFlag=false;
bool patternFlag=false;
int pathsNo;
int first;
int last;

//Flag for tee command
bool appendFlag=false;

void splitLine(char s[255]);
int executeLine();
int cpMain();
int isFile(const char* name);
int copyFile(const char* oldPath, const char* newPath);
int okToCopy(char* path1, char* path2);
void findAndCopy(const char* folderName, char* filePattern, char *destination);
int recursiveCopy(char* folder1,char* folder2);
void resetVars();
int teeMain();
int dirnameMain();
int piping(char *command1,char *command2);

int main()
{
	char *s=(char*)malloc(MAX*sizeof(char));
	
	
	do
	{
		s=readline(">>");
		add_history(s);
		splitLine(s);
		executeLine();
		resetVars();
	}
	while(s!=NULL);
	
	free(s);
return 0;
}

void version(){
	printf("Aneta's Terminal \n");
	printf("Made by Aneta Suletea, year2, sg 3\n\n");

}

void help(){

	printf("In Aneta's terminal are the following commands:\n\n\n");
	printf("cp -copy files and directories\n");
	printf("cp [Options] [SOURCE]...[DESTINATION]\n");
	printf("\t '-i' (interactive) asks before replacing an existing file\n");
	printf("\t '-r' (-R) (recursive) copies recursively all from the [SOURCE 1]...[SOURCE n] direcories to [DESTINATION] directory\n");
	printf("\t '-t' (target) [DESTINATION][SOURCE]\n");
	printf("\t '-v' (verbalise) shows steps. Prints what files were copied and where\n\n");

	printf("tee -output on screen and in a file specified the content of another file\n");
	printf("tee [Option] [File1] [File2]\n");
	printf("\t '-a' (append) instead of replacing the content of [File2], adds text at the end of it\n\n");
	
	printf("dirname - strips the last component from file name\n\n");
	printf("diname [File-name]\n");

}

int piping(char *command1, char *command2) {
	int fd[2];
	

	pipe(fd);
	// child process #1
	if (fork() == 0) {
		// Reassign stdin to fd[0] end of pipe.
		dup2(fd[0], STDIN_FILENO);
		close(fd[1]);
		close(fd[0]);
		// Execute the second command.
		// child process #2
		if (fork() == 0) {
			// Reassign stdout to fd[1] end of pipe.
			dup2(fd[1], STDOUT_FILENO);
			close(fd[0]);
			close(fd[1]);
			// Execute the first command.
			splitLine(command2);	
			char *p;
			int i;
			for(i=0,p=strtok(command2," ");p!=NULL; i++,p=strtok(NULL," ")){
				line[i]=(char*)malloc(MAX*sizeof(char));
				line[i]=p;
			}
			noOfLineArgs=i;
			if (strcmp(line[0],"cp")==0)
				cpMain();
			else if (strcmp(line[0],"tee")==0)
				teeMain();
			else if (strcmp(line[0],"dirname")==0)
				dirnameMain();
			else if (strcmp(line[0],"help")==0)
				help();
			else if (strcmp(line[0],"version")==0)
				version();
			else if(strcmp(line[0],"exit")==0){
				printf("Exiting... Bye!\n");
				exit(1);			
			}		
			else{
				execvp(line[0],line);
			}
		}
	wait(NULL);
	char *p;
	int i;
	for(i=0,p=strtok(command1," ");p!=NULL; i++,p=strtok(NULL," ")){
		line[i]=(char*)malloc(MAX*sizeof(char));
		line[i]=p;
	}
	noOfLineArgs=i;
   	if (strcmp(line[0],"cp")==0)
		cpMain();
	else if (strcmp(line[0],"tee")==0)
		teeMain();
	else if (strcmp(line[0],"dirname")==0)
		dirnameMain();
	else if (strcmp(line[0],"help")==0)
		help();
	else if (strcmp(line[0],"version")==0)
		version();
	else if(strcmp(line[0],"exit")==0){
		printf("Exiting... Bye!\n");
		exit(1);			
	}		
	else{
		execvp(line[0],line);
	}
    }
    close(fd[1]);
    close(fd[0]);
    wait(NULL);
	
}

void resetVars(){
	char *line[255];
	for(int i=0;i<noOfLineArgs;i++)
		line[i]='\0';
	noOfLineArgs=0;
	pipeFlag=false;
	//For cp command
	interactiveFlag=false;
	recursiveFlag=false;
	targetDirFlag=false;
	verboseFlag=false;
	patternFlag=false;
	pathsNo=0;
	first=0;
	last=0;
	//For tee command
	appendFlag=false;

}
void splitLine(char s[255]){
	noOfLineArgs=0;
	char *p;
	int i,j;
	if(strchr(s,'|')!=NULL && pipeFlag==false){
		for(i=0,p=strtok(s,"|");p!=NULL; i++,p=strtok(NULL,"|")){
			command[i]=(char*)malloc(MAX*sizeof(char));
			command[i]=p;
		}
		noOfCommandLines=i;
		pipeFlag=true;
	}
	else{
		for(i=0,p=strtok(s," ");p!=NULL; i++,p=strtok(NULL," ")){
			line[i]=(char*)malloc(MAX*sizeof(char));
			line[i]=p;
		}
		noOfLineArgs=i;
	}
}
int executeLine(){
	if(pipeFlag==false){
		if (strcmp(line[0],"cp")==0)
			cpMain();
		else if (strcmp(line[0],"tee")==0)
			teeMain();
		else if (strcmp(line[0],"dirname")==0)
			dirnameMain();
		else if (strcmp(line[0],"help")==0)
			help();
		else if (strcmp(line[0],"version")==0)
			version();
		else if(strcmp(line[0],"exit")==0){
			printf("Exiting... Bye!\n");
			exit(1);			
		}		
		else{
			printf("Command %s doesn't exist in Aneta's terminal.\n Try 'help' for more information.\n", line[0]);
			return -1;
		}
	}
	else{
		if(noOfCommandLines<2)
		{
			printf("Cannot pipe one single command. Please introduce at least 2\n");
		}
		else{
			printf("\n\n****Using commands from real terminal with pipes.\nIn case a command you introduced is implemented");
			printf("in Aneta's terminal\n");
			printf("it will be executed by Aneta's implementation ^_^\n\n");
			for(int i=0;i<noOfCommandLines-1;i++)
				piping(command[i],command[i+1]);
		}


	}
}
int dirnameMain(){
	char *p;
	for(int i=1;i<noOfLineArgs;i++){
		if((p=strchr(line[i],'-'))){
			printf("Invalid option -- %c\n",p[1]);
			return -1;
		}
		else{
			p=strrchr(line[i],'/');
			p[0]='\0';
			printf("%s\n",line[i]);
		}
	}
	return 0;
}
int teeMain(){
	FILE *file1, *file2;
	char buffer;
	int f1=0,f2=0;	
	char *p;

	for(int i=1;i<noOfLineArgs;i++){
		if(strcmp("-a",line[i])==0){
			appendFlag=true;
		}
		else if((p=strchr(line[i],'-'))){
			printf("Invalid option -- %c\n",p[1]);
			return -1;
		}
		else{
			if(f1==0)
				f1=i;
			else if(f2==0)
				f2=i;
			else {
				printf("Too much arguments for tee\n");
				return -1;
			}
		}

	}
	file1=fopen(line[f1],"rb");
	if(!file1){
		printf("Tee error no %d.\n",errno);
		perror("First file unable to open:");
		return errno;
	}
	if(!appendFlag)
		file2=fopen(line[f2],"wb");
	else
		file2=fopen(line[f2],"a");
	if(!file2){
		printf("Tee error no %d. \n",errno);
		perror("Second file unable to open:");
		return errno;
	}
	printf("File '%s' contains:\n",line[f1]);
	while((buffer=fgetc(file1))!=EOF){
                fputc(buffer, file2);
                printf("%c", buffer);
		
        }
	if (ferror(file1)){
		printf("Tee error no %d.\n",errno);
		perror("First file error:");
		return errno;
	}
	if (ferror(file2)){
		printf("Tee error no %d.\n",errno);
		perror("Second file error:");
		return errno;
	}
	printf("\n\n%s received %s\n", line[f2], line[f1]); 
	fclose(file1);
    	fclose(file2);
	return 1;
}

int cpMain(){
	int i=0;
	int j=0;
	char *p;
	for(i=1;i<noOfLineArgs;i++){
		if(strcmp("-i",line[i])==0){
			interactiveFlag=true;
		}
		else if(strcmp("-r",line[i])==0 || strcmp("-R",line[i])==0){
			recursiveFlag=true;
		}
		else if(strcmp("-t",line[i])==0){
			targetDirFlag=true;
		}
		else if(strcmp("-v",line[i])==0){
			verboseFlag=true;
		}
		else if((p=strchr(line[i],'-'))){
			printf("Invalid option -- %c\n",p[1]);
			return -1;	
		}
		else{
			if(first==0)
				first=i;
			else
				last=i;
			pathsNo++;
		}
	}	
	if(!recursiveFlag)
	{
		if(targetDirFlag==true){
			i=first;
			first=last;
			last=i;					
		}
		if(strchr(line[first],'#')!=NULL) 
			patternFlag=true;
		if(okToCopy(line[first],line[last]) && patternFlag==false){
			if(verboseFlag){
				printf("'%s' -> '%s'\n",line[first],line[last]);
			}
			copyFile(line[first], line[last]);
		}
		else if(patternFlag){
			char fileName[255];
			char directoryName[255];
			char *p;
			strcpy(directoryName,line[first]);
			p=strrchr(directoryName,'/');
			strcpy(fileName,p+1);
			p[0]='\0';
			printf("FILE name: %s\n",fileName);
			findAndCopy(directoryName, fileName, line[last]);
		}
		if(targetDirFlag)
			for(j=last;j>=first;j--){
				if(isFile(line[j])==0)
					printf("Ommiting directory '%s' \n",line[j]);
				if(isFile(line[j])==-1){
					printf("'%s': No such file or directory\n",line[j]);
					return 0;				
				}
			}
		else 
			for(j=first;j<=last;j++){
				if(isFile(line[j])==0)
					printf("Ommiting directory '%s' \n",line[j]);
				if(isFile(line[j])==-1){
					printf("'%s': No such file or directory\n",line[j]);
					return 0;
				}
			}
	}	
	else{
		if(targetDirFlag==true){
			i=last;
			last=first;
			first=i;	
			for(i=first;i>last;i--)
				recursiveCopy(line[i],line[last]);				
		}
		else{
			for(i=first;i<last;i++)
				recursiveCopy(line[i],line[last]);	
		}
	}
	return 1;

}
int isFile(const char* name)
{
    DIR* directory = opendir(name);

    if(directory != NULL)
    {
     closedir(directory);
     return 0;
    }

    if(errno == ENOTDIR)
    {
     closedir(directory);
     return 1;
    }
    closedir(directory);
    return -1;
}

int copyFile(const char* oldPath, const char* newPath){
	int pipefd[2];
	int fileSize;
        FILE *originFile;
        FILE *newFile;
	struct stat buf;
	int r;
	char replace='y';
	originFile = fopen(oldPath, "rb");
	if(!originFile){
			printf("Cp error no %d.\n",errno);
			perror(" origin file:");
			return errno;
	}
	if(interactiveFlag){
		newFile= fopen(newPath, "rb");
		if(newFile!=NULL){
			printf("Would you like to replace the '%s' existing file? Y/N\n",newPath);
			scanf("%c",&replace);
		}
	}
	if(tolower(replace)=='y'){
	newFile= fopen(newPath, "wb");
	if(newFile==NULL){
		printf("Cp error no %d.\n",errno);
		perror("New File: ");
		return errno;
	}
	if(pipe(pipefd)<0){
		printf("Cp error no %d.\n",errno);
		perror("Pipe error");
		return errno;
	}
	if(fstat(fileno(originFile), &buf)<0){
		printf("Cp error no %d.\n",errno);
		perror("Fstat error");
		return errno;
	}
	fileSize=buf.st_size;
	do{
		if(fileSize>BUFSIZ){
			r=splice(fileno(originFile), 0, pipefd[1], NULL,BUFSIZ , SPLICE_F_MORE | SPLICE_F_MOVE);
			if(r==-1){
				printf("Cp error no %d.\n",errno);
				perror("First splice error");
				return errno;
			}
			r=splice(pipefd[0], NULL, fileno(newFile), 0, BUFSIZ, SPLICE_F_MORE | SPLICE_F_MOVE);
			if (r == -1){
				printf("Cp error no %d.\n",errno);
        			perror("Second splice error");
				return errno;
			}
		}
		else{
			r=splice(fileno(originFile), 0, pipefd[1], NULL,fileSize , SPLICE_F_MORE | SPLICE_F_MOVE);
                        if(r==-1){
				printf("Cp error no %d.\n",errno);
                                perror("First splice error");
                                return errno;
                        }

                        r=splice(pipefd[0], NULL, fileno(newFile), 0, fileSize, SPLICE_F_MORE | SPLICE_F_MOVE);
                        if (r == -1){
				printf("Cp error no %d.\n",errno);
                                perror("Second splice error");
                                return errno;
                        }
		}

		fileSize-=BUFSIZ;
	}while(fileSize>0);
	}
	close(pipefd[0]);
	close(pipefd[1]);
	fclose(originFile);
	fclose(newFile);


}
int okToCopy(char path1[255], char* path2){
	char* aux;
	if(patternFlag==false){
		if(isFile(path2)==0){			
			aux=strrchr(path1,'/');
			if(aux!=NULL){
				if(recursiveFlag)
				strcat(path2,path1); 
				else
					strcat(path2,aux);
			}
			else{
				strcat(path2,"/");
				strcat(path2,path1);
			}
		}
		else if(isFile(path2)==1 || isFile(path2)==-1){
			if(recursiveFlag==false)
			{
				printf("'%s' is not a directory\n",path2);
				return 0;
			}
			else return 0;
		}
	}


}

void findAndCopy(const char* folderName, char* filePattern, char *destination){
	DIR *d;
	char *p1, *p2;
	struct dirent *dir;
	bool patternFound=true;
	char filePat[100];
	char fileName[255];
	char initial[255];
	char fin[255];
	bool endedInStar=false;
	char *aux;
	strcpy(filePat,filePattern);
    	d = opendir(folderName);
	if (d)
    	{
        	while ((dir = readdir(d)) != NULL)
        	{
			endedInStar=false;
			patternFound=true;
			strncpy(fileName,dir->d_name,254);
			fileName[254]='\0';
			strcpy(filePat,filePattern);
			p2=fileName;
			if(filePat[strlen(filePat)-1]=='#'){
				endedInStar=true;
			}

			if(isFile(fileName)){
				for(p1=strtok(filePat,"#");p1!=NULL && patternFound==true;p1=strtok(NULL,"#")){
					p2=strstr(p2,p1);
					if(p2==NULL)
						patternFound=false;
					else if(strcmp(p1,p2)!=0 && endedInStar==false)
						patternFound=false;

				}
				if(patternFound==true){
					strcpy(initial,folderName);
					strcat(initial,"/");
					strcat(initial,fileName);
					strcpy(fin,destination);
					strcat(fin,"/");
					strcat(fin,fileName);
					if(verboseFlag==true)
						printf("'%s' -> '%s'\n",initial,fin);
					copyFile(initial,fin);
				}
			}
		}
        closedir(d);
    }
}

int recursiveCopy(char* folder1,char* folder2){
	DIR *d;
	struct dirent *dir;
	DIR *aux;
	char fileName[255];
	char folderName[255];
	char copyTo[255];
	char copyFrom[255];
	char *p;
	int status;
	int childExitStatus;
	pid_t pid;
	d = opendir(folder1);
	if(d){
		while((dir=readdir(d))!=NULL)
		{
			folderName[0]='\0';
			strcpy(copyTo,folder2);
			p=folder1;
			if(folder1[0]=='.')
				p++;
			strncat(copyTo,p,strlen(folder1));
			strcpy(copyFrom,p);
			mkdir(copyTo, 0700);
			if(dir->d_type==DT_REG)
			{
				strncpy(fileName,dir->d_name,255);
				fileName[254]='\0';
				if(isFile(fileName)){
					strcat(copyFrom,"/");
					strcat(copyFrom,fileName);
					if(okToCopy(fileName,copyTo)){
						if(verboseFlag){
							printf("'%s' -> '%s'\n",copyFrom,copyTo);
						}
						copyFile(fileName, copyTo);
					
					}
				}
			}
			else if(dir->d_type==DT_DIR)
			{
				strcat(folderName,folder1);
				strcat(folderName,"/");
				strcat(folderName,dir->d_name);
				if(strcmp(dir->d_name,"..")!=0 && strcmp(dir->d_name,".")!=0)
				{
						int fd[2];
						pid_t pid;
						if(pipe(fd)<0)
						{
							printf("Cp error no %d.\n",errno);
							perror("Recursive pipe error:");
							return errno;
						}
						pid=fork();
						if(pid<0)
						{
							printf("Cp error no %d.\n",errno);
							perror("Recursive fork error:");
							return errno;
						}
						if(pid==0) //child
						{
	   						 pause();
							recursiveCopy(folderName,folder2);
							return errno;
						}
						else //parent
						{
							pid_t ws = waitpid( pid, &childExitStatus, WNOHANG);
							if (ws == -1)
							{ 
								printf("Cp error no %d.\n",errno);
								printf(" ");
								kill(pid, SIGKILL);
							}

							if( WIFEXITED(childExitStatus))
							{
							    status = WEXITSTATUS(childExitStatus);
								if(status!=0){
									printf("Cp error no %d.\n",errno);
									perror(" ");
								}	
								kill(pid, SIGKILL);
							}
							else if (WIFSIGNALED(childExitStatus)) 
							{
								printf("Cp error no %d.\n",errno);
								perror(" ");
								 kill(pid, SIGKILL);
							}
							else if (WIFSTOPPED(childExitStatus)) /* stopped */
							{
								printf("Cp error no %d.\n",errno);
								perror(" ");
								kill(pid, SIGKILL);
							}
								wait(NULL);
								recursiveCopy(folderName,folder2);
						}

				  }
			}
		}
	}


}
