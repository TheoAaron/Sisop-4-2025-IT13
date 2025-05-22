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
#include <stdlib.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <zlib.h>

<<<<<<< HEAD
#define BASEDIR "/home/ubuntu/maimai_fs/chiho"
=======
#define BASEDIR "/home/aaron31/Sisop-4-2025-IT13/soal_4/chiho"
>>>>>>> f38c36f (Update README)
#define MAX_PATH 1024

static const char *areas[] = {
    "starter", "metro", "dragon", "blackrose", "heaven", "youth"
};
static const int n_areas = sizeof(areas) / sizeof(areas[0]);

#define AES_KEYLEN 32
#define AES_BLOCKLEN 16

const unsigned char aes_key[AES_KEYLEN] = "41204a63d38dcb7432c9265ba03e62a8";

void shift_file_name(char *dst, const char *src) {
    for(int i = 0; src[i]; i++){
        dst[i] = src[i] + (i % 256);
    }
    dst[strlen(src)] = '\0';
}

void rot_13(char *str){
    for(int i = 0; str[i]; i++){
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = 'a' + (str[i] - 'a' + 13) % 26;
        }
        else if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = 'A' + (str[i] - 'A' + 13) % 26;
        }
    }
}

int aes_encrypt(const char *in, const char *out) {
    FILE *fin = fopen(in, "rb");
    FILE *fout = fopen(out, "wb");

    if(!fin || !fout) return -1;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if(!ctx) {
        fclose(fin);
        fclose(fout);
        return -1;
    }
    
    unsigned char iv[AES_BLOCKLEN];
    RAND_bytes(iv, AES_BLOCKLEN);
    fwrite(iv, 1, AES_BLOCKLEN, fout);

    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, iv);

    unsigned char inbuf[1024], outbuf[1024 + EVP_MAX_BLOCK_LENGTH];
    int inlen, outlen;
    while((inlen = fread(inbuf, 1, 1024, fin)) > 0){
        if(!EVP_EncryptUpdate(ctx, outbuf, &outlen, inbuf, inlen)) {
            EVP_CIPHER_CTX_free(ctx);
            fclose(fin);
            fclose(fout);
            return -1;
        }
        fwrite(outbuf, 1, outlen, fout);
    }
    
    if(!EVP_EncryptFinal_ex(ctx, outbuf, &outlen)) {
        EVP_CIPHER_CTX_free(ctx);
        fclose(fin);
        fclose(fout);
        return -1;
    }
    fwrite(outbuf, 1, outlen, fout);
    
    EVP_CIPHER_CTX_free(ctx);
    fclose(fin); 
    fclose(fout);
    return 0;
}

int aes_decrypt(const char *in, const char *out) {
    FILE *fin = fopen(in, "rb");
    FILE *fout = fopen(out, "wb");

    if(!fin || !fout) return -1;

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if(!ctx) {
        fclose(fin);
        fclose(fout);
        return -1;
    }
    
    unsigned char iv[AES_BLOCKLEN];
    if(fread(iv, 1, AES_BLOCKLEN, fin) != AES_BLOCKLEN) {
        EVP_CIPHER_CTX_free(ctx);
        fclose(fin);
        fclose(fout);
        return -1;
    }

    if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, iv)) {
        EVP_CIPHER_CTX_free(ctx);
        fclose(fin);
        fclose(fout);
        return -1;
    }

    unsigned char inbuf[1024], outbuf[1024 + EVP_MAX_BLOCK_LENGTH];
    int inlen, outlen;
    
    while((inlen = fread(inbuf, 1, 1024, fin)) > 0) {
        if(!EVP_DecryptUpdate(ctx, outbuf, &outlen, inbuf, inlen)) {
            EVP_CIPHER_CTX_free(ctx);
            fclose(fin);
            fclose(fout);
            return -1;
        }
        fwrite(outbuf, 1, outlen, fout);
    }
    
    if(!EVP_DecryptFinal_ex(ctx, outbuf, &outlen)) {
        EVP_CIPHER_CTX_free(ctx);
        fclose(fin);
        fclose(fout);
        return -1;
    }
    fwrite(outbuf, 1, outlen, fout);

    EVP_CIPHER_CTX_free(ctx);
    fclose(fin);
    fclose(fout);
    return 0;
}

