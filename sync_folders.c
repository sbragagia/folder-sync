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

void ft_sync_folders(const char *source_folder, const char *replica_folder, const char *log_file);
void ft_sync_directory(const char *source_folder, const char *replica_folder, const char *log_file);
void ft_copy_file(const char *source_path, const char *replica_path, const char *log_file);
void ft_remove_files_not_in_source(const char *source_folder, const char *replica_folder, const char *log_file);
void ft_finalize_logging(const char *log_file);

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

void ft_sync_folders(const char *source_folder, const char *replica_folder, const char *log_file) {
    ft_sync_directory(source_folder, replica_folder, log_file);
    ft_finalize_logging(log_file);
}

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