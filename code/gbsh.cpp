#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <cerrno>
#include <fcntl.h>
#include <experimental/filesystem>
#include<stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<cstring>
using namespace std;
namespace fs = std::experimental::filesystem::v1;
/********************************************************************SOME HASH DEFINE*********************************************************/
#define do_environ int i = 1; char *s = *environ;  for (; s; i++)   {    printf("%s\n", s);s = *(environ+i); }
#define do_ls for (const auto & entry :  std::experimental::filesystem::v1::directory_iterator(c_w_dir))cout << entry.path() <<endl;
#define print_pwd cout<<c_w_dir<<endl;  
#define clean  for (unsigned int i=0; i<argc; i++)argv[i] = NULL;     
#define error cout<<"ERROR"<<endl;
#define print_cwd cout<<temp;
#define set_cwd     name_of_user = getenv("USER");    getcwd(c_w_dir, sizeof(c_w_dir));     gethostname(name_of_host,sizeof(name_of_host));    strcpy(temp,name_of_user);    strcat(temp,"@");    strcat(temp,name_of_host);    strcat(temp,c_w_dir);    strcat(temp," >");
/********************************************************************GLOBAL VARIABLES*********************************************************/
char *name_of_user, c_w_dir[100], name_of_host[100];
char temp[100];
extern char **environ;
bool is_out_Put_redirection = false;
/**********************************************************PROTOTYPES***********************************************************************************************/
bool out_put_redirec_should(string arg);
enum PipeRedirect {PIPE, REDIRECT, NEITHER};
string execute( string cmd);
PipeRedirect parse_command(int, char**, char**, char**);
void pipe_cmd(char**, char**);
int read_args(char**);
void redirect_cmd(char**, char**);
void run_cmd(int, char**);
bool want_to_quit(string);
/**********************************************************SPLITS INTO ARGC AND ARGC ******************************************************/
PipeRedirect parse_command(int argc, char** argv, char** command_no_1, char** command_no_2) 
{
  	PipeRedirect answer_ = NEITHER;
  	int split = -1;
  	for (unsigned int i=0; i<argc; i++) 
  	{
    		if (strcmp(argv[i], "|") == 0) 
    		{
      		answer_ = PIPE;
      		split = i;
    		}
     		else if (strcmp(argv[i], ">>") == 0) 
     		{
      		answer_ = REDIRECT;
      		split = i;
    		}
  	}
  	if (answer_ != NEITHER)
  	{
    		for (unsigned int i=0; i<split; i++)
      		command_no_1[i] = argv[i];
    			int counter_temp = 0;
    		for (unsigned int i=split+1; i<argc; i++) 
    		{
      		command_no_2[counter_temp] = argv[i];
      		counter_temp++;
    		}
    		command_no_1[split] = NULL;
    		command_no_2[counter_temp] = NULL;
  	}
  	return answer_;
}
/************************************************************TAKES INTO ACCOUNT PIPES AND EXECUTE THEM*********************************/
void pipe_cmd(char** command_no_1, char** command_no_2) 
{
  	int file_discriptor_arr[2]; 
  	pipe(file_discriptor_arr);
  	pid_t P_ID;
  	if (fork() == 0) 
  	{
    		dup2(file_discriptor_arr[0], 0);
    		close(file_discriptor_arr[1]);
    		execvp(command_no_2[0], command_no_2);
    		perror("execvp failed");
  	}
  	else if ((P_ID = fork()) == 0) 
  	{
    		dup2(file_discriptor_arr[1], 1);
    		close(file_discriptor_arr[0]);
    		execvp(command_no_1[0], command_no_1);
    		perror("execvp failed");
  	} 
  	else
    		waitpid(P_ID, NULL, 0);
}
/***********************************************************TAKES INPUT FROM THE SHELL AND DOES SOME BASIC CHECKS*****************************/

int read_args(char **argv) 
{
 	char *c_string;
  	string arg;
  	int argc = 0;

  	while (cin >> arg) 
  	{
    		if (want_to_quit(arg)) 
    		{
      		cout << "Goodbye!\n";
      		exit(0);
	    	}
	    	if(out_put_redirec_should(arg))
	      	is_out_Put_redirection = true;

	    	c_string = new char[arg.size()+1];
	    	strcpy(c_string, arg.c_str());
	    	argv[argc] = c_string;

    		argc++;

    		if (cin.get() == '\n')
    			break;
  	}

  	argv[argc] = NULL;

  	return argc;
}
/***********************************************************DOES OUTPUT REDIRECTION HERE*************************************************/

