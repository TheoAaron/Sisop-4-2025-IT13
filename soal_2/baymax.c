#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>
#include <curl/curl.h>
#include <sys/wait.h>
#include <ctype.h>
#include <limits.h>

#define BLOCK_SIZE 1024
#define LOG_FILE "/home/aaron31/Sisop-4-2025-IT13/soal_2/activity.log"
#define RELICS_ZIP_URL "https://drive.google.com/uc?export=download&id=1MHVhFT57Wa9Zcx69Bv9j5ImHc9rXMH1c"
#define RELICS_ZIP "relics.zip"

char base_dir[PATH_MAX];
char relics_dir[PATH_MAX];
char zip_path[PATH_MAX];

static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

int download_relics_zip() {
    printf("⬇️ Downloading relics.zip...\n");

    FILE *fp = fopen(RELICS_ZIP, "wb");
    if (!fp) {
        perror("🛑 Failed to download relics.zip");
        return -1;
    }
    
    CURL *curl = curl_easy_init();
    if (!curl) {
        fclose(fp);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, RELICS_ZIP_URL);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) return -1;

    printf("🟢 Download completed successfully\n");
    return 0;
}

int extract_relics_zip() {
    mkdir("relics", 0755);
    printf("📦 Extracting to relics/ directory...\n");
    
    pid_t pid = fork();
    if (pid == 0) {
        execlp("unzip", "unzip", "-o", RELICS_ZIP, "-d", "relics", NULL);
        perror("❌ Failed to extract");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("🟢 Extraction successful\n");
            unlink(RELICS_ZIP);
            return 0;
        }
    }
    return -1;
}

int relics_exists() {
    DIR *dir = opendir(relics_dir);
    if (!dir) return 0;

    struct dirent *entry;
    int has_files = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            has_files = 1;
            break;
        }
    }
    closedir(dir);
    return has_files;
}

void write_log(const char *action, const char *details) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        perror("Error opening log file");
        return;
    }

    time_t now = time(NULL);
    struct tm *tm = localtime(&now);
    fprintf(log_file, "[%04d-%02d-%02d %02d:%02d:%02d] %s: %s\n",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec,
            action, details);

    fclose(log_file);
}

void get_fragment_path(const char *path, int fragment_num, char *fragment_path) {
    if (!path || !fragment_path) return;
    
    char ext[16];
    snprintf(ext, sizeof(ext), ".%03d", fragment_num);
    
    size_t relics_len = strlen(relics_dir);
    size_t path_len = strlen(path + 1);
    size_t ext_len = strlen(ext);
    
    if (relics_len + 1 + path_len + ext_len >= PATH_MAX) {
        fragment_path[0] = '\0';
        return;
    }
    
    char *p = fragment_path;
    memcpy(p, relics_dir, relics_len);
    p += relics_len;
    *p++ = '/';
    memcpy(p, path + 1, path_len);
    p += path_len;
    memcpy(p, ext, ext_len + 1);
}

int count_fragments(const char *path) {
    char fragment_path[PATH_MAX];
    int count = 0;
    
    while (1) {
        get_fragment_path(path, count, fragment_path);
        if (access(fragment_path, F_OK) != 0) break;
        count++;
    }
    return count;
}

static int baymax_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void)fi;
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        return 0;
    }

    int fragment_count = count_fragments(path);
    if (fragment_count > 0) {
        stbuf->st_mode = S_IFREG | 0644;
        stbuf->st_nlink = 1;
        stbuf->st_size = fragment_count * BLOCK_SIZE;

        char last_fragment[PATH_MAX];
        get_fragment_path(path, fragment_count - 1, last_fragment);
        struct stat last_stat;
        if (stat(last_fragment, &last_stat) == 0) {
            stbuf->st_size = (fragment_count - 1) * BLOCK_SIZE + last_stat.st_size;
        }
        return 0;
    }

    return -ENOENT;
}

static int baymax_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi
                         #if FUSE_USE_VERSION >= 30
                         , enum fuse_readdir_flags flags
                         #endif
                        ) {
    (void)offset;
    (void)fi;
    #if FUSE_USE_VERSION >= 30
    (void)flags;
    #endif

    if (strcmp(path, "/") != 0) return -ENOENT;

    filler(buf, ".", NULL, 0
           #if FUSE_USE_VERSION >= 30
           , 0
           #endif
          );
    filler(buf, "..", NULL, 0
           #if FUSE_USE_VERSION >= 30
           , 0
           #endif
          );

    DIR *dp = opendir(relics_dir);
    if (!dp) return -errno;

    struct dirent *de;
    char prev_name[PATH_MAX] = "";
    while ((de = readdir(dp)) != NULL) {
        if (strlen(de->d_name) > 4 && de->d_name[strlen(de->d_name)-4] == '.') {
            char main_name[PATH_MAX];
            strncpy(main_name, de->d_name, strlen(de->d_name)-4);
            main_name[strlen(de->d_name)-4] = '\0';
            
            if (strcmp(main_name, prev_name) != 0) {
                filler(buf, main_name, NULL, 0
                       #if FUSE_USE_VERSION >= 30
                       , 0
                       #endif
                      );
                strcpy(prev_name, main_name);
            }
        }
    }
    closedir(dp);
    return 0;
}

static int baymax_open(const char *path, struct fuse_file_info *fi) {
    if (count_fragments(path) == 0) return -ENOENT;

    write_log("READ", path + 1);
    return 0;
}

