#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <ctype.h>
#include <dirent.h>

void get_version(char *buffer, size_t size);
void get_uptime(char *buffer, size_t size);
void get_downtime(char *buffer, size_t size);
void get_datetime(char *buffer, size_t size);
void get_cpuinfo(char *buffer, size_t size);
void get_loadavg(char *buffer, size_t size);
void get_cpu_usage(char *buffer, size_t size);
void get_memory_info(char *buffer, size_t size);
void get_io_stats(char *buffer, size_t size);
void get_filesystem(char *buffer, size_t size);
void get_devices(char *buffer, size_t size);
void get_net_devices(char *buffer, size_t size);
void get_process_list(char *buffer, size_t size);

int main()
{
    while (1)
    {
        FILE *file = fopen("index.html", "w");
        if (!file)
        {
            perror("Can't open html file");
            return 1;
        }

        if (flock(fileno(file), LOCK_EX) != 0)
        {
            perror("Can't block file");
            fclose(file);
            return 1;
        }

        fprintf(file, "<html><head><meta http-equiv=\"refresh\" content=\"3\"></head><body>\n");
        fprintf(file, "<h1>System Info</h1>\n");

        char buffer[2048];

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

        get_cpu_usage(buffer, sizeof(buffer));
        fprintf(file, "%s\n", buffer);

        get_memory_info(buffer, sizeof(buffer));
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

void get_process_list(char *buffer, size_t size)
{
    DIR *dir;
    struct dirent *entry;
    char temp[256];
    char res[2048] = {0};

    dir = opendir("/proc");
    if (!dir)
    {
        snprintf(buffer, size, "<p><strong>Process List:</strong> Unable to access /proc directory</p>\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        int is_process = 1;
        for (char *p = entry->d_name; *p; p++)
        {
            if (!isdigit(*p))
            {
                is_process = 0;
                break;
            }
        }

        if (is_process)
        {
            FILE *file;
            char pid[32];
            snprintf(pid, sizeof(pid), "/proc/%s/status", entry->d_name);
            file = fopen(pid, "r");
            if (!file)
            {
                snprintf(temp, sizeof(temp), "PID:%s - The comm file didn't open<br>\n", entry->d_name);
            }
            else
            {
                char aux[64] = {0};
                fgets(aux, sizeof(aux), file);
                fclose(file);
                aux[strcspn(aux, "\n")] = '\0';
                snprintf(temp, sizeof(temp), "PID:%s - %s<br>\n", entry->d_name, aux);
            }
            strncat(res, temp, sizeof(res) - strlen(res) - 1);
        }
    }

    closedir(dir);

    snprintf(buffer, size, "<p><strong>Process List:</strong><br>\n%s</p>\n", res);
}

void get_net_devices(char *buffer, size_t size)
{
    FILE *file;
    char temp[256];
    char res[1024] = {0};

    file = fopen("/proc/net/dev", "r");
    if (!file)
    {
        snprintf(buffer, size, "<p><strong>Network devices:</strong> The file didn't open</p>\n");
        return;
    }
    int i = 0;
    while (fgets(temp, sizeof(temp), file))
    {
        if (i != 0 && i != 1)
        {
            temp[strcspn(temp, "\n")] = '\0';
            char *ptr = strtok(temp, " ");
            char aux[32];
            int j = 0;
            while (ptr)
            {
                printf("%s\n", ptr);
                if (j == 0)
                {
                    snprintf(aux, sizeof(aux), "\nDevice Name: %s ", ptr);
                    strncat(res, aux, sizeof(res) - strlen(res) - 1);
                    strncat(res, "<br>\n", sizeof(res) - strlen(res) - 1);
                }
                else if (j == 1)
                {
                    snprintf(aux, sizeof(aux), "- receives(bytes): %s ", ptr);
                    strncat(res, aux, sizeof(res) - strlen(res) - 1);
                    strncat(res, "<br>\n", sizeof(res) - strlen(res) - 1);
                }
                else if (j == 9)
                {
                    snprintf(aux, sizeof(aux), "- transmissions(bytes): %s\n", ptr);
                    strncat(res, aux, sizeof(res) - strlen(res) - 1);
                    strncat(res, "<br>\n", sizeof(res) - strlen(res) - 1);

                    break;
                }
                ptr = strtok(0, " ");
                j++;
            }
            // printf("%s\n", temp);
        }
        i++;
    }
    fclose(file);

    snprintf(buffer, size, "<p><strong>Network devices:</strong><br>\n%s</p>\n", res);
}

void get_devices(char *buffer, size_t size)
{
    FILE *file;
    char temp[32];
    char res[1024] = {0};

    file = fopen("/proc/devices", "r");
    if (!file)
    {
        snprintf(buffer, size, "<p><strong>Devices:</strong> The file didn't open</p>\n");
        return;
    }

    while (fgets(temp, sizeof(temp), file))
    {
        temp[strcspn(temp, "\n")] = '\0';
        strncat(res, temp, sizeof(res) - strlen(res) - 1);
        strncat(res, "<br>\n", sizeof(res) - strlen(res) - 1);
    }
    fclose(file);

    snprintf(buffer, size, "<p><strong>Devices:</strong><br>\n%s</p>\n", res);
}

void get_filesystem(char *buffer, size_t size)
{
    FILE *file;
    char temp[32];
    char res[1024] = {0};

    file = fopen("/proc/filesystems", "r");
    if (!file)
    {
        snprintf(buffer, size, "<p><strong>Filesystem:</strong> The file didn't open</p>\n");
        return;
    }

    while (fgets(temp, sizeof(temp), file))
    {
        strncat(res, temp, sizeof(res) - strlen(res) - 1);
    }
    fclose(file);

    snprintf(buffer, size, "<p><strong>Filesystem:</strong> %s</p>\n", res);
}

void get_io_stats(char *buffer, size_t size)
{
    FILE *file;
    char temp[256];
    char res[1024] = {0};

    file = fopen("/proc/diskstats", "r");
    if (!file)
    {
        snprintf(buffer, size, "<p><strong>Input/Output opertaions:</strong> The file didn't open</p>\n");
        return;
    }
    while (fgets(temp, sizeof(temp), file))
    {
        char *ptr = strtok(temp, " ");
        char aux[32];
        int i = 0;
        while (ptr)
        {
            if (i == 2)
            {
                snprintf(aux, sizeof(aux), "\nDevice Name: %s ", ptr);
                strncat(res, aux, sizeof(res) - strlen(res) - 1);
            }
            else if (i == 3)
            {
                snprintf(aux, sizeof(aux), "- reads: %s ", ptr);
                strncat(res, aux, sizeof(res) - strlen(res) - 1);
            }
            else if (i == 7)
            {
                snprintf(aux, sizeof(aux), "- writes: %s\n", ptr);
                strncat(res, aux, sizeof(res) - strlen(res) - 1);
                break;
            }
            ptr = strtok(0, " ");
            i++;
        }
    }

    fclose(file);

    snprintf(buffer, sizeof(res), "<p><strong>Input/Output operations:</strong> %s</p>\n", res);
}

void get_version(char *buffer, size_t size)
{
    FILE *file;
    char temp[256];

    file = fopen("/proc/version", "r");
    if (!file)
    {
        snprintf(buffer, size, "<p><strong>System version and Kernel:</strong> The file didn't open</p>\n");
        return;
    }

    if (!fgets(temp, sizeof(temp), file))
    {
        snprintf(buffer, size, "<p><strong>System version and Kernel:</strong> Can't read file</p>\n");
        fclose(file);
        return;
    }

    fclose(file);

    snprintf(buffer, size, "<p><strong>System version and Kernel:</strong> %s</p>\n", temp);
}

void get_uptime(char *buffer, size_t size)
{
    FILE *file;
    double uptime;

    file = fopen("/proc/uptime", "r");

    if (!file)
    {
        snprintf(buffer, size, "<p><strong>Uptime:</strong> The file didn't open</p>\n");
        return;
    }

    if (fscanf(file, "%lf", &uptime) != 1)
    {
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

void get_downtime(char *buffer, size_t size)
{
    FILE *file;
    double downtime;

    file = fopen("/proc/uptime", "r");

    if (!file)
    {
        snprintf(buffer, size, "<p><strong>downtime:</strong> The file didn't open</p>\n");
        return;
    }

    if (fscanf(file, "%*lf %lf", &downtime) != 1)
    {
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

void get_datetime(char *buffer, size_t size)
{
    FILE *file;
    char line[128], time[16], date[16];

    file = fopen("/proc/driver/rtc", "r");

    if (!file)
    {
        snprintf(buffer, size, "<p><strong>=DateTime:</strong> The file didn't open</p>\n");
        return;
    }

    while (fgets(line, sizeof(line), file))
    {
        if (sscanf(line, "rtc_time        : %15s", time) == 1)
        {
            continue;
        }
        if (sscanf(line, "rtc_date        : %15s", date) == 1)
        {
            break;
        }
    }

    fclose(file);

    snprintf(buffer, size, "<p><strong>DateTime:</strong> %s %s</p>\n", date, time);
}

void get_cpuinfo(char *buffer, size_t size)
{
    FILE *file;
    char line[128], cpu[128] = "Unknown";
    double mhz = 0;
    int cores = 0;

    file = fopen("/proc/cpuinfo", "r");

    if (!file)
    {
        snprintf(buffer, size, "<p><strong>Processor Information:</strong> The file didn't open</p>\n");
        return;
    }

    while (fgets(line, sizeof(line), file))
    {
        if (strncmp(line, "model name", 10) == 0)
        {
            char *colon = strchr(line, ':');
            if (colon)
            {
                strncpy(cpu, colon + 2, sizeof(cpu) - 1);
                cpu[strcspn(cpu, "\n")] = '\0';
            }
        }
        else if (sscanf(line, "cpu MHz         : %lf", &mhz) == 1)
        {
            continue;
        }
        else if (sscanf(line, "cpu cores       : %d", &cores) == 1)
        {
            break;
        }
    }

    fclose(file);

    snprintf(buffer, size, "<p><strong>Processor Information:</strong> %s with %.2f MHz and %d core(s)</p>\n", cpu, mhz, cores);
}

void get_loadavg(char *buffer, size_t size)
{
    FILE *file;
    double load1, load5, load15;

    file = fopen("/proc/loadavg", "r");

    if (!file)
    {
        snprintf(buffer, size, "<p><strong>Load Average:</strong> The file didn't open</p>\n");
        return;
    }

    if (fscanf(file, "%lf %lf %lf", &load1, &load5, &load15) != 3)
    {
        snprintf(buffer, size, "<p><strong>Load Average:</strong> Can't read file</p>\n");
        fclose(file);
        return;
    }

    fclose(file);

    snprintf(buffer, size, "<p><strong>Load Average:</strong> 1 min: %.2f, 5 min: %.2f, 15 min: %.2f</p>\n", load1, load5, load15);
}

void get_cpu_usage(char *buffer, size_t size)
{
    unsigned long user1, nice1, system1, idle1, iowait1, irq1, softirq1;
    unsigned long user2, nice2, system2, idle2, iowait2, irq2, softirq2;
    unsigned long total1, total2, total_diff, idle_diff;
    double cpu_usage;
    FILE *file;

    file = fopen("/proc/stat", "r");
    if (!file)
    {
        snprintf(buffer, size, "<p><strong>CPU Usage:</strong> The file didn't open</p>\n");
        return;
    }

    if (fscanf(file, "cpu %lu %lu %lu %lu %lu %lu %lu", &user1, &nice1, &system1, &idle1, &iowait1, &irq1, &softirq1) != 7)
    {
        snprintf(buffer, size, "<p><strong>CPU Usage:</strong> Can't read file</p>\n");
        fclose(file);
        return;
    }

    fclose(file);

    sleep(1);

    file = fopen("/proc/stat", "r");
    if (!file)
    {
        snprintf(buffer, size, "<p><strong>CPU Usage:</strong> The file didn't open</p>\n");
        return;
    }

    if (fscanf(file, "cpu %lu %lu %lu %lu %lu %lu %lu", &user2, &nice2, &system2, &idle2, &iowait2, &irq2, &softirq2) != 7)
    {
        snprintf(buffer, size, "<p><strong>CPU Usage:</strong> Can't read file</p>\n");
        fclose(file);
        return;
    }

    fclose(file);

    total1 = user1 + nice1 + system1 + idle1 + iowait1 + irq1 + softirq1;
    total2 = user2 + nice2 + system2 + idle2 + iowait2 + irq2 + softirq2;

    total_diff = total2 - total1;
    idle_diff = idle2 - idle1;

    cpu_usage = (1 - (idle_diff) / (total_diff)) * 100.0;

    snprintf(buffer, size, "<p><strong>CPU Usage:</strong> %.2f%%</p>\n", cpu_usage);
}

void get_memory_info(char *buffer, size_t size)
{
    int memTotal = 0, memAvailable = 0;
    char label[32];
    int value;
    FILE *file;

    file = fopen("/proc/meminfo", "r");

    if (!file)
    {
        snprintf(buffer, size, "<p><strong>Memory:</strong> The file didn't open</p>\n");
        return;
    }

    while (fscanf(file, "%31s %lu kB", label, &value) == 2)
    {
        if (strcmp(label, "MemTotal:") == 0)
            memTotal = value;
        if (strcmp(label, "MemAvailable:") == 0)
            memAvailable = value;
    }

    fclose(file);

    int memUsed = memTotal - memAvailable;

    snprintf(buffer, size, "<p><strong>Memory:</strong> Total: %d MB, Used: %d MB</p>\n", memTotal / 1024, memUsed / 1024);
}