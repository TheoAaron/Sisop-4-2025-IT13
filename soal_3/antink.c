#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

static const char *source_dir;
static const char *log_dir;

void log_warning(const char *filename) {
    time_t now;
    time(&now);
    char log_path[1024];
    snprintf(log_path, sizeof(log_path), "%s/antink.log", log_dir);
    
    FILE *log_file = fopen(log_path, "a");
    if (log_file) {
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
        fprintf(log_file, "[%s] WARNING: Suspicious file detected - %s\n", time_str, filename);
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

char* convert_path_if_needed(const char *path) {
    char *full_path = (char *)malloc(1024);
    char *dirname = strdup(path);
    char *basename = NULL;
    
    char *last_slash = strrchr(dirname, '/');
    if (last_slash) {
        *last_slash = '\0';
        basename = last_slash + 1;
    } else {
        basename = dirname;
        dirname = strdup("");
    }
    
    snprintf(full_path, 1024, "%s%s", source_dir, dirname);
    
    if (strlen(basename) > 0) {
        DIR *dp = opendir(full_path);
        if (dp) {
            struct dirent *de;
            int found = 0;
            
            char direct_path[1024];
            snprintf(direct_path, 1024, "%s/%s", full_path, basename);
            if (access(direct_path, F_OK) == 0) {
                strcat(full_path, "/");
                strcat(full_path, basename);
                found = 1;
            } else {
                char reversed_name[256];
                strcpy(reversed_name, basename);
                reverse_string(reversed_name);
                
                while ((de = readdir(dp)) != NULL && !found) {
                    if (is_suspicious(de->d_name) && strcmp(reversed_name, de->d_name) == 0) {
                        strcat(full_path, "/");
                        strcat(full_path, de->d_name);
                        found = 1;
                        break;
                    }
                }
                
                if (!found) {
                    strcat(full_path, "/");
                    strcat(full_path, basename);
                }
            }
            
            closedir(dp);
        } else {
            strcat(full_path, "/");
            strcat(full_path, basename);
        }
    }
    
    if (dirname != path) {
        free(dirname);
    }
    
    return full_path;
}

static int antink_getattr(const char *path, struct stat *stbuf) {
    char *full_path = convert_path_if_needed(path);
    
    int res = lstat(full_path, stbuf);
    free(full_path);
    
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
        }
        
        filler(buf, displayed_name, &st, 0);
    }
    
    closedir(dp);
    return 0;
}

static int antink_open(const char *path, struct fuse_file_info *fi) {
    char *full_path = convert_path_if_needed(path);
    
    int fd = open(full_path, fi->flags);
    free(full_path);
    
    if (fd == -1) return -errno;
    
    fi->fh = fd;
    return 0;
}

static int antink_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
    int fd;
    int res;
    
    if (fi == NULL) {
        char *full_path = convert_path_if_needed(path);
        fd = open(full_path, O_RDONLY);
        free(full_path);
        if (fd == -1) return -errno;
    } else {
        fd = fi->fh;
    }
    
    res = pread(fd, buf, size, offset);
    if (res == -1) res = -errno;
    
    if (fi == NULL) {
        close(fd);
    }
    
    return res;
}

static int antink_release(const char *path, struct fuse_file_info *fi) {
    close(fi->fh);
    return 0;
}

static struct fuse_operations antink_oper = {
    .getattr = antink_getattr,
    .readdir = antink_readdir,
    .open = antink_open,
    .read = antink_read,
    .release = antink_release,
};

int main(int argc, char *argv[]) {
    source_dir = getenv("SOURCE_DIR");
    log_dir = getenv("LOG_DIR");
    
    if (!source_dir) source_dir = "/it24_host";
    if (!log_dir) log_dir = "/antink_logs";
    
    mkdir(log_dir, 0755);
    
    if (argc > 1) {
        return fuse_main(argc, argv, &antink_oper, NULL);
    } 
    else {
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
}