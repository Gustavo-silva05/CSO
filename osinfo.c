#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <ctype.h>
#include <dirent.h>

void get_version    (char *buffer, size_t size);
void get_uptime     (char *buffer, size_t size);
void get_downtime   (char *buffer, size_t size);
void get_datetime   (char *buffer, size_t size);
void get_cpuinfo    (char *buffer, size_t size);
void get_loadavg    (char *buffer, size_t size);
void get_io_stats   (char *buffer, size_t size);
void get_filesystem (char *buffer, size_t size);
void get_devices    (char *buffer, size_t size);
void get_net_devices (char *buffer, size_t size);
void get_process_list(char *buffer, size_t size);

int main() {
    while (1) {
        FILE *file = fopen("index.html", "w");
        if (!file) {
            perror("Can't open html file");
            return 1;
        }

        if (flock(fileno(file), LOCK_EX) != 0) {
            perror("Can't block file");
            fclose(file);
            return 1;
        }

        fprintf(file, "<html><head><meta http-equiv=\"refresh\" content=\"3\"></head><body>\n");
        fprintf(file, "<h1>System Info</h1>\n");

        char buffer[1024];

        get_version(buffer, sizeof(buffer));
        fprintf(file, "%s\n", buffer);

        get_uptime(buffer, sizeof(buffer));
        fprintf(file, "%s\n", buffer);

        get_downtime(buffer, sizeof(buffer));
        fprintf(file, "%s\n", buffer);

        get_datetime(buffer, sizeof(buffer));
        fprintf(file, "%s\n", buffer);

        get_cpuinfo(buffer, sizeof(buffer));
        fprintf(file, "%s\n", buffer);

        get_loadavg(buffer, sizeof(buffer));
        fprintf(file, "%s\n", buffer);

        get_io_stats(buffer, sizeof(buffer));
        fprintf(file, "%s", buffer);

        get_filesystem(buffer, sizeof(buffer));
        fprintf(file, "%s", buffer);

        get_devices(buffer, sizeof(buffer));
        fprintf(file, "%s", buffer);

        get_net_devices(buffer, sizeof(buffer));
        fprintf(file, "%s", buffer);
	
	get_process_list(buffer, sizeof(buffer));
        fprintf(file, "%s", buffer);

        fprintf(file, "</body></html>\n");

        flock(fileno(file), LOCK_UN);
        fclose(file);

        sleep(3); 
    }

    return 0;
}

