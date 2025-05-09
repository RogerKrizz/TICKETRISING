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
    *dest++ = '\0';
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

int check_credentials_from_file(const char *role, const char *username, const char *password) {
    FILE *fp = fopen("users.txt", "r");
    if (!fp) {
        printf("<h2>Cannot open users.txt for reading.</h2>");
        return 0;
    }

    char file_role[50], file_username[50], file_password[50];
    char file_fullName[100], file_dob[20], file_email[100], file_digitalId[50];

    while (fscanf(fp, "%49s %49s %49s %99s %19s %99s %49s",
                  file_role, file_username, file_password,
                  file_fullName, file_dob, file_email, file_digitalId) == 7) {

        // Strip newline characters just in case
        file_role[strcspn(file_role, "\r\n")] = '\0';
        file_username[strcspn(file_username, "\r\n")] = '\0';
        file_password[strcspn(file_password, "\r\n")] = '\0';

        // Debug print
        printf("<p>Checking against file entry: [%s] [%s] [%s]</p>", file_role, file_username, file_password);

        if (strcmp(file_role, role) == 0 &&
            strcmp(file_username, username) == 0 &&
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
        printf("<h2>No data received.</h2>");
        return 1;
    }

   
    char input[500] = {0};
    fread(input, 1, len, stdin);
    input[len] = '\0';

   
    char raw_role[50], raw_username[50], raw_password[50];
    get_field_value(input, "role", raw_role, sizeof(raw_role));
    get_field_value(input, "username", raw_username, sizeof(raw_username));
    get_field_value(input, "password", raw_password, sizeof(raw_password));

   
    char role[50], username[50], password[50];
    urldecode(raw_role, role);
    urldecode(raw_username, username);
    urldecode(raw_password, password);

    
    if (check_credentials_from_file(role, username, password)) {
        
        if (strcmp(role, "employee") == 0) {
            
            printf("<body><script>window.location.href = '/project sdp/employee dashboard.html';</script></body>");
            printf("</html>");
        } else if (strcmp(role, "customer") == 0) {
            
            printf("<html>");
            printf("<body><script>window.location.href = '/project sdp/ticket submission page.html';</script></body>");
            printf("</html>");
        } else {
            
            printf("<h2>Login Failed: Unknown Role</h2>");
        }
    } else {
        printf("<p>Decoded Input: role=%s, username=%s, password=%s</p>", role, username, password);
        printf("<h2>Login Failed</h2>");
    }

    return 0;
}
