#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

char *merger(char *path, const char *name) 
{
	int length = strlen(path) + strlen(name) + 1; 
	char *buf = calloc(length + 1, 1); 
	assert(sprintf(buf, "%s/%s", path, name) == length); 
	return buf; 
}
void get_round(char *path_root, char *path, int file) 
{
	DIR *dir; 
	dir = opendir(path);
	const struct dirent *temp;

	while (temp = readdir(dir)) {  
		if (temp->d_type != 4) {  
			char *path_new = merger(path, temp->d_name); 

			int length = strlen(path_new) - strlen(path_root);

			write(file, &length, sizeof(int)); 
			write(file, path_new + strlen(path_root),length);
			int file_in = open(path_new, O_RDONLY); 
			int file_size = lseek(file_in, 0, SEEK_END); 

			write(file, &file_size, sizeof(int));
			lseek(file_in, 0, SEEK_SET); 
			void *buf = malloc(file_size); 

			read(file_in, buf, file_size); 
			write(file, buf, file_size); 
			free(buf); 
			free(path_new);
			close(file_in); 
		} else {
			if (strcmp(temp->d_name, ".") == 0 ||strcmp(temp->d_name, "..") == 0)
				continue;
			char *path_new = merger(path, temp->d_name);
			get_round(path_root, path_new, file);
			free(path_new);
		}
	}
	closedir(dir);
}
void unpack(char *path, int file)
{
	int path_length;
	while (read(file, &path_length, sizeof(int)) == sizeof(int)) { 
		char *path_data = calloc(path_length + 1, 1);

		read(file, path_data, path_length);
		int data_length;

		read(file, &data_length, sizeof(int));
		void *data = malloc(data_length);

		read(file, data, data_length);
		char *path_full = merger(path, path_data);
		free(path_data);

		size_t length = strlen(path_full);

		for (size_t i = 1; i < length; i++) {
			if (path_full[i] == '/') {
				path_full[i] = 0;
				mkdir(path_full, 0700);
				path_full[i] = '/';
			}
		}
		int file_out = creat(path_full, 0700);

		write(file_out, data, data_length);
		close(file_out);
		free(path_full);
		free(data);
	}
}

int main(int argc, char *argv[]) 
{
	/* gcc arch.c -o res
	 ./res
	 from pack to              
	 to unpack from */
	if (argc != 4) {              
		puts("you must enter three parameters (pack/unpack)");
		return 0;
	}
	if (strcmp(argv[2], "pack") == 0) { 
		int file = creat(argv[3], 0700); 
		get_round(argv[1], argv[1], file);
		close(file);
	} else if (strcmp(argv[2], "unpack") == 0) {  
		char *name = argv[1];
		int file = open(name, O_RDONLY);
		unpack(argv[3], file);
		close(file);
	}
	return 0;
}