#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "requests.h"
#include "helpers.h"
#include "./parson/parson.h"
#include "./parson/parson.c"

void handle_register(char *host, int port)
{
    char username[50];
    char password[50];
    char *response;
    char *json_payload;
    int sockfd;

    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);

    json_payload = (char *)malloc(200 * sizeof(char));
    sprintf(json_payload, "{\"username\":\"%s\",\"password\":\"%s\"}", username, password);

    sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Nu s-a putut conecta\n");
        free(json_payload);
        return;
    }

    response = compute_post_request(host, "/api/v1/tema/auth/register", "application/json", &json_payload, 1, NULL, 0);
    send_to_server(sockfd, response);
    char *server_response = receive_from_server(sockfd);
    close_connection(sockfd);

    if (strstr(server_response, "400 Bad Request") || strstr(server_response, "\"error\""))
    {
        printf("ERROR Eroare la înregistrare: %s\n", strstr(server_response, "{"));
    }
    else if (strstr(server_response, "201 Created") || strstr(server_response, "200 OK"))
    {
        printf("SUCCESS Utilizator înregistrat cu SUCCES !\n");
    }
    else
    {
        printf("Răspuns neașteptat de la server.\n");
    }

    free(json_payload);
    free(response);
    free(server_response);
}

void handle_login(char *host, int port, char **session_cookie)
{
    char username[50];
    char password[50];
    char *response;
    char *json_payload;
    int sockfd;

    printf("username=");
    scanf("%s", username);
    username[strcspn(username, "\n")] = 0;
    printf("password=");
    scanf("%s", password);
    password[strcspn(password, "\n")] = 0;
    getchar();

    json_payload = (char *)malloc(200 * sizeof(char));
    sprintf(json_payload, "{\"username\":\"%s\",\"password\":\"%s\"}", username, password);

    sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);
    response = compute_post_request(host, "/api/v1/tema/auth/login", "application/json", &json_payload, 1, NULL, 0);
    send_to_server(sockfd, response);
    char *server_response = receive_from_server(sockfd);
    close_connection(sockfd);

    // Print server response for debugging
    // Check response for HTTP status code and error message
    if (strstr(server_response, "400 Bad Request") || strstr(server_response, "\"error\""))
    {
        printf("ERROR Eroare la login: %s\n", strstr(server_response, "{"));
        if (*session_cookie)
        {
            free(*session_cookie);
            *session_cookie = NULL;
        }
    }
    else
    {
        printf("SUCCESS Utilizator logat cu SUCCES!\n");
        // Parse and set the session cookie
        char *cookie_start = strstr(server_response, "Set-Cookie: ");
        if (cookie_start)
        {
            cookie_start += strlen("Set-Cookie: ");
            char *cookie_end = strstr(cookie_start, ";");
            if (cookie_end)
            {

                *session_cookie = strndup(cookie_start, cookie_end - cookie_start);

            }
        }
    }

    free(json_payload);
    free(response);
    free(server_response);
}

char *handle_enter_library(char *host, int port, char *session_cookie)
{
    char *token = NULL;

    // Prepare the HTTP request
    char message[BUFLEN];
    snprintf(message, sizeof(message),
             "GET /api/v1/tema/library/access HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Cookie: %s\r\n"
             "\r\n",
             host, session_cookie);

    // Open connection to the server
    int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Send the request
    send_to_server(sockfd, message);

    // Receive the response
    char *response = receive_from_server(sockfd);

    // Close the connection
    close_connection(sockfd);

    // Print the server response for debugging
    // Find the token in the response
    char *token_start = strstr(response, "\"token\":\"");
    if (token_start)
    {
        printf("SUCCESS Biblioteca a fost accesata!\n");
        token_start += strlen("\"token\":\""); // Move past the "token":" part
        char *token_end = strchr(token_start, '"');
        if (token_end)
        {
            token = malloc(token_end - token_start + 1);
            if (token)
            {
                strncpy(token, token_start, token_end - token_start);
                token[token_end - token_start] = '\0'; // Null-terminate the string
            }
        }
    }
    else
    {
        printf("Nu s-a gasit token\n");
    }

    free(response);
    return token;
}

void handle_get_books(char *host, int port, char *auth_token)
{
    char *response;
    int sockfd;
    char message[BUFLEN];
    snprintf(message, sizeof(message),
             "GET /api/v1/tema/library/books HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: Bearer %s\r\n"
             "\r\n",
             host, auth_token);

    // Open connection to the server
    sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Send the request
    send_to_server(sockfd, message);

    // Receive the response
    response = receive_from_server(sockfd);



    // Close the connection
    close_connection(sockfd);

    char *json_start = strstr(response, "\r\n\r\n");
    if (json_start)
    {
        json_start += 4; // Skip the "\r\n\r\n" to start of JSON data
        printf("%s\n", json_start);
    }
    else
    {
        printf("Nu s-a gasit json\n");
    }

    free(response);
}

