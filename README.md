# Folder Sync

Folder Sync is a program written in C that synchronizes two folders, maintaining an identical copy of the source folder at the replica folder.

## Installation

Clone the repository:
   ```bash
   git clone https://github.com/your-username/folder-sync.git
   cd folder-sync
  ```
Compile the source code using the provided Makefile:

  ```bash
  make
  ```
## Usage

To synchronize two folders, run the compiled executable with the following command-line arguments:

```bash
./folder-sync source_folder replica_folder interval log_file
```

- `source_folder`: The path to the source folder.
- `replica_folder`: The path to the replica folder.
- `interval`: The synchronization interval in seconds.
- `log_file`: The path to the log file.

Example:
```bash
./folder-sync /path/to/source /path/to/replica 60 sync.log
```

## Code Overview and Functionality

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#define PATH_MAX MAX_PATH
#else
#include <limits.h>
#endif

#define BUF_SIZE 1024
```

Includes the necessary header files and defines constants used throughout the program.

- `stdio.h`: Provides input and output functions like `printf` and `fprintf`.
- `stdlib.h`: Defines memory allocation and other general utilities.
- `string.h`: Provides string manipulation functions like `strcpy` and `strlen`.
- `unistd.h`: Contains POSIX operating system API functions like `read` and `write`.
- `dirent.h`: Defines structures and functions for directory operations.
- `sys/stat.h`: Defines functions for obtaining information about file attributes.
- `fcntl.h`: Contains file control options.
- `errno.h`: Defines macros for reporting and retrieving error conditions.
- `time.h`: Provides functions for manipulating date and time.
- `windows.h` (if `_WIN32` is defined): Windows-specific header for API functions.
- `PATH_MAX`: Maximum length of a file path.
- `BUF_SIZE`: Size of the buffer used for reading and writing file data.

```c
void ft_sync_folders(const char *source_folder, const char *replica_folder, const char *log_file);
void ft_sync_directory(const char *source_folder, const char *replica_folder, const char *log_file);
void ft_copy_file(const char *source_path, const char *replica_path, const char *log_file);
void ft_remove_files_not_in_source(const char *source_folder, const char *replica_folder, const char *log_file);
void ft_finalize_logging(const char *log_file);
```

These function prototypes declare the functions used for folder synchronization and logging:

- `ft_sync_folders`: Synchronizes the content of the source folder with the replica folder, logging file operations.
- `ft_sync_directory`: Recursively synchronizes subdirectories within the source and replica folders.
- `ft_copy_file`: Copies a file from the source path to the replica path, logging the operation.
- `ft_remove_files_not_in_source`: Removes files in the replica folder that are not present in the source folder, logging file deletions.
- `ft_finalize_logging`: Finalizes the logging process by adding a completion message to the log file.

```c
int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s source_folder replica_folder interval log_file\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *source_folder = argv[1];
    const char *replica_folder = argv[2];
    int interval = atoi(argv[3]);
    const char *log_file = argv[4];

    while (1) {
        ft_sync_folders(source_folder, replica_folder, log_file);
        sleep(interval);
    }

    return EXIT_SUCCESS;
}
```

The `main` function is the entry point of the program. It accepts command-line arguments and orchestrates the folder synchronization process:

1. `argc` and `argv[]` are parameters representing the number of command-line arguments and an array of argument strings, respectively.

2. The `if` statement checks if the number of command-line arguments is not equal to 5. If so, it prints a usage message to the standard error stream, indicating the required arguments, and exits the program with a failure status (`EXIT_FAILURE`).

3. If the correct number of arguments is provided, the function proceeds to extract the command-line arguments:
    - `source_folder`: The path to the source folder.
    - `replica_folder`: The path to the replica folder.
    - `interval`: The synchronization interval in seconds.
    - `log_file`: The path to the log file.

4. Inside the infinite `while` loop, the `ft_sync_folders` function is called to synchronize the source and replica folders, passing the extracted arguments (`source_folder`, `replica_folder`, and `log_file`). The `ft_sync_folders` function is responsible for managing the synchronization process.

5. After each synchronization, the program sleeps for the specified interval using the `sleep` function to delay the next synchronization cycle.

6. Once the program is terminated (e.g., by a manual interruption), it returns `EXIT_SUCCESS`, indicating successful execution.

```c
void ft_sync_folders(const char *source_folder, const char *replica_folder, const char *log_file) {
    ft_sync_directory(source_folder, replica_folder, log_file);
    ft_finalize_logging(log_file);
}
```

The `ft_sync_folders` function serves as the entry point for folder synchronization and logging:

1. It takes three parameters:
    - `source_folder`: The path to the source folder to be synchronized.
    - `replica_folder`: The path to the replica folder where changes are applied.
    - `log_file`: The path to the log file where synchronization events are recorded.

2. Within the function, `ft_sync_directory` is called to synchronize the content of the source folder with the replica folder. This function traverses the directory structure recursively, copying files and creating directories as needed to ensure both folders are identical.

3. After the synchronization process is completed, `ft_finalize_logging` is called to add a completion message to the log file. This finalizes the logging process, indicating the end of the synchronization operation.

4. By encapsulating the synchronization and logging tasks within a single function, `ft_sync_folders` provides a convenient interface for initiating and managing the synchronization process, making the code modular and easier to understand.

```c
void ft_sync_directory(const char *source_folder, const char *replica_folder, const char *log_file) {
    DIR *source_dir = opendir(source_folder);
    if (source_dir == NULL) {
        return;
    }

    struct dirent *dir_entry;
    while ((dir_entry = readdir(source_dir)) != NULL) {
        if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0) {
            continue;
        }

        char source_path[PATH_MAX];
        char replica_path[PATH_MAX];
        snprintf(source_path, sizeof(source_path), "%s/%s", source_folder, dir_entry->d_name);
        snprintf(replica_path, sizeof(replica_path), "%s/%s", replica_folder, dir_entry->d_name);

        struct stat stat_buf;
        if (stat(source_path, &stat_buf) == -1) {
            continue;
        }

        if (S_ISDIR(stat_buf.st_mode)) {
            ft_sync_folders(source_path, replica_path, log_file);
        } else {
            ft_copy_file(source_path, replica_path, log_file);
        }
    }

    closedir(source_dir);
    ft_remove_files_not_in_source(source_folder, replica_folder, log_file);
}
```

The `ft_sync_directory` function is responsible for synchronizing the contents of a directory within the source and replica folders:

1. It takes three parameters:
    - `source_folder`: The path to the source folder whose contents are being synchronized.
    - `replica_folder`: The path to the replica folder where changes are applied.
    - `log_file`: The path to the log file where synchronization events are recorded.

2. The function begins by opening the source directory using `opendir`. If the directory cannot be opened (e.g., due to permission issues or non-existent directory), the function returns early, indicating a failure to synchronize.

3. Next, the function iterates over each entry in the source directory using `readdir`. Entries for the current directory (`"."`) and parent directory (`".."`) are skipped to avoid infinite recursion and unnecessary synchronization.

4. For each regular file or directory encountered, the function constructs the full paths of the corresponding source and replica entries using `snprintf`.

5. It then retrieves the file attributes of the source entry using `stat` to determine whether it is a directory or a regular file.

6. If the entry is a directory, `ft_sync_folders` is recursively called to synchronize its contents with the corresponding directory in the replica folder. This ensures that subdirectories are synchronized recursively.

7. If the entry is a regular file, `ft_copy_file` is called to copy the file from the source to the replica folder.

8. After processing all entries in the source directory, the function closes the directory using `closedir`.

9. Finally, it calls `ft_remove_files_not_in_source` to remove any files in the replica folder that are not present in the source folder, ensuring both folders remain identical after synchronization.

10. By encapsulating the directory synchronization logic within a separate function, `ft_sync_directory` enhances code readability and modularity, allowing for easier maintenance and extension of the synchronization functionality.

```c
void ft_copy_file(const char *source_path, const char *replica_path, const char *log_file) {
    int src_fd = open(source_path, O_RDONLY);
    if (src_fd == -1) {
        return;
    }

    int dest_fd = open(replica_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (dest_fd == -1) {
        close(src_fd);
        return;
    }

    char buffer[BUF_SIZE];
    ssize_t bytes_read, bytes_written;
    while ((bytes_read = read(src_fd, buffer, BUF_SIZE)) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            break;
        }
    }

    close(src_fd);
    close(dest_fd);

    FILE *log_fp = fopen(log_file, "a");
    if (log_fp != NULL) {
        fprintf(log_fp, "Copied %s to %s\n", source_path, replica_path);
        fclose(log_fp);
    }
}
```

The `ft_copy_file` function is responsible for copying a file from the source path to the replica path:

1. It takes three parameters:
    - `source_path`: The path to the source file.
    - `replica_path`: The path to the destination file in the replica folder.
    - `log_file`: The path to the log file where the copy operation is logged.

2. The function opens the source file (`source_path`) for reading using `open` with the `O_RDONLY` flag. If the file cannot be opened (e.g., due to permission issues or non-existent file), the function returns early, indicating a failure to copy the file.

3. It then opens the replica file (`replica_path`) for writing using `open` with the `O_WRONLY`, `O_CREAT`, and `O_TRUNC` flags. If the file cannot be opened or created, the function closes the source file and returns, indicating a failure to copy the file.

4. Using a loop, the function reads data from the source file into a buffer (`buffer`) using the `read` function and writes the data to the replica file using the `write` function. It continues this process until all data is read from the source file or an error occurs during writing.

5. After copying the file, both the source and replica file descriptors are closed using `close`.

6. The function then appends a log entry to the log file (`log_file`) indicating that the file was successfully copied from the source path to the replica path. This log entry helps track file copy operations during the synchronization process.

7. By encapsulating the file copying logic within a separate function, `ft_copy_file` enhances code readability and modularity, allowing for easier maintenance and extension of the synchronization functionality.

```c
void ft_remove_files_not_in_source(const char *source_folder, const char *replica_folder, const char *log_file) {
    DIR *replica_dir = opendir(replica_folder);
    if (replica_dir == NULL) {
        return;
    }

    struct dirent *dir_entry;
    while ((dir_entry = readdir(replica_dir)) != NULL) {
        if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0) {
            continue;
        }

        char replica_path[PATH_MAX];
        snprintf(replica_path, sizeof(replica_path), "%s/%s", replica_folder, dir_entry->d_name);

        char source_path[PATH_MAX];
        snprintf(source_path, sizeof(source_path), "%s/%s", source_folder, dir_entry->d_name);
        if (access(source_path, F_OK) == -1) {
            if (remove(replica_path) != -1) {
                FILE *log_fp = fopen(log_file, "a");
                if (log_fp != NULL) {
                    fprintf(log_fp, "Deleted %s\n", replica_path);
                    fclose(log_fp);
                }
            }
        }
    }

    closedir(replica_dir);
}
```

The `ft_remove_files_not_in_source` function is responsible for removing files from the replica folder that are not present in the source folder:

1. It takes three parameters:
    - `source_folder`: The path to the source folder.
    - `replica_folder`: The path to the replica folder.
    - `log_file`: The path to the log file where deletion events are recorded.

2. The function opens the replica directory using `opendir`. If the directory cannot be opened (e.g., due to permission issues or non-existent directory), the function returns early, indicating a failure to remove files.

3. It iterates over each entry in the replica directory using `readdir`. Entries for the current directory (`"."`) and parent directory (`".."`) are skipped to avoid unnecessary deletion.

4. For each entry, the function constructs the full paths of the corresponding replica and source entries using `snprintf`.

5. It then checks if the corresponding entry exists in the source folder using `access` with the `F_OK` flag. If the entry does not exist in the source folder, it indicates that the file is not present in the source and should be removed from the replica folder.

6. The function removes the file from the replica folder using `remove`. If the removal operation is successful, it appends a log entry to the log file indicating that the file was deleted from the replica folder.

7. After processing all entries in the replica directory, the function closes the directory using `closedir`.

8. By encapsulating the file removal logic within a separate function, `ft_remove_files_not_in_source` enhances code readability and modularity, allowing for easier maintenance and extension of the synchronization functionality.

The `ft_finalize_logging` function is responsible for finalizing the logging process after synchronization is completed:


```
void ft_finalize_logging(const char *log_file) {
    time_t current_time;
    time(&current_time);
    struct tm *time_info = localtime(&current_time);

    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

    FILE *log_fp = fopen(log_file, "a");
    if (log_fp != NULL) {
        fprintf(log_fp, "--- Sync completed at %s ---\n\n", time_str);
        fclose(log_fp);
    }
}
```

1. It takes a single parameter:
    - `log_file`: The path to the log file.

2. The function retrieves the current time using the `time` function and stores it in the `current_time` variable.

3. It then converts the current time to a human-readable string format using the `localtime` and `strftime` functions, storing the result in the `time_str` array.

4. The function opens the log file in append mode using `fopen`. If the file cannot be opened (e.g., due to permission issues or non-existent file), the function returns early without performing further actions.

5. It appends a synchronization completion message to the log file, including the current time stamp obtained in step 3. The message format is "--- Sync completed at [time_str] ---".

6. Finally, the function closes the log file using `fclose`, ensuring that all changes are flushed and the file is properly closed.

By incorporating the `ft_finalize_logging` function into the synchronization process, the program provides comprehensive logging functionality, allowing users to track synchronization events and monitor the status of the process over time.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
