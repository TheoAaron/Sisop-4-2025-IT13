#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define ZIP_URL "https://drive.google.com/uc?export=download&id=1hi_GDdP51Kn2JJMw02WmCOxuc3qrXzh5"
#define ZIP_NAME "anomaly.zip"
#define ANOMALY_DIR "anomali"   // sesuai hasil unzip otomatis
#define IMAGE_DIR "image"
#define LOG_FILE "conversion.log"

void create_dir_if_not_exists(const char *dirname) {
    struct stat st = {0};
    if (stat(dirname, &st) == -1) {
        mkdir(dirname, 0755);
    }
}

void get_current_timestamp(char *date_str, char *time_str) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(date_str, 20, "%Y-%m-%d", t);
    strftime(time_str, 20, "%H:%M:%S", t);
}

unsigned char hex_pair_to_byte(const char *hex) {
    unsigned int byte;
    sscanf(hex, "%2x", &byte);
    return (unsigned char)byte;
}

void convert_hex_to_image(const char *txt_file, const char *input_dir, const char *output_dir, FILE *log_fp) {
    char txt_path[256];
    snprintf(txt_path, sizeof(txt_path), "%s/%s", input_dir, txt_file);

    FILE *fp = fopen(txt_path, "r");
    if (!fp) {
        fprintf(stderr, "❌ Failed to open %s\n", txt_path);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *hex_data = malloc(size + 1);
    fread(hex_data, 1, size, fp);
    hex_data[size] = '\0';
    fclose(fp);

    char *filtered = malloc(size + 1);
    int j = 0;
    for (int i = 0; i < size; i++) {
        if ((hex_data[i] >= '0' && hex_data[i] <= '9') ||
            (hex_data[i] >= 'A' && hex_data[i] <= 'F') ||
            (hex_data[i] >= 'a' && hex_data[i] <= 'f')) {
            filtered[j++] = hex_data[i];
        }
    }
    filtered[j] = '\0';
    free(hex_data);

    int hex_len = strlen(filtered);
    int byte_len = hex_len / 2;
    unsigned char *bytes = malloc(byte_len);

    for (int i = 0; i < byte_len; i++) {
        bytes[i] = hex_pair_to_byte(&filtered[i * 2]);
    }

    free(filtered);

    char name_part[100];
    strncpy(name_part, txt_file, strlen(txt_file) - 4);
    name_part[strlen(txt_file) - 4] = '\0';

    char date[20], time_s[20];
    get_current_timestamp(date, time_s);

    char out_filename[256];
    snprintf(out_filename, sizeof(out_filename), "%s_image_%s_%s.png", name_part, date, time_s);

    char out_path[512];
    snprintf(out_path, sizeof(out_path), "%s/%s", output_dir, out_filename);

    FILE *img_fp = fopen(out_path, "wb");
    if (!img_fp) {
        fprintf(stderr, "❌ Failed to write image %s\n", out_path);
        free(bytes);
        return;
    }
    fwrite(bytes, 1, byte_len, img_fp);
    fclose(img_fp);
    free(bytes);

    fprintf(log_fp, "[%s][%s]: Successfully converted hexadecimal text %s to %s.\n", date, time_s, txt_file, out_filename);
    fflush(log_fp);
}

int main() {
    printf("⬇️ Downloading zip file...\n");
    char wget_cmd[512];
    snprintf(wget_cmd, sizeof(wget_cmd), "wget -O %s \"%s\"", ZIP_NAME, ZIP_URL);
    system(wget_cmd);

    printf("📦 Extracting zip file...\n");
    char unzip_cmd[512];
    snprintf(unzip_cmd, sizeof(unzip_cmd), "unzip -o %s -d .", ZIP_NAME);
    system(unzip_cmd);

    remove(ZIP_NAME);

    create_dir_if_not_exists(IMAGE_DIR);
    FILE *log_fp = fopen(LOG_FILE, "a");
    if (!log_fp) {
        perror("🛑 Cannot open conversion.log");
        return 1;
    }

    DIR *dir = opendir(ANOMALY_DIR);
    if (!dir) {
        perror("🛑 Cannot open anomaly directory");
        fclose(log_fp);
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".txt")) {
            convert_hex_to_image(entry->d_name, ANOMALY_DIR, IMAGE_DIR, log_fp);
        }
    }

    closedir(dir);
    fclose(log_fp);

    printf("✅ All conversions completed.\n");
    return 0;
}