void get_process_list(char *buffer, size_t size) {
    DIR * dir;
    struct dirent *entry;
    char temp[256];
    char res[1024] = {0}; // Store process list

    dir = opendir("/proc");
    if (!dir) {
        snprintf(buffer, size, "<p><strong>Process List:</strong> Unable to access /proc directory</p>\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Check if the directory name consists only of digits (i.e., it's a process ID)
        int is_process = 1;
        for (char *p = entry->d_name; *p; p++) {
            if (!isdigit(*p)) {
                is_process = 0;
                break;
            }
        }

        if (is_process) {
            snprintf(temp, sizeof(temp), "PID: %s\n", entry->d_name);
            strncat(res, temp, sizeof(res) - strlen(res) - 1);
        }
    }

    closedir(dir);

    snprintf(buffer, size, "<p><strong>Process List:</strong><br>%s</p>\n", res);
}

void get_net_devices(char *buffer, size_t size) {
    FILE *file;
    char temp[32];
    char res [1024] = {0};

    file = fopen("/proc/net/dev", "r");
    if (!file) {
        snprintf(buffer, size, "<p><strong>Network devices:</strong> The file didn't open</p>\n");
        return;
    }
    
    while (fgets(temp, sizeof(temp), file)) {
        // printf("%s", temp);
        strncat(res, temp, sizeof(res)-strlen(res)-1);
    }
    fclose(file);    

    snprintf(buffer, size, "<p><strong>Network devices:</strong> %s</p>\n", res);    
}

void get_devices(char *buffer, size_t size) {
    FILE *file;
    char temp[32];
    char res [1024] = {0};

    file = fopen("/proc/devices", "r");
    if (!file) {
        snprintf(buffer, size, "<p><strong>Devices:</strong> The file didn't open</p>\n");
        return;
    }
    
    while (fgets(temp, sizeof(temp), file)) {
        // printf("%s", temp);
        strncat(res, temp, sizeof(res)-strlen(res)-1);
    }
    fclose(file);

    snprintf(buffer, size, "<p><strong>Devices:</strong> %s</p>\n", res);
}

void get_filesystem(char *buffer, size_t size) {
    FILE *file;
    char temp[32];
    char res [1024] = {0};

    file = fopen("/proc/filesystems", "r");
    if (!file) {
        snprintf(buffer, size, "<p><strong>Filesystem:</strong> The file didn't open</p>\n");
        return;
    }
    
    while (fgets(temp, sizeof(temp), file)) {
        // printf("%s", temp);
        strncat(res, temp, sizeof(res)-strlen(res)-1);
    }
    fclose(file);

    snprintf(buffer, size, "<p><strong>Filesystem:</strong> %s</p>\n", res);
}

void get_io_stats (char *buffer, size_t size) {
    FILE *file;
    char temp[256];
    char res [1024] = {0};


    file = fopen("/proc/diskstats", "r");
    if (!file) {
        snprintf(buffer, size, "<p><strong>Input/Output opertaions:</strong> The file didn't open</p>\n");
        return;
    }
    while(fgets(temp, sizeof(temp), file)) {
        char * ptr = strtok(temp, " ");
        char aux [32]; 
        int i = 0;  
        while (ptr) {
            if (i==2) {
                snprintf(aux,  sizeof(aux),"\nDevice Name: %s ",ptr);
                strncat (res, aux, sizeof(res)-strlen(res)-1);
            }
            else if (i==3) {
                snprintf(aux, sizeof(aux),"- reads: %s ",ptr);
                strncat (res, aux, sizeof(res)-strlen(res)-1);
            }
            else if (i==7) {
                snprintf(aux, sizeof(aux),"- writes: %s\n",ptr);
                strncat (res, aux, sizeof(res)-strlen(res)-1);
                break;
            }
            ptr = strtok(0, " ");
            i++;
	    }
    }

    fclose(file);

    snprintf(buffer, sizeof(res), "<p><strong>Input/Output opertaions:</strong> %s</p>\n", res);
}


void get_version(char *buffer, size_t size) {
    FILE *file;
    char temp[256];

    file = fopen("/proc/version", "r");
    if (!file) {
        snprintf(buffer, size, "<p><strong>System version and Kernel:</strong> The file didn't open</p>\n");
        return;
    }

    if (!fgets(temp, sizeof(temp), file)) {
        snprintf(buffer, size, "<p><strong>System version and Kernel:</strong> Can't read file</p>\n");
        fclose(file);
        return;
    }
    fclose(file);

    snprintf(buffer, size, "<p><strong>System version and Kernel:</strong> %s</p>\n", temp);
}

void get_uptime(char *buffer, size_t size) {
    FILE *file;
    double uptime;

    file = fopen("/proc/uptime", "r");

    if (!file) {
        snprintf(buffer, size, "<p><strong>Uptime:</strong> The file didn't open</p>\n");
        return;
    }

    if (fscanf(file, "%lf", &uptime) != 1) {
        snprintf(buffer, size, "<p><strong>Uptime:</strong> Can't read file</p>\n");
        fclose(file);
        return;
    }
    fclose(file);

    int days = uptime / (24 * 3600);
    uptime = (int)uptime % (24 * 3600);
    int hours = uptime / 3600;
    uptime = (int)uptime % 3600;
    int minutes = uptime / 60;
    int seconds = (int)uptime % 60;

    snprintf(buffer, size, "<p><strong>Uptime:</strong> %02d:%02d:%02d:%02d</p>\n", days, hours, minutes, seconds);
}

void get_downtime(char *buffer, size_t size) {
    FILE *file;
    double downtime;

    file = fopen("/proc/uptime", "r");

    if (!file) {
        snprintf(buffer, size, "<p><strong>downtime:</strong> The file didn't open</p>\n");
        return;
    }

    if (fscanf(file, "%*lf %lf", &downtime) != 1) {
        snprintf(buffer, size, "<p><strong>downtime:</strong> Can't read file</p>\n");
        fclose(file);
        return;
    }
    fclose(file);

    int days = downtime / (24 * 3600);
    downtime = (int)downtime % (24 * 3600);
    int hours = downtime / 3600;
    downtime = (int)downtime % 3600;
    int minutes = downtime / 60;
    int seconds = (int)downtime % 60;

    snprintf(buffer, size, "<p><strong>downtime:</strong> %02d:%02d:%02d:%02d</p>\n", days, hours, minutes, seconds);
}

void get_datetime(char *buffer, size_t size) {
    FILE *file;
    char line[128], time[16], date[16];

    file = fopen("/proc/driver/rtc", "r");
    if (!file) {
        snprintf(buffer, size, "<p><strong>=DateTime:</strong> The file didn't open</p>\n");
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "rtc_time        : %15s", time) == 1) {
            continue;
        }
        if (sscanf(line, "rtc_date        : %15s", date) == 1) {
            break;
        }
    }

    fclose(file);

    snprintf(buffer, size, "<p><strong>DateTime:</strong> %s %s</p>\n", date, time);
}

void get_cpuinfo(char *buffer, size_t size) {
    FILE *file;
    char line[128], cpu[128] = "Unknown";
    double mhz = 0;
    int cores = 0;

    file = fopen("/proc/cpuinfo", "r");
    if (!file) {
        snprintf(buffer, size, "<p><strong>Processor Information:</strong> The file didn't open</p>\n");
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "model name", 10) == 0) {
            char *colon = strchr(line, ':'); 
            if (colon) {
                strncpy(cpu, colon + 2, sizeof(cpu) - 1); 
                cpu[strcspn(cpu, "\n")] = '\0';  
            }
        } 
        else if (sscanf(line, "cpu MHz         : %lf", &mhz) == 1) {
            continue;
        } 
        else if (sscanf(line, "cpu cores       : %d", &cores) == 1) {
            break;
        }
    }

    fclose(file);

    snprintf(buffer, size, "<p><strong>Processor Information:</strong> %s with %.2f MHz and %d core(s)</p>\n", cpu, mhz, cores);
}

void get_loadavg(char *buffer, size_t size) {
    FILE *file;
    double load1, load5, load15;

    file = fopen("/proc/loadavg", "r");
    if (!file) {
        snprintf(buffer, size, "<p><strong>Load Average:</strong> The file didn't open</p>\n");
        return;
    }

    if (fscanf(file, "%lf %lf %lf", &load1, &load5, &load15) != 3) {
        snprintf(buffer, size, "<p><strong>Load Average:</strong> Can't read file</p>\n");
        fclose(file);
        return;
    }

    fclose(file);

    snprintf(buffer, size, "<p><strong>Load Average:</strong> 1 min: %.2f, 5 min: %.2f, 15 min: %.2f</p>\n", load1, load5, load15);
}