static int baymax_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
    int fragment_num = offset / BLOCK_SIZE;
    int fragment_offset = offset % BLOCK_SIZE;
    size_t bytes_read = 0;
    char fragment_path[PATH_MAX];
    
    while (bytes_read < size) {
        get_fragment_path(path, fragment_num, fragment_path);
        FILE *f = fopen(fragment_path, "rb");
        if (!f) break;
        
        if (fragment_offset > 0) fseek(f, fragment_offset, SEEK_SET);
        size_t to_read = BLOCK_SIZE - fragment_offset;
        if (to_read > size - bytes_read) to_read = size - bytes_read;
        
        size_t actually_read = fread(buf + bytes_read, 1, to_read, f);
        bytes_read += actually_read;
        fclose(f);
        
        if (actually_read < to_read) break;
        
        fragment_num++;
        fragment_offset = 0;
    }
    return bytes_read;
}

static int baymax_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char fragment_path[PATH_MAX];
    get_fragment_path(path, 0, fragment_path);

    FILE *f = fopen(fragment_path, "wb");
    if (!f) return -errno;
    fclose(f);

    char log_msg[PATH_MAX];
    snprintf(log_msg, sizeof(log_msg), "%s -> %s.000", path + 1, path + 1);
    write_log("WRITE", log_msg);

    return 0;
}

static int baymax_write(const char *path, const char *buf, size_t size, off_t offset,
                       struct fuse_file_info *fi) {
    size_t bytes_written = 0;
    int fragment_num = offset / BLOCK_SIZE;
    int fragment_offset = offset % BLOCK_SIZE;
    size_t remaining = size;

    char log_detail[PATH_MAX * 10] = "";
    char *log_cursor = log_detail;
    size_t log_len = 0;

    while (remaining > 0) {
        char fragment_path[PATH_MAX];
        get_fragment_path(path, fragment_num, fragment_path);

        FILE *fp = fopen(fragment_path, offset == 0 ? "wb" : "r+b");
        if (!fp) return -errno;

        fseek(fp, fragment_offset, SEEK_SET);

        size_t chunk_size = BLOCK_SIZE - fragment_offset;
        if (chunk_size > remaining) chunk_size = remaining;

        size_t written = fwrite(buf + bytes_written, 1, chunk_size, fp);
        fclose(fp);

        if (written == 0) break;

        const char *fragment_name = strrchr(fragment_path, '/');
        if (!fragment_name) fragment_name = fragment_path;
        else fragment_name++;

        if (log_len + strlen(fragment_name) + 2 < sizeof(log_detail)) {
            if (log_len > 0) {
                *log_cursor++ = ',';
                *log_cursor++ = ' ';
                log_len += 2;
            }
            strcpy(log_cursor, fragment_name);
            log_cursor += strlen(fragment_name);
            log_len += strlen(fragment_name);
        }

        bytes_written += written;
        remaining -= written;
        fragment_num++;
        fragment_offset = 0;
    }

    char final_log[PATH_MAX * 10];
    snprintf(final_log, sizeof(final_log) - 1, "%s -> ", path + 1);
    strncat(final_log, log_detail, sizeof(final_log) - strlen(final_log) - 1);

    write_log("WRITE", final_log);

    return bytes_written;
}

static int baymax_unlink(const char *path) {
    char log_details[PATH_MAX * 10] = "";
    int fragment_num = 0;
    int found = 0;

    while (1) {
        char fragment_path[PATH_MAX];
        get_fragment_path(path, fragment_num, fragment_path);
        if (access(fragment_path, F_OK) != 0) break;

        if (unlink(fragment_path) == 0) {
            if (found > 0) strcat(log_details, ", ");
            const char *fragment_name = strrchr(fragment_path, '/');
            strcat(log_details, fragment_name ? fragment_name + 1 : fragment_path);
            found++;
        }
        fragment_num++;
    }

    if (found > 0) {
        write_log("DELETE", log_details);
        return 0;
    }

    return -ENOENT;
}

static int baymax_release(const char *path, struct fuse_file_info *fi) {
    (void) fi;

    if (count_fragments(path) == 0) return 0;

    char log_msg[PATH_MAX * 2];
    snprintf(log_msg, sizeof(log_msg), "%s -> %s", path + 1, base_dir);
    write_log("COPY", log_msg);

    return 0;
}


static struct fuse_operations baymax_oper = {
    .getattr = baymax_getattr,
    .readdir = baymax_readdir,
    .open = baymax_open,
    .read = baymax_read,
    .create = baymax_create,
    .write = baymax_write,
    .unlink = baymax_unlink,
    .release = baymax_release,
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <mountpoint>\n", argv[0]);
        return 1;
    }

    if (!realpath(argv[1], base_dir)) return 1;
    if (!getcwd(relics_dir, PATH_MAX)) return 1;
    
    size_t len = strlen(relics_dir);
    if (len + 8 < PATH_MAX) {
        memcpy(relics_dir + len, "/relics", 8);
    } else {
        return 1;
    }

    FILE *log_init = fopen(LOG_FILE, "w");
    if (!log_init) {
        perror("Failed to initialize log file");
        return 1;
    }
    fclose(log_init);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    if (access("relics", F_OK) != 0 || !relics_exists()) {
        if (download_relics_zip() == 0) extract_relics_zip();
    }
    curl_global_cleanup();

    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    int ret = fuse_main(args.argc, args.argv, &baymax_oper, NULL);
    fuse_opt_free_args(&args);

    return ret;
}