int compress_to_gzip(const char *in, const char *out){
    FILE *fin = fopen(in, "rb");
    gzFile fout = gzopen(out, "wb");

    if(!fin || !fout) {
        if (fin) fclose(fin);
        if (fout) gzclose(fout);
        return -1;
    }

    char buff[1024];
    int len;
    while((len = fread(buff, 1, sizeof(buff), fin)) > 0){
        gzwrite(fout, buff, len);
    }

    fclose(fin);
    gzclose(fout);
    return 0;
}

int decompress_gzip(const char *in, const char *out){
    gzFile fin = gzopen(in, "rb");
    FILE *fout = fopen(out, "wb");
    if(!fin || !fout) {
        if (fin) gzclose(fin);
        if (fout) fclose(fout);
        return -1;
    }

    char buff[1024];
    int len;
    while((len = gzread(fin, buff, sizeof(buff))) > 0){
        fwrite(buff, 1, len, fout);
    }

    gzclose(fin);
    fclose(fout);
    return 0;
}

// Forward declaration for map_7sref_to_real
static int map_7sref_to_real(const char *path, char *real_out);

static int get_real_path(const char *path, char *real_out){
    if(strncmp(path, "/7sref", 6) == 0){
        return map_7sref_to_real(path, real_out);
    }else{
        snprintf(real_out, MAX_PATH, "%s%s", BASEDIR, path);
        return 0;
    }
}

// Implementation of map_7sref_to_real
static int map_7sref_to_real(const char *path, char *real_out) {
    if (strncmp(path, "/7sref/", 7) != 0) {
        return -1;
    }
    
    const char *subpath = path + 7; // Skip "/7sref/"
    char area[64] = {0};
    char filename[256] = {0};
    
    // Find the first underscore to separate area and filename
    const char *underscore = strchr(subpath, '_');
    if (!underscore) {
        return -1;
    }
    
    strncpy(area, subpath, underscore - subpath);
    area[underscore - subpath] = '\0';
    
    // Check if area is valid
    int area_valid = 0;
    for (int i = 0; i < n_areas; i++) {
        if (strcmp(area, areas[i]) == 0) {
            area_valid = 1;
            break;
        }
    }
    
    if (!area_valid) {
        return -1;
    }
    
    // Get filename part
    strcpy(filename, underscore + 1);
    
    // Construct real path
    snprintf(real_out, MAX_PATH, "%s/%s/%s", BASEDIR, area, filename);
    
    // For starter area, add .mai extension if not already present
    if (strcmp(area, "starter") == 0 && !strstr(filename, ".mai")) {
        strcat(real_out, ".mai");
    }
    
    return 0;
}

