#define FUSE_USE_VERSION 30
#define _GNU_SOURCE
#include <fuse.h>
#include <syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define LOG_FILE "/var/log/it24.log"

static const char *source_dir;
static const char *log_dir;

void write_log(const char *type, const char *message, const char *filename) {
    time_t now;
    time(&now);
    struct tm *tm_info = localtime(&now);
    
    char timestamp[20];
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", tm_info);
    
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file) {
        fprintf(log_file, "[%s] [%s] %s: %s\n", 
                timestamp, type, message, filename);
        fclose(log_file);
    }
    
    syslog(LOG_NOTICE, "[%s] %s: %s", type, message, filename);
}

void rot13(char *str) {
    if (!str) return;
    
    for (int i = 0; str[i]; i++) {
        char c = str[i];
        if (c >= 'a' && c <= 'z') {
            str[i] = ((c - 'a' + 13) % 26) + 'a';
        } else if (c >= 'A' && c <= 'Z') {
            str[i] = ((c - 'A' + 13) % 26) + 'A';
        }
    }
}

void log_warning(const char *filename) {
    time_t now;
    time(&now);
    char log_path[1024];
    snprintf(log_path, sizeof(log_path), "%s/antink.log", log_dir);
    
    FILE *log_file = fopen(log_path, "a");
    if (log_file) {
        fprintf(log_file, "[%s] WARNING: Suspicious file detected - %s\n", ctime(&now), filename);
        fclose(log_file);
    }
}

int is_suspicious(const char *filename) {
    return strstr(filename, "nafis") != NULL || strstr(filename, "kimcun") != NULL;
}

void reverse_string(char *str) {
    if (str) {
        int i = 0;
        int j = strlen(str) - 1;
        while (i < j) {
            char temp = str[i];
            str[i] = str[j];
            str[j] = temp;
            i++;
            j--;
        }
    }
}

static int antink_getattr(const char *path, struct stat *stbuf) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);
    
    int res = lstat(full_path, stbuf);
    if (res == -1) return -errno;
    
    return 0;
}

static int antink_readdir(const char *path, void *buf, fuse_fill_dir_t filler, 
                         off_t offset, struct fuse_file_info *fi) {
    DIR *dp;
    struct dirent *de;
    char full_path[1024];
    
    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);
    dp = opendir(full_path);
    if (dp == NULL) return -errno;
    
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    
    while ((de = readdir(dp)) != NULL) {
        struct stat st;
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        
        char displayed_name[256];
        strcpy(displayed_name, de->d_name);
        
        if (is_suspicious(de->d_name)) {
        log_warning(de->d_name);
        reverse_string(displayed_name);
        write_log("REVERSED", "File has been reversed", displayed_name);
        }

        filler(buf, displayed_name, &st, 0);
    }
    
    closedir(dp);
    return 0;
}

static int antink_open(const char *path, struct fuse_file_info *fi) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);
    
    int res = open(full_path, fi->flags);
    if (res == -1) return -errno;
    
    close(res);
    return 0;
}
 
static int antink_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s%s", source_dir, path);
    
    int fd = open(full_path, O_RDONLY);
    if (fd == -1) return -errno;
    
    int res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;
    
    if (!is_suspicious(path)) {
    rot13(buf);
    }

    close(fd);
    return res;
}

static struct fuse_operations antink_oper = {
    .getattr = antink_getattr,
    .readdir = antink_readdir,
    .open = antink_open,
    .read = antink_read,
};

int main(int argc, char *argv[]) {
    source_dir = getenv("SOURCE_DIR");
    log_dir = getenv("LOG_DIR");
    
    if (!source_dir) source_dir = "/it24_host";
    if (!log_dir) log_dir = "/antink_logs";
    
    mkdir(log_dir, 0755);
    
    char *fuse_argv[] = {
        argv[0],
        "-f",  
        "-d",  
        "/antink_mount",  
        NULL
    };
    int fuse_argc = 4;
    
    printf("=== Starting AntiNK FUSE Filesystem ===\n");
    printf("Source directory: %s\n", source_dir);
    printf("Log directory: %s\n", log_dir);
    printf("Mount point: /antink_mount\n");
    
    return fuse_main(fuse_argc, fuse_argv, &antink_oper, NULL);
}