bool out_put_redirec_should(string arg)
{
  	for (int i = 0; i < arg.length(); i++)
  	{
    		if(arg[i]=='<')
      	return true;
  	}
    	return false;
}

void output_dedirection(string from_which_file,  string cmd,const char* to_which_file = NULL)
{
  	string answer;
    	string command_line = cmd+" "+from_which_file;
    	const char* alpha = from_which_file.c_str();
	answer = execute(command_line);
	int i = answer.length();
	const void * a = answer.c_str();
    	if(to_which_file != NULL)
    	{
        	fstream obj(to_which_file);
        	if(!obj)
        	{
            	printf("Error opening the file\n");
        	}
         	obj<<answer;
        	obj.close();
    	}
    	else 
    		write(1,a,i);
}
void func(char **argc, int argv,char command[], char to_which_file[], char from_which_file[])
{
  	strcpy(command, argc[0]);
  	int i = 1;
  	for(;;)
  	{
    		if(!strcmp(argc[i], "<"))
    		{
      		break;
    		}
    		strcat(command, " ");
    		strcat(command, argc[i]);  
    		i++;  
  	}
  	strcpy(from_which_file, argc[i+1]);
  	i+=2;
  	if(i>=argv)
    		to_which_file[0] = 'N';
    	else
    	{
      	strcpy(to_which_file, argc[i+1]);
    	}
    	if(from_which_file[0]== '<')
      	from_which_file = &from_which_file[0];
      if(to_which_file[0]== '>')
      	to_which_file = &to_which_file[0];
    	if(to_which_file[0] == 'N')
    	{
    	  	output_dedirection((string)from_which_file, (string)command);
    	}
    	else
    	{
      	output_dedirection((string)from_which_file, (string)command, to_which_file);
    	}
    
     
}  
/*********************************************************************************************************************************************************/

void redirect_cmd(char** cmd, char** file) {
  	int file_discriptor_arr[2]; 
  	int counter_temp;  
  	int file_desp;   
  	char c;    
  	pid_t P_ID; 
  	pipe(file_discriptor_arr);
  	if (fork() == 0) 
  	{
    		file_desp = open(file[0], O_RDWR | O_CREAT, 0666);
    		if (file_desp < 0) 
    		{
      		printf("Error: %s\n", strerror(errno));
      		return;
    		}
    		dup2(file_discriptor_arr[0], 0);
    		close(file_discriptor_arr[1]);
    		while ((counter_temp = read(0, &c, 1)) > 0)
      		write(file_desp, &c, 1);
    		execlp("echo", "echo", NULL);
  	} 
  	else if ((P_ID = fork()) == 0) 
  	{
    		dup2(file_discriptor_arr[1], 1);
    		close(file_discriptor_arr[0]);
    		execvp(cmd[0], cmd);
  	} 
  	else 
  	{
    		waitpid(P_ID, NULL, 0);
    		close(file_discriptor_arr[0]);
    		close(file_discriptor_arr[1]);
  	}
}
/*********************************************************************************************************************************************************/

int return_size_of_char_array(char a[])
{
  	int i = 0;
  	for (i =0; a[i]!='\0';i++);
  		return i;
}
/*********************************EXECUTES BACKGROUD PROCESSES AND SIMPLE PROCESSES********************************************************/

void run_cmd(int argc, char** argv) {
  	pid_t P_ID;
  	const char *amp;
  	amp = "&";
  	bool found_amp = false;

  	if (strcmp(argv[argc-1], amp) == 0)
    		found_amp = true;
  	P_ID = fork();

  	if (P_ID == 0) 
  	{
    		if (found_amp) 
    		{
      		argv[argc-1] = NULL;
      		argc--;
    		}

    		execvp(argv[0], argv);
    		perror("execvp error");
  	} 
  	else if (!found_amp)
    		waitpid(P_ID, NULL, 0); 
}
/**********************************************************choice if user inputs exit************************************************/

bool want_to_quit(string choice) 
{
  	return (choice == "exit");
}
/************************GETS OUTPUT FROM COMMANT PROMPT IN A STRING***************************************************/

