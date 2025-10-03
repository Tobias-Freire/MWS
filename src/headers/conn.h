/**
 * @brief Creates a TCP server socket on the specified port
 * 
 * This function creates a server socket using IPv4 and TCP protocol,
 * binding it to the specified port and configuring it to accept connections.
 * 
 * @param port Port number where the server will listen (1-65535)
 * @return Socket descriptor of the created socket, or -1 on error
 * 
 * @warning The function checks if the port is already in use
 * @note The returned socket must be closed with close() after use
 * 
 * @example
 * @code
 * int server = createSocketAndBind(8080);
 * if (server == -1) {
 *     // Error treatment
 * }
 * @endcode
 */
int createSocketAndBind(int port);

