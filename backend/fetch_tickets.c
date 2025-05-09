#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256
#define QUEUE_SIZE 100  // Maximum number of tickets in the queue

// Ticket structure definition
typedef struct {
    char id[20];
    char service[50];
    char pages[10];
    char copies[10];
    char username[100];
    char notes[200];
    char date[20];
    char time[20];
    char status[20];
} Ticket;

// Queue structure definition
typedef struct {
    Ticket tickets[QUEUE_SIZE];
    int front;
    int rear;
} TicketQueue;

// Initialize the queue
void init_queue(TicketQueue* queue) {
    queue->front = 0;
    queue->rear = 0;
}

// Check if the queue is empty
int is_queue_empty(TicketQueue* queue) {
    return queue->front == queue->rear;
}

// Check if the queue is full
int is_queue_full(TicketQueue* queue) {
    return (queue->rear + 1) % QUEUE_SIZE == queue->front;
}

// Add ticket to the queue
void enqueue(TicketQueue* queue, Ticket ticket) {
    if (is_queue_full(queue)) {
        printf("Queue is full! Cannot add more tickets.\n");
        return;
    }
    queue->tickets[queue->rear] = ticket;
    queue->rear = (queue->rear + 1) % QUEUE_SIZE;
}

// Remove ticket from the queue
Ticket dequeue(TicketQueue* queue) {
    if (is_queue_empty(queue)) {
        printf("Queue is empty! No tickets to process.\n");
        Ticket empty_ticket = {0};
        return empty_ticket;
    }
    Ticket ticket = queue->tickets[queue->front];
    queue->front = (queue->front + 1) % QUEUE_SIZE;
    return ticket;
}

// Print ticket data in plain text format
void print_ticket_data(Ticket t) {
    printf("Ticket ID: %s\n", t.id);
    printf("Username: %s\n", t.username);
    printf("Service: %s\n", t.service);
    printf("Pages: %s\n", t.pages);
    printf("Copies: %s\n", t.copies);
    printf("Notes: %s\n", t.notes);
    printf("Submitted Date: %s\n", t.date);
    printf("Submitted Time: %s\n", t.time);
    printf("Status: %s\n", t.status);
    printf("----------------------------\n");
}

// Save a ticket to the file
void add_ticket(Ticket new_ticket) {
    FILE *fp = fopen("tickets.txt", "a");
    if (!fp) {
        printf("Error opening tickets.txt for writing.\n");
        return;
    }

    fprintf(fp, "Ticket ID: %s\n", new_ticket.id);
    fprintf(fp, "Service: %s\n", new_ticket.service);
    fprintf(fp, "Pages: %s\n", new_ticket.pages);
    fprintf(fp, "Copies: %s\n", new_ticket.copies);
    fprintf(fp, "Username: %s\n", new_ticket.username);
    fprintf(fp, "Notes: %s\n", new_ticket.notes);
    fprintf(fp, "Submitted Date: %s\n", new_ticket.date);
    fprintf(fp, "Submitted Time: %s\n", new_ticket.time);
    fprintf(fp, "Status: %s\n", new_ticket.status);
    fprintf(fp, "----------------------------\n");

    fclose(fp);
}

int main() {
    char *request_method = getenv("REQUEST_METHOD");
    char *content_length = getenv("CONTENT_LENGTH");

    printf("Content-type: text/plain\n\n");  // IMPORTANT: plain text output

    // Initialize the queue
    TicketQueue queue;
    init_queue(&queue);

    // Handle POST (add new ticket)
    if (request_method && strcmp(request_method, "POST") == 0) {
        int length = atoi(content_length);
        char *data = malloc(length + 1);
        fread(data, 1, length, stdin);
        data[length] = '\0';

        Ticket new_ticket = {0};
        sscanf(data, "id=%19[^&]&service=%49[^&]&pages=%9[^&]&copies=%9[^&]&username=%99[^&]&notes=%199[^&]&date=%19[^&]&time=%19[^&]&status=%19[^&]",
               new_ticket.id, new_ticket.service, new_ticket.pages, new_ticket.copies,
               new_ticket.username, new_ticket.notes, new_ticket.date, new_ticket.time, new_ticket.status);

        free(data);
        add_ticket(new_ticket);
        enqueue(&queue, new_ticket);  // Add the ticket to the queue
    }

    // Read tickets.txt and enqueue them into the queue
    FILE *fp = fopen("tickets.txt", "r");
    if (!fp) {
        printf("No tickets found.\n");
        return 1;
    }

    char line[MAX_LINE];
    Ticket t = {0};
    strcpy(t.status, "Pending");

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "Ticket ID:", 10) == 0)
            sscanf(line, "Ticket ID: %[^\n]", t.id);
        else if (strncmp(line, "Service:", 8) == 0)
            sscanf(line, "Service: %[^\n]", t.service);
        else if (strncmp(line, "Pages:", 6) == 0)
            sscanf(line, "Pages: %[^\n]", t.pages);
        else if (strncmp(line, "Copies:", 7) == 0)
            sscanf(line, "Copies: %[^\n]", t.copies);
        else if (strncmp(line, "Username:", 9) == 0)
            sscanf(line, "Username: %[^\n]", t.username);
        else if (strncmp(line, "Notes:", 6) == 0)
            sscanf(line, "Notes: %[^\n]", t.notes);
        else if (strncmp(line, "Submitted Date:", 15) == 0)
            sscanf(line, "Submitted Date: %[^\n]", t.date);
        else if (strncmp(line, "Submitted Time:", 15) == 0)
            sscanf(line, "Submitted Time: %[^\n]", t.time);
        else if (strncmp(line, "Status:", 7) == 0)
            sscanf(line, "Status: %[^\n]", t.status);
        else if (strncmp(line, "----------------------------", 28) == 0) {
            enqueue(&queue, t);  // Add to the queue instead of printing directly
            memset(&t, 0, sizeof(Ticket));
            strcpy(t.status, "Pending");
        }
    }

    fclose(fp);

    // Process and display tickets from the queue
    while (!is_queue_empty(&queue)) {
        Ticket ticket = dequeue(&queue);  // Get a ticket from the queue
        print_ticket_data(ticket);  // Print ticket data
    }

    return 0;
}