string execute( string cmd)
{
    	string file_name ="answer_.txt";
    	system( ( cmd + " > " + file_name ).c_str() ) ;
    	ifstream file(file_name);
    	if(!file)
    	{
      	cout<<"Please place a file by the name of *answer_.txt* in the current working directory"<<endl;
      	return NULL;
    	}
    	return { istreambuf_iterator<char>(file), istreambuf_iterator<char>() } ;    
}
/**************************************************************EXECUTES CD COMMAND************************************************/

void cd_execute(char** argv, int argc)
{
	char temp1[100];
    	name_of_user = getenv("USER");
      strcpy(c_w_dir, "/home/");
    	strcat(c_w_dir, name_of_user);
      strcat(c_w_dir, "/");
    	strcpy(temp1,name_of_user);
    	strcat(temp1,"@");
    	strcat(temp1,name_of_host);
      
	if(argc==0 || argv[1]==".")
	{
		return;
	}
	else
	{
		int i=1;
		while(i<argc)
		{
      		strcat(c_w_dir,argv[i]);
			strcat(c_w_dir,"/");
			strcat(temp1,argv[i]);
			strcat(temp1,"/");
			i++;
		}
		strcat(temp1," >");
	}
	strcpy(temp,temp1);
  	cout<<c_w_dir<<endl;
	return;
}  
/***************************************************************INT MAIN********************************************************************/
/*********************************************************************************************************************************************************/
/*********************************************************************************************************************************************************/
/*********************************************************************************************************************************************************/
/********************************************CHECKS FOR CERTAIN AND REDIRECTS TO OTHER FUNCTION TO EXECUTE*************************/

int main() 
{

  	char *argv[256], *command_no_1[256], *command_no_2[256];
  	PipeRedirect pipe_redirect;
  	int argc;
  	char command[10], from_which_file[10], to_which_file[10];
  	set_cwd
  	while (true) 
  	{
    
    		print_cwd
    		argc = read_args(argv);
    
    		if(is_out_Put_redirection)// REDIRECTS TO OUTPUT REDIRECTION
    		{
      		func(argv, argc, command, to_which_file, from_which_file); 
      		is_out_Put_redirection = false;
    		}
    		else
    		{
                	pipe_redirect = parse_command(argc, argv, command_no_1, command_no_2);
                 	if (pipe_redirect == PIPE)         
				pipe_cmd(command_no_1, command_no_2);
                  if (pipe_redirect == REDIRECT) 
                      	redirect_cmd(command_no_1, command_no_2);
                  if(!strcmp(argv[argc-1], "&"))
                        run_cmd(argc, argv);   
            	if(strcmp(argv[argc-1], "&"))
            	{
                    	if(!strcmp(argv[0],"cd"))
                    	{
                      		if(argv[0][return_size_of_char_array(argv[0])-1] == ' ')
                      		{
                        		argv[0][return_size_of_char_array(argv[0])-1] = '\0';
                      		}
                      		cd_execute(argv,argc);
                    	}
                    	else if(!strcmp(argv[0],"man")||!strcmp(argv[0],"ps")||!strcmp(argv[0],"cat")||!strcmp(argv[0],"top"))
                    	{
                      		if(fork())
                      		{
                        		system(argv[0]);        // FORKS SO THAT THE SHELL DOESNT CLOSES AFTER CNTRL + C
                      		}
                    	}
                    	else if(!strcmp(argv[0],"environ"))
                    	{
                      		do_environ              //PRINTS ENVIROMENTAL VARIABLES
                    	}
                    	else if(!strcmp(argv[0], "setenv"))
                    	{
                      		if(argc == 1)
                      		{
                        		do_environ              //PRINTS ENVIROMENTAL VARIABLES                       
                     		}
                      		else if(argc == 2)
                      		{
                        		error                 ///PRINTS ERROR ON THE SHELL
                      		}
                      		else
                      		{
                        		setenv(argv[1], argv[2], 1);
                      		}
                    	}
                    	else if(!strcmp(argv[0], "unsetenv"))
                    	{
                      		if(argc == 1)
                      		{
                        		error                 ///PRINTS ERROR ON THE SHELL
                      		}
                      		else
                      		{
                        		unsetenv(argv[1]);
                      		}
                    	}
                    	else if(!strcmp(argv[0], "ls"))
                    	{
                          	do_ls
                    	}
                    	else if(!strcmp(argv[0], "pwd"))
                    	{
                          	print_pwd
                    	}
                    	else
                    	{ 
                    		cout<<"Error! Wrong Input."<<endl;
                    		clean
                    	}
                    
            	}
            
    		}
  	}

  	return 0;

}
