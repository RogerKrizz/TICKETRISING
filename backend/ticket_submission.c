#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

typedef struct Ticket {
    int ticketID;
    char service[200];
    char pages[50];
    char copies[50];
    char username[50];
    char notes[500];
    char date[20];
    char time_str[20];
    struct Ticket* next;
} Ticket;

Ticket* createTicket(int ticketID, const char* service, const char* pages, const char* copies, const char* username, const char* notes, const char* date, const char* time_str) {
    Ticket* newTicket = (Ticket*)malloc(sizeof(Ticket));
    newTicket->ticketID = ticketID;
    strcpy(newTicket->service, service);
    strcpy(newTicket->pages, pages);
    strcpy(newTicket->copies, copies);
    strcpy(newTicket->username, username);
    strcpy(newTicket->notes, notes);
    strcpy(newTicket->date, date);
    strcpy(newTicket->time_str, time_str);
    newTicket->next = NULL;
    return newTicket;
}

void addTicket(Ticket** head, Ticket* newTicket) {
    if (*head == NULL) {
        *head = newTicket;
    } else {
        Ticket* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newTicket;
    }
}

void saveTicketsToFile(Ticket* head) {
    FILE *fp = fopen("tickets.txt", "a");
    if (fp == NULL) {
        printf("<h2>Could not open file to save ticket.</h2>");
        return;
    }

    Ticket* temp = head;
    while (temp != NULL) {
        fprintf(fp, "Ticket ID: %d\n", temp->ticketID);
        fprintf(fp, "Service: %s\n", temp->service);
        fprintf(fp, "Pages: %s\n", temp->pages);
        fprintf(fp, "Copies: %s\n", temp->copies);
        fprintf(fp, "Username: %s\n", temp->username);
        fprintf(fp, "Notes: %s\n", temp->notes);
        fprintf(fp, "Submitted Date: %s\n", temp->date);
        fprintf(fp, "Submitted Time: %s\n", temp->time_str);
        fprintf(fp, "Status: Pending\n");
        fprintf(fp, "----------------------------\n");
        temp = temp->next;
    }

    fclose(fp);
}

void urldecode(char *src, char *dest) {
    char a, b;
    while (*src) {
        if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10); else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10); else b -= '0';
            *dest++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dest++ = ' ';
            src++;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';
}

void get_field_value(const char *data, const char *fieldname, char *result, int maxlen) {
    char *start = strstr(data, fieldname);
    if (start) {
        start += strlen(fieldname) + 1;
        char *end = strchr(start, '&');
        int len = end ? (end - start) : strlen(start);
        if (len >= maxlen) len = maxlen - 1;
        strncpy(result, start, len);
        result[len] = '\0';
    } else {
        result[0] = '\0';
    }
}

int generate_ticket_id() {
    return 100000 + rand() % 900000;
}

int main() {
    printf("Content-Type: text/html\n\n");

    char *lenstr = getenv("CONTENT_LENGTH");
    if (!lenstr) {
        printf("<h2>Invalid data length.</h2>");
        return 1;
    }

    int len = atoi(lenstr);
    if (len <= 0 || len >= 4000) {
        printf("<h2>Invalid data length.</h2>");
        return 1;
    }

    char input[4000] = {0};
    fread(input, 1, len, stdin);
    input[len] = '\0';

    char raw_service[200], raw_pages[50], raw_copies[50], raw_username[50], raw_notes[500];
    char service[200], pages[50], copies[50], username[50], notes[500];

    get_field_value(input, "service", raw_service, sizeof(raw_service));
    get_field_value(input, "pages", raw_pages, sizeof(raw_pages));
    get_field_value(input, "copies", raw_copies, sizeof(raw_copies));
    get_field_value(input, "username", raw_username, sizeof(raw_username));
    get_field_value(input, "notes", raw_notes, sizeof(raw_notes));

    urldecode(raw_service, service);
    urldecode(raw_pages, pages);
    urldecode(raw_copies, copies);
    urldecode(raw_username, username);
    urldecode(raw_notes, notes);

    srand(time(NULL));
    int ticket_id = generate_ticket_id();

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char date[20], time_str[20];
    strftime(date, sizeof(date), "%Y-%m-%d", t);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", t);

    Ticket* newTicket = createTicket(ticket_id, service, pages, copies, username, notes, date, time_str);

    static Ticket* ticketList = NULL;
    addTicket(&ticketList, newTicket);

    saveTicketsToFile(ticketList);

    printf("<html><head><title>Redirecting...</title></head><body>");
    printf("<script>");
    printf("window.location.href = '/project sdp/ticket_confirmation.html?ticket_id=%d';", ticket_id);
    printf("</script>");
    printf("</body></html>");

    return 0;
}

