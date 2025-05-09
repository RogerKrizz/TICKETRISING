#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

void get_field_value(char *data, const char *fieldname, char *result, int maxlen) {
    char *start = strstr(data, fieldname);
    if (start) {
        start += strlen(fieldname) + 1;
        char *end = strchr(start, '&');
        int len = end ? end - start : strlen(start);
        if (len >= maxlen) len = maxlen - 1;
        strncpy(result, start, len);
        result[len] = '\0';
    } else {
        result[0] = '\0';
    }
}

int check_username_password(const char *username, const char *password) {
    FILE *fp = fopen("users.txt", "r");
    if (!fp) return 0;

    char file_role[50], file_username[50], file_password[50];
    char dummy1[100], dummy2[20], dummy3[100], dummy4[50];

    while (fscanf(fp, "%49s %49s %49s %99s %19s %99s %49s",
                  file_role, file_username, file_password,
                  dummy1, dummy2, dummy3, dummy4) == 7) {
        if (strcmp(file_username, username) == 0 &&
            strcmp(file_password, password) == 0) {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

int main() {
    printf("Content-Type: text/html\r\n\r\n");

    char *lenstr = getenv("CONTENT_LENGTH");
    int len = lenstr ? atoi(lenstr) : 0;

    if (len <= 0) {
        printf("<h3>No data received.</h3>");
        return 1;
    }

    char input[500] = {0};
    fread(input, 1, len, stdin);
    input[len] = '\0';

    char raw_username[50], raw_password[50];
    get_field_value(input, "username", raw_username, sizeof(raw_username));
    get_field_value(input, "password", raw_password, sizeof(raw_password));

    char username[50], password[50];
    urldecode(raw_username, username);
    urldecode(raw_password, password);

    if (check_username_password(username, password)) {
        // Redirect to next page
        printf("<html><body>");
        printf("<h2>Login Successful</h2>");
        printf("<script>window.location.href = '/project sdp/ticket submission page.html';</script>");
        printf("</body></html>");
    } else {
        printf("<html><body>");
        printf("<h2>Login Failed</h2>");
        printf("<p>Invalid username or password.</p>");
        printf("</body></html>");
    }

    return 0;
}

