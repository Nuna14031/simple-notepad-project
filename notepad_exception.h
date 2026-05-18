#ifndef NOTEPAD_EXCEPTION_H
#define NOTEPAD_EXCEPTION_H

#include <stdexcept>
#include <string>

class notepad_exception : public std::runtime_error {
public:
    explicit notepad_exception(const std::string& message)
        : std::runtime_error(message)
    {
    }
};

class file_not_found_exception : public notepad_exception {
public:
    explicit file_not_found_exception(const std::string& filename)
        : notepad_exception("File not found: '" + filename + "'")
    {
    }
};

class file_read_exception : public notepad_exception {
public:
    explicit file_read_exception(const std::string& filename)
        : notepad_exception("Failed to read file: '" + filename + "'")
    {
    }
};

class file_write_exception : public notepad_exception {
public:
    explicit file_write_exception(const std::string& filename)
        : notepad_exception("Failed to write file: '" + filename + "'")
    {
    }
};

#endif // NOTEPAD_EXCEPTION_H
