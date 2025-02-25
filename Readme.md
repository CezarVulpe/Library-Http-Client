# Virtual Library Client

This program is a client-side application for interacting with a virtual library server using HTTP requests. It allows users to register, log in, access the library, manage books, and log out. The client communicates with the server using JSON payloads and handles authentication via session cookies and JWT tokens.

## Features

- **User Authentication:** Users can register and log in. Successful login retrieves a session cookie for authentication.
- **Library Access:** Users can request access to the library, receiving a JWT token for further interactions.
- **Book Management:** Users can retrieve a list of books, access details of a specific book, add new books, and delete books from the library.
- **Session Management:** The program handles session cookies and authentication tokens, ensuring secure user sessions.

## Implementation Details

- **HTTP Communication:** The client sends HTTP requests to interact with the server, using GET, POST, and DELETE methods.
- **Session Handling:** Upon login, a session cookie is stored and used for subsequent authenticated requests. JWT tokens are used for library access.
- **JSON Parsing:** The Parson library is used to parse JSON responses from the server.
- **Memory Management:** Dynamic allocation is used for handling request payloads and server responses.
- **Error Handling:** The program checks server responses for errors, displaying appropriate messages to the user.

## Core Functions

- `handle_register()`: Registers a new user by sending a JSON payload with the username and password to the server.
- `handle_login()`: Authenticates a user and stores the session cookie for further requests.
- `handle_enter_library()`: Requests access to the library, retrieving a JWT token.
- `handle_get_books()`: Fetches and displays the list of available books in the library.
- `handle_get_book()`: Retrieves details about a specific book by its ID.
- `handle_add_book()`: Collects book details from the user and sends a request to add a new book.
- `handle_delete_book()`: Deletes a book by sending a DELETE request with the book ID.
- `handle_logout()`: Ends the user session by invalidating the session cookie and authentication token.

## Dependencies

- **Parson JSON Library:** Used for parsing JSON responses from the server.
- **Helper Functions:** `helpers.c` and `requests.c` from the official laboratory provide utility functions for managing HTTP communication.

This implementation ensures a structured approach to user authentication and book management, enabling smooth interaction with the virtual library server.