/* get file attributes */
static int fs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
    (void)fi;
    memset(stbuf, 0, sizeof(*stbuf));

    if (strcmp(path, "/") == 0 || strcmp(path, "/7sref") == 0) {
        stbuf->st_mode = S_IFDIR | 0555;
        stbuf->st_nlink = 2;
        return 0;
    }
    
    // Check if path is an area directory
    for (int i = 0; i < n_areas; i++) {
        char area_path[MAX_PATH];
        snprintf(area_path, MAX_PATH, "/%s", areas[i]);
        if (strcmp(path, area_path) == 0) {
            stbuf->st_mode = S_IFDIR | 0555;
            stbuf->st_nlink = 2;
            return 0;
        }
    }

    if (strncmp(path, "/7sref/", 7) == 0) {
        char real_path[MAX_PATH];
        if (map_7sref_to_real(path, real_path) < 0)
            return -ENOENT;
        if (lstat(real_path, stbuf) == -1)
            return -errno;
        return 0;
    }
    
    // For regular files in areas
    char real_path[MAX_PATH];
    if (get_real_path(path, real_path) < 0)
        return -ENOENT;
    
    if (lstat(real_path, stbuf) == -1)
        return -errno;
    
    return 0;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                      off_t offset, struct fuse_file_info *fi,
                      enum fuse_readdir_flags flags) {
    (void)offset;
    (void)fi;
    (void)flags;

    filler(buf, ".", NULL, 0, 0);
    filler(buf, "..", NULL, 0, 0);

    if (strcmp(path, "/") == 0) {
        filler(buf, "7sref", NULL, 0, 0);

        for(int i = 0; i < n_areas; i++){
            filler(buf, areas[i], NULL, 0, 0);
        }
        return 0;
    }

    if (strcmp(path, "/7sref") == 0) {
        for (int i = 0; i < n_areas; i++) {
            char dirpath[MAX_PATH];
            snprintf(dirpath, sizeof(dirpath), "%s/%s", BASEDIR, areas[i]);
            DIR *dp = opendir(dirpath);
            if (!dp) continue;
            struct dirent *de;
            while ((de = readdir(dp)) != NULL) {
                if (de->d_name[0] == '.') continue;
                char entry[512];
                snprintf(entry, sizeof(entry), "%s_%s", areas[i], de->d_name);
                filler(buf, entry, NULL, 0, 0);
            }
            closedir(dp);
        }
        return 0;
    }

    char real_path[MAX_PATH];
    if(get_real_path(path, real_path) < 0){
        return -ENOENT;
    }

    DIR *dp = opendir(real_path);
    if(!dp) return -errno;

    struct dirent *de;
    while((de = readdir(dp)) != NULL){
        if(de->d_name[0] == '.') continue;

        if(strncmp(path, "/starter", 8) == 0){
            const char *ext = strrchr(de->d_name, '.');
            if(ext && strcmp(ext, ".mai") == 0){
                char cleanname[512];
                size_t name_len = ext - de->d_name;
                strncpy(cleanname, de->d_name, name_len);
                cleanname[name_len] = '\0';
                filler(buf, cleanname, NULL, 0, 0);
            }
        } else {
            filler(buf, de->d_name, NULL, 0, 0);
        }
    }

    closedir(dp);
    return 0;
}

/* open file */
static int fs_open(const char *path, struct fuse_file_info *fi) {
    char real_path[MAX_PATH];
    if (get_real_path(path, real_path) < 0)
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

    if(strstr(path, "/dragon/") != NULL){
        rot_13(buf);
    }
    return res;
}

/* write data */
static int fs_write(const char *path, const char *buf, size_t size,
                    off_t offset, struct fuse_file_info *fi) {
    char *rot_buff = (char *)malloc(size);
    if(!rot_buff) return -ENOMEM;
    
    memcpy(rot_buff, buf, size);

    if(strstr(path, "/dragon/") != NULL){
        rot_13(rot_buff);
    }

    int res = pwrite(fi->fh, rot_buff, size, offset);
    free(rot_buff);

    if(res < 0) return -errno;
    return res;
}

/* create file */
static int fs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char real_path[MAX_PATH];
    if(get_real_path(path, real_path) < 0){
        return -ENOENT;
    }

    if(strncmp(path, "/starter/", 9) == 0 && !strstr(real_path, ".mai")){
        strcat(real_path, ".mai");
    }

    int fd = open(real_path, fi->flags | O_CREAT, mode);
    if(fd < 0) return -errno;

    fi->fh = fd;
    return 0;
}

/* unlink file */
static int fs_unlink(const char *path) {
    char real_path[MAX_PATH];
    if(get_real_path(path, real_path) < 0) return -ENOENT;
    int res = unlink(real_path);
    if(res < 0) return -errno;
    return 0;
}

/* release file handle */
static int fs_release(const char *path, struct fuse_file_info *fi) {
    (void) path;
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