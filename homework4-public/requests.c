#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params, char **headers, int headers_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Start the request line
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // Add the host header
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    // Add other headers (including cookies or authorization token)
    for (int i = 0; i < headers_count; i++) {
        if (headers[i] != NULL) {
            compute_message(message, headers[i]);
        }
    }

    // Finalize the headers
    compute_message(message, "");

    free(line);
    return message;
}


char *compute_post_request(char *host, char *url, char *content_type, char **body_data,
                           int body_data_fields_count, char **cookies, int cookies_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (content_type) {
        sprintf(line, "Content-Type: %s", content_type);
        compute_message(message, line);
    }

    // Append cookie if needed
    if (cookies != NULL && cookies_count > 0) {
        sprintf(line, "Cookie: %s", cookies[0]);
        compute_message(message, line);
    }

    if (body_data != NULL) {
        sprintf(line, "Content-Length: %ld", strlen(*body_data));
        compute_message(message, line);
    }

    compute_message(message, "");  // end of headers

    if (body_data != NULL) {
        compute_message(message, *body_data);  // Append JSON data
    }

    free(line);
    return message;
}

char *compute_delete_request(char *host, char *url, char **cookies, int cookies_count) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (cookies != NULL && cookies_count > 0) {
        sprintf(line, "Cookie: %s", cookies[0]);
        compute_message(message, line);
    }

    compute_message(message, ""); // End of headers

    free(line);
    return message;
}
