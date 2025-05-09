#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256

// URL decode function (decodes %XX and replaces + with space)
void url_decode(char *src, char *dest) {
    char *p = src;
    char code[3] = {0};
    while (*p) {
        if (*p == '%') {
            if (p[1] && p[2]) {
                code[0] = p[1];
                code[1] = p[2];
                *dest++ = (char) strtol(code, NULL, 16);
                p += 3;
            }
        } else if (*p == '+') {
            *dest++ = ' ';
            p++;
        } else {
            *dest++ = *p++;
        }
    }
    *dest = '\0';
}

// Function to update ticket status
int update_ticket_status(const char *ticketId, const char *newStatus) {
    FILE *file = fopen("tickets.txt", "r");
    if (!file) {
        perror("Error opening tickets.txt");
        return 0;
    }

    FILE *tempFile = fopen("tickets_temp.txt", "w");
    if (!tempFile) {
        fclose(file);
        perror("Error creating temporary file");
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    int isTargetTicket = 0;
    int updated = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "Ticket ID:", 10) == 0) {
            if (strstr(line, ticketId)) {
                isTargetTicket = 1;
                updated = 1;
            } else {
                isTargetTicket = 0;
            }
            fputs(line, tempFile);  // Always write Ticket ID
            continue;
        }

        if (isTargetTicket && strncmp(line, "Status:", 7) == 0) {
            fprintf(tempFile, "Status: %s\n", newStatus);
        } else {
            fputs(line, tempFile);
        }
    }

    fclose(file);
    fclose(tempFile);

    if (updated) {
        remove("tickets.txt");
        rename("tickets_temp.txt", "tickets.txt");
    } else {
        remove("tickets_temp.txt");
    }

    return updated;
}

int main(void) {
    printf("Content-type: text/plain\n\n");

    // Read POST data
    char *lenstr = getenv("CONTENT_LENGTH");
    int len = lenstr ? atoi(lenstr) : 0;

    if (len <= 0) {
        printf("No data received.\n");
        return 1;
    }

    char *postData = malloc(len + 1);
    if (!postData) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    fread(postData, 1, len, stdin);
    postData[len] = '\0';

    // Temporary variables for raw and decoded values
    char raw_ticketId[MAX_LINE_LENGTH] = "";
    char raw_status[MAX_LINE_LENGTH] = "";
    char ticketId[MAX_LINE_LENGTH] = "";
    char status[MAX_LINE_LENGTH] = "";

    sscanf(postData, "ticketId=%255[^&]&status=%255s", raw_ticketId, raw_status);
    free(postData);

    url_decode(raw_ticketId, ticketId);
    url_decode(raw_status, status);

    if (update_ticket_status(ticketId, status)) {
        printf("Success\n");
    } else {
        printf("Failed to update ticket status.\n");
    }

    return 0;
}