void handle_get_book(char *host, int port, char *auth_token)
{
    int book_id;
    printf("id=");
    scanf("%d", &book_id);
    getchar();
    char *response;
    int sockfd;
    char message[BUFLEN];

    snprintf(message, sizeof(message),
             "GET /api/v1/tema/library/books/%d HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: Bearer %s\r\n"
             "\r\n",
             book_id, host, auth_token);

    // Open connection to the server
    sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Send the request
    send_to_server(sockfd, message);

    // Receive the response
    response = receive_from_server(sockfd);

    // Close the connection
    close_connection(sockfd);

    if (strstr(response, "ERROR") || strstr(response, "EROARE"))
    {
        printf("ERROR Eroare la accesarea cărții: %s\n", response);
    }
    else
    {
        // Get the last line from the response which contains the JSON data
        char *json_response;
        char *last_line = strrchr(response, '\n');
        if (last_line && *(last_line + 1))
        {
            json_response = last_line + 1; // Skip newline character
        }
        else
        {
            json_response = response; // Assume the whole response is the JSON
        }
        // Parse JSON response using Parson
        JSON_Value *root_value = json_parse_string(json_response);
        JSON_Object *root_object = json_value_get_object(root_value);

        if (root_value == NULL)
        {
            printf("ERROR Eroare la parsarea JSON\n");
        }
        else
        {
            // Print formatted JSON
            printf("{\n");
            printf("   \"id\": %d,\n", (int)json_object_get_number(root_object, "id"));
            printf("   \"title\": \"%s\",\n", json_object_get_string(root_object, "title"));
            printf("   \"author\": \"%s\",\n", json_object_get_string(root_object, "author"));
            printf("   \"publisher\": \"%s\",\n", json_object_get_string(root_object, "publisher"));
            printf("   \"genre\": \"%s\",\n", json_object_get_string(root_object, "genre"));
            printf("   \"page_count\": %d\n", (int)json_object_get_number(root_object, "page_count"));
            printf("}\n");
        }
        json_value_free(root_value);
    }

    free(response);
}

void flush_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

void clear_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';  // Înlocuiește newline-ul cu un caracter null
    }
}

void handle_add_book(char *host, int port, char *auth_token)
{
    char title[1000], author[1000], genre[1000], publisher[1000];
    int page_count;
    char *response;
    int sockfd;
    char auxatoi[1000];
    char *json_payload; // Use dynamic memory allocation for JSON payload
    char message[2048]; // Increased buffer size to handle larger payloads

    // Input gathering
    flush_input_buffer();
    printf("title=");
    fgets(title, sizeof(title), stdin);
    clear_newline(title);  // Șterge newline-ul de la sfârșitul stringului

    printf("author=");
    fgets(author, sizeof(author), stdin);
    clear_newline(author);

    printf("genre=");
    fgets(genre, sizeof(genre), stdin);
    clear_newline(genre);

    printf("publisher=");
    fgets(publisher, sizeof(publisher), stdin);
    clear_newline(publisher);

    printf("page_count=");
    fgets(auxatoi, sizeof(auxatoi), stdin);
    clear_newline(auxatoi);  // Pregătește stringul pentru conversie
    page_count = atoi(auxatoi);  // Convertește stringul la întreg

    if(page_count == 0)
    {
        printf("Incorrect data type for page count\n");
        return;
    }

    // Allocate memory for JSON payload dynamically
    json_payload = (char *)malloc(1024 * sizeof(char));
    if (json_payload == NULL)
    {
        printf("Failed to allocate memory for JSON payload\n");
        return;
    }
    int json_length = snprintf(json_payload, 1024, "{\"title\":\"%s\",\"author\":\"%s\",\"genre\":\"%s\",\"page_count\":%d,\"publisher\":\"%s\"}",
                               title, author, genre, page_count, publisher);
    if (json_length >= 1024)
    {
        printf("JSON payload is too long and has been truncated\n");
        free(json_payload);
        return;
    }

    // Prepare the HTTP POST request
    snprintf(message, sizeof(message),
                                  "POST /api/v1/tema/library/books HTTP/1.1\r\n"
                                  "Host: %s\r\n"
                                  "Authorization: Bearer %s\r\n"
                                  "Content-Type: application/json\r\n"
                                  "Content-Length: %d\r\n"
                                  "\r\n"
                                  "%s",
                                  host, auth_token, json_length, json_payload);

    // Open connection to the server
    sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Send the request
    send_to_server(sockfd, message);

    // Receive the response
    response = receive_from_server(sockfd);

    // Close the connection
    close_connection(sockfd);

    if (strstr(response, "ERROR") || strstr(response, "EROARE"))
    {
        printf("ERROR Eroare la adaugarea cartii: %s\n", response);
    }
    else
    {
        printf("SUCCESS Cartea adaugata cu succes!\n");
    }

    free(json_payload);
    free(response);
}

