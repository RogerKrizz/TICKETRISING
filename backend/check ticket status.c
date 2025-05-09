#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 256

// URL decode function
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

// Function to search the ticket
int search_ticket(const char *ticketId, const char *username, char *statusOut, char *detailsOut) {
    FILE *file = fopen("tickets.txt", "r");
    if (!file) {
        perror("Could not open tickets.txt");
        return 0;
    }

    char line[MAX_LINE_LENGTH];
    int found = 0;
    int match = 0;
    detailsOut[0] = '\0';

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "Ticket ID:") && strstr(line, ticketId)) {
            found = 1;
            strcat(detailsOut, line);
        } else if (found && strstr(line, "Username:") && strstr(line, username)) {
            strcat(detailsOut, line);
            match = 1;
        } else if (match && strstr(line, "Status:")) {
            strcat(detailsOut, line);
            strcpy(statusOut, line + strlen("Status: "));
            break;
        } else if (found && !match) {
            strcat(detailsOut, line);  // Save in case user match happens later
        }
    }

    fclose(file);
    return match;
}

int main(void) {
    printf("Content-type: text/html\n\n");

    // Read content length
    char *lenstr = getenv("CONTENT_LENGTH");
    int len = lenstr ? atoi(lenstr) : 0;
    if (len <= 0) {
        printf("<h2>No data received.</h2>");
        return 1;
    }

    // Read POST data
    char *postData = malloc(len + 1);
    fread(postData, 1, len, stdin);
    postData[len] = '\0';

    // Extract values
    char ticketId[MAX_LINE_LENGTH] = "";
    char username[MAX_LINE_LENGTH] = "";

    char *token = strtok(postData, "&");
    while (token != NULL) {
        if (strncmp(token, "ticketId=", 9) == 0) {
            url_decode(token + 9, ticketId);
        } else if (strncmp(token, "username=", 9) == 0) {
            url_decode(token + 9, username);
        }
        token = strtok(NULL, "&");
    }

    // Output
    char status[MAX_LINE_LENGTH] = "";
    char details[1024] = "";

    if (search_ticket(ticketId, username, status, details)) {
        printf("<h2>Ticket Status for (Ticket ID: %s)</h2>", ticketId);
        printf("<pre>%s</pre>", details);
        printf("<p><strong>Status:</strong> %s</p>", status);
    } else {
        printf("<h2>Ticket not found or username mismatch.</h2>");
    }

    free(postData);
    return 0;
}

