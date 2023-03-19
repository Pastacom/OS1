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

int main(int argc, char* argv[]) {
    // Check for correct number of arguments.
    if (argc != 3) {
        printf("Incorrect number of parameters. Provide path to input and output files.\n");
        exit(-1);
    }
    
    size_t size;
    size_t input_size;
    int status;
    
    // Creating 2 named pipes.
    char pipe_name1[] = "string_pipe";
    char pipe_name2[] = "answer_pipe";
    
    if (mkfifo(pipe_name1, 0666) < 0 || mkfifo(pipe_name2, 0666) < 0) {
        perror("Error occured while creating named pipes");
        exit(-1);
    }
        
        // Open input file and read string to buffer.
	int file;
	if ((file = open(argv[1], O_RDONLY)) < 0) {
	    printf("Can't open input file");
	}
	
	char input_buffer[BUFFER_SIZE];
	input_size = read(file, input_buffer, BUFFER_SIZE);

	if (close(file) < 0) {
	    printf("Can't close input file");
	    exit(-1);
	}

	// Transfer string to pipe to read after in 2-nd process.
	input_buffer[input_size] = '\0';
        int fd = open(pipe_name1, O_WRONLY);
        if (write(fd, input_buffer, input_size+1) != input_size+1) {
	    printf("Can\'t write all string to pipe\n");
	    exit(-1);
        }
        
        if(close(fd) < 0) {
          printf("First process can\'t close writing string pipe\n");
          exit(-1);
        }
        
    
        int ans = 0;
        
        // Reading answer from pipe.
        int ans_fd = open(pipe_name2, O_RDONLY);
        size = read(ans_fd, &ans, sizeof(int));

        if (size < 0) {
            printf("Can\'t read result from pipe\n");
            exit(-1);
        }

        if(close(ans_fd) < 0) {
            printf("First process can\'t close reading answer pipe\n"); 
            exit(-1);
        }

	// Opening and writing answer to output file.
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
    unlink(pipe_name1);
    unlink(pipe_name2);
        printf("First process exit\n");
        
    

    return 0;
}
