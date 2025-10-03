#pragma once
#include <string>

/**
 * @namespace libtslog
 * @brief Thread-safe logging library with timestamp
 * 
 * This library provides thread-safe logging functionalities,
 * including automatic timestamps and synchronized file access.
 */
namespace libtslog {
    
    /**
     * @brief Initializes the logging system
     * 
     * Sets up the logging system by defining the output file.
     * Must be called before any logging operations.
     * 
     * @param fileName Name of the file where logs will be saved
     * @throws std::runtime_error If unable to create/open the file
     * 
     * @example
     * @code
     * libtslog::init("application.log");
     * @endcode
     */
    void init(const std::string& fileName);
    
    /**
     * @brief Logs a message to the file
     * 
     * Writes a message to the log file with automatic timestamp.
     * The operation is thread-safe.
     * 
     * @param message Message to be logged
     * 
     * @pre init() must have been called previously
     * @post The message is written to the file with timestamp
     * 
     * @example
     * @code
     * libtslog::log("Server started on port 8080");
     * @endcode
     */
    void log(const std::string& message);
    
    /**
     * @brief Finalizes the logging system
     * 
     * Closes the log file and releases associated resources.
     * Should be called at the end of the application.
     * 
     * @post The log file is closed and resources are freed
     * 
     * @example
     * @code
     * libtslog::close();
     * @endcode
     */
    void close();
}