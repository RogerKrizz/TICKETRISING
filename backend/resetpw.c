#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void url_decode(char *src, char *dest) {
    char *pstr = src, *p = dest;
    while (*pstr) {
        if (*pstr == '%') {
            char hex[3] = {pstr[1], pstr[2], '\0'};
            *p = (char) strtol(hex, NULL, 16);
            pstr += 3;
        } else if (*pstr == '+') {
            *p = ' ';
            pstr++;
        } else {
            *p = *pstr++;
        }
        p++;
    }
    *p = '\0';
}

int reset_password(const char *identifier, const char *new_password) {
    FILE *fp = fopen("users.txt", "r");
    if (!fp) {
        printf("<h2>Error: Cannot open users.txt for reading.</h2>");
        return 0;
    }

    FILE *temp_fp = fopen("temp_users.txt", "w");
    if (!temp_fp) {
        printf("<h2>Error: Cannot create temporary file for writing.</h2>");
        fclose(fp);
        return 0;
    }

    char line[512];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        char role[50], username[50], password[50], name[100], date[50], email[100], id[50];

        if (sscanf(line, "%s %s %s %s %s %s %s", role, username, password, name, date, email, id) == 7) {
            if (strcmp(username, identifier) == 0 || strcmp(email, identifier) == 0) {
                fprintf(temp_fp, "%s %s %s %s %s %s %s\n", role, username, new_password, name, date, email, id);
                found = 1;
            } else {
                fputs(line, temp_fp);
            }
        } else {
            fputs(line, temp_fp);  // preserve malformed lines
        }
    }

    fclose(fp);
    fclose(temp_fp);

    if (found) {
        remove("users.txt");
        rename("temp_users.txt", "users.txt");
        return 1;
    } else {
        remove("temp_users.txt");
        return 0;
    }
}

int main() {
    printf("Content-Type: text/html\r\n\r\n");

    char *lenstr = getenv("CONTENT_LENGTH");
    int len = lenstr ? atoi(lenstr) : 0;

    if (len <= 0) {
        printf("<h2>No data received.</h2>");
        return 1;
    }

    char input[512] = {0};
    fread(input, 1, len, stdin);
    input[len] = '\0';

    char raw_identifier[100], raw_new_password[100], raw_confirm_password[100];
    char identifier[100], new_password[100], confirm_password[100];
    char *start = strstr(input, "identifier=");
    char *end = strstr(input, "&new_password=");

    if (start && end) {
        start += strlen("identifier=");
        int length = end - start;
        strncpy(raw_identifier, start, length);
        raw_identifier[length] = '\0';

        start = end + strlen("&new_password=");
        end = strstr(start, "&confirm_password=");
        length = end - start;
        strncpy(raw_new_password, start, length);
        raw_new_password[length] = '\0';

        start = end + strlen("&confirm_password=");
        strcpy(raw_confirm_password, start);

        url_decode(raw_identifier, identifier);
        url_decode(raw_new_password, new_password);
        url_decode(raw_confirm_password, confirm_password);

        if (strcmp(new_password, confirm_password) != 0) {
            printf("<h2>Passwords do not match. Please try again.</h2>");
            return 0;
        }

        if (reset_password(identifier, new_password)) {
            printf("<h2>Password reset successfully!</h2>");
        } else {
            printf("<h2>Failed to reset password: User not found.</h2>");
        }
    } else {
        printf("<h2>Invalid data received.</h2>");
    }

    return 0;
}

