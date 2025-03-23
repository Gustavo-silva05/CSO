#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>

void get_version    (char *buffer, size_t size);
void get_uptime     (char *buffer, size_t size);
void get_downtime   (char *buffer, size_t size);
void get_datetime   (char *buffer, size_t size);
void get_cpuinfo    (char *buffer, size_t size);
void get_loadavg    (char *buffer, size_t size);
void get_io_stats   (char *buffer, size_t size);

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

        fprintf(file, "</body></html>\n");

        flock(fileno(file), LOCK_UN);
        fclose(file);

        sleep(3); 
    }

    return 0;
}




void get_io_stats (char *buffer, size_t size) {
    FILE *file;
    char temp[256];
    char res [256];


    file = fopen("/proc/diskstats", "r");
    if (!file) {
        snprintf(buffer, size, "<p><strong>Input/Output opertaions:</strong> The file didn't open</p>\n");
        return;
    }
    while(fgets(temp, sizeof(temp), file)) {
        printf("%s", temp);
        char * ptr = strtok(temp, " ");
        char aux [128]; 
        int i = 0;  
        while (ptr) {
            if (i==2) {
                sprintf(aux, "\nDevice Name: %s ",ptr);
                strncat (res, aux, sizeof(res));
            }
            else if (i==3) {
                sprintf(aux, "- leituras Completas: %s ",ptr);
                strncat (res, aux, sizeof(res));
            }
            else if (i==7) {
                sprintf(aux, "- Escritas Completas: %s ",ptr);
                strncat (res, aux, sizeof(res));
                break;
            }
            ptr = strtok(0, " ");
            i++;
	    }
        printf("%s", aux);    
    }

    fclose(file);

    snprintf(buffer, size, "<p><strong>Input/Output opertaions:</strong> %s</p>\n", res);
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
