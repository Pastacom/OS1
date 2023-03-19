#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
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
    if (argc != 4) {
        printf("Incorrect number of parameters. Add path to input and output files and substring.\n");
        exit(-1);
    }
    
    int fd[2], ans_fd[2];
    pid_t pid1, pid2, pid3;
    size_t size;
    size_t input_size;
    int status;
    
    if(pipe(fd) < 0) {
        printf("Can\'t open pipe\n");
        exit(-1);
    }
    
    if(pipe(ans_fd) < 0) {
        printf("Can\'t open pipe\n");
        exit(-1);
    }
    
    // Create first child process.
    pid1 = fork();
    if (pid1 == -1) {
        printf("Error occured while fork process");
        exit(-1);
    }
    
    // First child process.
    if (pid1 == 0) {
        
        // Open input file and read string to buffer.
	int file;
	if ((file = open(argv[1], O_RDONLY)) < 0) {
	    printf("Can't open input file");
	}
	
	char input_buffer[BUFFER_SIZE];
	input_size = read(file, input_buffer, BUFFER_SIZE);
	printf("Readed string:%sGot substring:%s\n", input_buffer, argv[3]);

	if (close(file) < 0) {
	    printf("Can't close input file");
	    exit(-1);
	}

	// Transfer string to pipe to read after in 2-nd process.
	input_buffer[input_size] = '\0';
        if (write(fd[1], input_buffer, input_size+1) != input_size+1) {
	    printf("Can\'t write all string to pipe\n");
	    exit(-1);
        }
        
        if(close(fd[1]) < 0) {
          printf("First process can\'t close writing side of pipe\n");
          exit(-1);
        }
        
        exit(0);
    }
    
    // Waiting till 1-st process is done reading string and transferring data.
    if (waitpid(pid1, &status, 0) == -1) {
        perror("Occured error in waitpid1");
        exit(-1);
    }
    
    // Create second child process.
    pid2 = fork();
    if (pid2 == -1) {
        printf("Error occured while fork process");
        exit(-1);
    }
    
    // Second child process.
    if (pid2 == 0) {
       
        char str_buf[BUFFER_SIZE];
        
        size = read(fd[0], str_buf, sizeof(str_buf));

        if(size < 0){
            printf("Can\'t read string from pipe\n");
            exit(-1);
        }

	str_buf[size] = '\0';
        if(close(fd[0]) < 0){
            printf("Second process can\'t close reading side of pipe\n"); 
            exit(-1);
        }
        
        // Computing result.
        
        int ans = firstOccurenceIndex(str_buf, size, argv[3], strlen(argv[3]));

        printf("Result computed\n");
        printf("First index of substring occurence:%d\n", ans);
        
	// Writing answer to pipe.
        size = write(ans_fd[1], &ans, sizeof(ans));

        if(size < 0) {
            printf("Can\'t write result to pipe\n");
            exit(-1);
        }

        if(close(ans_fd[1]) < 0) {
            printf("Second process can\'t close writing side of pipe\n");
            exit(-1);
        }
        printf("Second process exit\n");
        exit(0);
    }
    
    // Waiting till 2-st process is done reading data from pipes, computing result and transferring it to 3-rd process.
    if (waitpid(pid2, &status, 0) == -1) {
        perror("Occured error in waitpid2");
        exit(-1);
    }
    
    // Create 3rd child process
    pid3 = fork();
    if (pid3 == -1) {
        printf("Error occured while fork process");
        exit(-1);
    }
    
    // Third child process.
    if (pid3 == 0) {
        
        int ans = 0;
        
        // Reading answer from pipe.
        size = read(ans_fd[0], &ans, sizeof(int));

        if (size < 0) {
            printf("Can\'t read result from pipe\n");
            exit(-1);
        }

	// Opening and writing answer to output file.
	int file;
        if ((file = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0) {
            printf("Can\'t open output file for writing\n");
            exit(-1);
        }
        
        // Converting int answer to char sequence.
        char result[BUFFER_SIZE];
        snprintf(result, sizeof(result), "First occurrence index: %d\n", ans);
        write(file, result, strlen(result));
        
        if(close(file) < 0) {
            printf("Can\'t close input file\n");
            exit(-1);
        }

        printf("Data was successfully written to %s\n", argv[2]);

        if(close(ans_fd[0]) < 0) {
            printf("Third process can\'t close reading side of pipe\n"); 
            exit(-1);
        }

        printf("Third process exit\n");
        exit(0);
    }
    
    // Wait till 3-rd process is done reading data from pipe, converting it and writing to file.
    if (waitpid(pid3, &status, 0) == -1) {
        perror("Occured error in waitpid3");
        exit(-1);
    }
 
    return 0;
}
