#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

#define BUFFER_SIZE 5000

// Function that finds first occurence of substring in given string.
// If there is no such index returns -1.
int firstOccurenceIndex(char* string, int size1, char* substring, int size2) {
    for (int i = 0; i < size1 - size2; ++i) {
	int flag = 1;
        for (int j = 0; j < size2; ++j) {
	    if (substring[j] != string[i+j]) {
		flag = 0;
		break;
	    }
	}
	if (flag) {
	   return i;
	}
    }
    return -1;
}

int main(int argc, char* argv[]) {
    // Check for correct number of arguments.
    if (argc != 2) {
        printf("Incorrect number of parameters. Provide substring.\n");
        exit(-1);
    }
    
    size_t size;
    size_t input_size;
    int status;
    
    // Creating 2 named pipes.
    char pipe_name1[] = "string_pipe";
    char pipe_name2[] = "answer_pipe";
      
        char str_buf[BUFFER_SIZE];
        
        int fd = open(pipe_name1, O_RDONLY);
        size = read(fd, str_buf, sizeof(str_buf));
        
        if(size < 0){
            printf("Can\'t read string from pipe\n");
            exit(-1);
        }
        
        str_buf[size] = '\0';
        if(close(fd) < 0){
            printf("Second process can\'t close reading string pipe\n"); 
            exit(-1);
        }
	printf("Readed string:%sGot substring:%s\n", str_buf, argv[1]);
        // Computing result.
        
        int ans = firstOccurenceIndex(str_buf, size, argv[1], strlen(argv[1]));

        printf("Result computed\n");
        printf("First index of substring occurence:%d\n", ans);
        

	// Writing answer to pipe.
        int ans_fd = open(pipe_name2, O_WRONLY);
        size = write(ans_fd, &ans, sizeof(ans));

        if(size < 0) {
            printf("Can\'t write result to pipe\n");
            exit(-1);
        }

        if(close(ans_fd) < 0) {
            printf("Second process can\'t close writing answer pipe\n");
            exit(-1);
        }
        printf("Second process exit\n");
        exit(0);

    return 0;
}