void handle_delete_book(char *host, int port, char *auth_token)
{

    int book_id;
    printf("id=");
    scanf("%d", &book_id);
    getchar();
    char *response;
    int sockfd;
    char message[BUFLEN];

    snprintf(message, sizeof(message),
             "DELETE /api/v1/tema/library/books/%d HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: Bearer %s\r\n"
             "\r\n",
             book_id, host, auth_token);

    // Open connection to the server
    sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

    // Send the request
    send_to_server(sockfd, message);

    // Receive the response
    response = receive_from_server(sockfd);

    // Close the connection
    close_connection(sockfd);

    if (strstr(response, "ERROR") || strstr(response, "EROARE"))
    {
        printf("ERROR Eroare la ștergerea cărții: %s\n", response);
    }
    else
    {
        printf("SUCCESS Cartea cu id %d a fost ștearsă cu succes!\n", book_id);
    }

    free(response);
}

void handle_logout(char* host, int port, char **session_cookie, char **auth_token)
{
    if (*session_cookie)
    {
        char message[BUFLEN];
        snprintf(message, sizeof(message),
                 "GET /api/v1/tema/library/access HTTP/1.1\r\n"
                 "Host: %s\r\n"
                 "Cookie: %s\r\n"
                 "\r\n",
                 host, *session_cookie);

        // Open connection to the server
        int sockfd = open_connection(host, port, AF_INET, SOCK_STREAM, 0);

        // Send the request
        send_to_server(sockfd, message);

        // Receive the response
        char *response = receive_from_server(sockfd);

        // Close the connection
        close_connection(sockfd);

        // Print the server response for debugging
        free(*session_cookie);  // Free the memory allocated for the cookie if any
        *session_cookie = NULL; // Set the session cookie pointer to NULL
        *auth_token = NULL;
        printf("SUCCESS Utilizatorul s-a delogat cu succes\n");
    }
    else
    {
        printf("ERROR Niciun utilizator nu este logat.\n");
    }
}

int main()
{
    char host[] = "34.246.184.49";
    int port = 8080;
    char command[100];
    char *session_cookie = NULL; // Manage session state
    char *auth_token = NULL;

    while (1)
    {
        scanf("%s", command);

        if (strcmp(command, "exit") == 0)
        {
            break; // Exit without printing any message
        }
        else if (strcmp(command, "register") == 0)
        {
            handle_register(host, port);
        }
        else if (strcmp(command, "login") == 0)
        {
            handle_login(host, port, &session_cookie);
        }
        else if (strcmp(command, "enter_library") == 0)
        {
            if (session_cookie != NULL)
            {
                auth_token = handle_enter_library(host, port, session_cookie);
            }
            else
            {
                printf("ERROR : Utilizatorul nu este logat.\n");
            }
        }
        else if (strcmp(command, "get_books") == 0)
        {
            if (auth_token)
            {
                handle_get_books(host, port, auth_token);
            }
            else
            {
                printf("ERROR : Utilizatorul nu este conectat la o biblioteca.\n");
            }
        }
        else if (strcmp(command, "get_book") == 0)
        {
            if (auth_token)
            {
                handle_get_book(host, port, auth_token);
            }
            else
            {
                printf("ERROR : Utilizatorul nu este conectat la o biblioteca.\n");
            }
        }
        else if (strcmp(command, "add_book") == 0)
        {
            if (auth_token)
            {
                handle_add_book(host, port, auth_token);
            }
            else
            {
                printf("ERROR : Utilizatorul nu este conectat la o biblioteca.\n");
            }
        }
        else if (strcmp(command, "delete_book") == 0)
        {
            if (auth_token)
            {
                handle_delete_book(host, port, auth_token);
            }
            else
            {
                printf("ERROR : Utilizatorul nu este conectat la o biblioteca.\n");
            }
        }
        else if (strcmp(command, "logout") == 0)
        {
            if (session_cookie != NULL)
            {
            handle_logout(host, port, &session_cookie, &auth_token);
            }
            else
            {
                printf("ERROR : Utilizatorul nu este logat.\n");
            }
        }
    }
    return 0; // Program will terminate here after breaking out of the loop.
}
