#define FUSE_USE_VERSION 31
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BASEDIR "/path/ke/fuse_dir"

static const char *areas[] = {
    "starter", "metro", "dragon", "blackrose", "heaven", "youth"
};
static const int n_areas = sizeof(areas) / sizeof(areas[0]);

static int is_valid_area(const char *a) {
    for (int i = 0; i < n_areas; i++) {
        if (strcmp(a, areas[i]) == 0)
            return 1;
    }
    return 0;
}

/* Mapping path /7sref/<area>_<file> ke real path */
static int map_7sref_to_real(const char *path, char *real_out) {
    if (strncmp(path, "/7sref/", 7) != 0)
        return -1;
    char area[32], fname[256];
    if (sscanf(path + 7, "%31[^_]_%255[^"]", area, fname) != 2)
        return -1;
    if (!is_valid_area(area))
        return -1;
    snprintf(real_out, 512, "%s/%s/%s", BASEDIR, area, fname);
    return 0;
}

/* get file attributes */
static int fs_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(*stbuf));

    if (strcmp(path, "/") == 0 || strcmp(path, "/7sref") == 0) {
        stbuf->st_mode = S_IFDIR | 0555;
        stbuf->st_nlink = 2;
        return 0;
    }

    if (strncmp(path, "/7sref/", 7) == 0) {
        char real_path[512];
        if (map_7sref_to_real(path, real_path) < 0)
            return -ENOENT;
        if (lstat(real_path, stbuf) == -1)
            return -errno;
        return 0;
    }

    return -ENOENT;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi) {
    (void)offset;
    (void)fi;

    if (strcmp(path, "/") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        filler(buf, "7sref", NULL, 0);
        return 0;
    }

    if (strcmp(path, "/7sref") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);
        for (int i = 0; i < n_areas; i++) {
            char dirpath[512];
            DIR *dp = opendir(strcat(strdup(BASEDIR), "/")); 
            sprintf(dirpath, "%s/%s", BASEDIR, areas[i]);
            dp = opendir(dirpath);
            if (!dp) continue;
            struct dirent *de;
            while ((de = readdir(dp)) != NULL) {
                if (de->d_name[0] == '.') continue;
                char entry[512];
                snprintf(entry, sizeof(entry), "%s_%s", areas[i], de->d_name);
                filler(buf, entry, NULL, 0);
            }
            closedir(dp);
        }
        return 0;
    }

    return -ENOENT;
}

/* open file */
static int fs_open(const char *path, struct fuse_file_info *fi) {
    char real_path[512];
    if (map_7sref_to_real(path, real_path) < 0)
        return -ENOENT;
    int fd = open(real_path, fi->flags);
    if (fd < 0) return -errno;
    fi->fh = fd;
    return 0;
}

/* read data */
static int fs_read(const char *path, char *buf, size_t size,
                   off_t offset, struct fuse_file_info *fi) {
    int res = pread(fi->fh, buf, size, offset);
    if (res < 0) return -errno;
    return res;
}

/* write data */
static int fs_write(const char *path, const char *buf, size_t size,
                    off_t offset, struct fuse_file_info *fi) {
    int res = pwrite(fi->fh, buf, size, offset);
    if (res < 0) return -errno;
    return res;
}

/* create file */
static int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char real_path[512];
    if (map_7sref_to_real(path, real_path) < 0)
        return -ENOENT;
    int fd = open(real_path, fi->flags | O_CREAT, mode);
    if (fd < 0) return -errno;
    fi->fh = fd;
    return 0;
}

/* unlink file */
static int fs_unlink(const char *path) {
    char real_path[512];
    if (map_7sref_to_real(path, real_path) < 0)
        return -ENOENT;
    if (unlink(real_path) < 0)
        return -errno;
    return 0;
}

/* release file handle */
static int fs_release(const char *path, struct fuse_file_info *fi) {
    close(fi->fh);
    return 0;
}

/* register operations */
static struct fuse_operations fs_ops = {
    .getattr = fs_getattr,
    .readdir = fs_readdir,
    .open    = fs_open,
    .read    = fs_read,
    .write   = fs_write,
    .create  = fs_create,
    .unlink  = fs_unlink,
    .release = fs_release,
};

int main(int argc, char *argv[]) {
    return fuse_main(argc, argv, &fs_ops, NULL);
}
