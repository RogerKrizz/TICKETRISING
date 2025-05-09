#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 1024

void urldecode(char *src, char *dest) {
    char a, b;
    while (*src) {
        if (*src == '%' && isxdigit(src[1]) && isxdigit(src[2])) {
            a = src[1];
            b = src[2];

            a = (a >= 'A') ? ((a & ~0x20) - 'A' + 10) : (a - '0');
            b = (b >= 'A') ? ((b & ~0x20) - 'A' + 10) : (b - '0');

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

// Function to parse and decode form data
void parseFormData(char *data, char *role, char *username, char *password,
                   char *fullName, char *dob, char *email, char *digitalId) {
    char *token = strtok(data, "&");
    while (token != NULL) {
        char key[MAX_LEN], value_encoded[MAX_LEN], value_decoded[MAX_LEN];
        sscanf(token, "%[^=]=%s", key, value_encoded);
        urldecode(value_encoded, value_decoded);

        if (strcmp(key, "role") == 0)
            strncpy(role, value_decoded, 49);
        else if (strcmp(key, "username") == 0)
            strncpy(username, value_decoded, 49);
        else if (strcmp(key, "password") == 0)
            strncpy(password, value_decoded, 49);
        else if (strcmp(key, "fullName") == 0)
            strncpy(fullName, value_decoded, 99);
        else if (strcmp(key, "dateOfBirth") == 0)
            strncpy(dob, value_decoded, 19);
        else if (strcmp(key, "email") == 0)
            strncpy(email, value_decoded, 99);
        else if (strcmp(key, "digitalId") == 0)
            strncpy(digitalId, value_decoded, 49);

        token = strtok(NULL, "&");
    }
}

int main(void) {
    char *lenStr = getenv("CONTENT_LENGTH");
    int len = (lenStr != NULL) ? atoi(lenStr) : 0;

    char postData[MAX_LEN] = {0};
    if (len > 0 && len < MAX_LEN) {
        fread(postData, 1, len, stdin);
    }

    // Output content type header first
    printf("Content-type: text/html\n\n");

    // Variables for form fields
    char role[50] = "", username[50] = "", password[50] = "";
    char fullName[100] = "", dob[20] = "", email[100] = "", digitalId[50] = "";

    // Parse and decode the form data
    parseFormData(postData, role, username, password, fullName, dob, email, digitalId);

    // Save to file
    FILE *fp = fopen("users.txt", "a");
    if (fp == NULL) {
        printf("<html><body><h2>Error: Could not open users.txt</h2></body></html>");
        return 1;
    }

    // Store data in correct order
    fprintf(fp, "%s %s %s %s %s %s %s \n", role, username, password, fullName, dob, email, digitalId);
    fclose(fp);

    // Success response
    printf("<html><body>");
    printf("<script>");
    printf("alert('Account created successfully! Redirecting...');\n");

    // Redirect based on role
    if (strcmp(role, "employee") == 0)
        printf("window.location.href = '/project sdp/employee dashboard.html';");
    else
        printf("window.location.href = '/project sdp/ticket submission page.html';");

    printf("</script>");
    printf("</body></html>");

    return 0;
}

