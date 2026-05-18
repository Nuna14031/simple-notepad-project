#include "notepad_exception.h"

#include <iostream>
#include <stdexcept>

void test_single_catch()
{
    std::cout << "Test 1: Single catch\n";
    try {
        throw file_not_found_exception("missing.txt");
    } catch (const notepad_exception& ex) {
        std::cout << "Caught notepad_exception: " << ex.what() << "\n";
    }
    std::cout << "\n";
}

void test_multiple_catches()
{
    std::cout << "Test 2: Multiple catches\n";

    // Sub-test 2a: throws file_not_found_exception → caught by most specific handler
    try {
        throw file_not_found_exception("missing.txt");
    } catch (const file_not_found_exception& ex) {
        std::cout << "Test 2a: Caught file_not_found_exception: " << ex.what() << "\n";
    } catch (const notepad_exception& ex) {
        std::cout << "Test 2a: Caught notepad_exception: " << ex.what() << "\n";
    } catch (const std::exception& ex) {
        std::cout << "Test 2a: Caught std::exception: " << ex.what() << "\n";
    }

    // Sub-test 2b: throws file_read_exception → caught by notepad_exception handler
    try {
        throw file_read_exception("corrupt.dat");
    } catch (const file_not_found_exception& ex) {
        std::cout << "Test 2b: Caught file_not_found_exception: " << ex.what() << "\n";
    } catch (const notepad_exception& ex) {
        std::cout << "Test 2b: Caught notepad_exception: " << ex.what() << "\n";
    } catch (const std::exception& ex) {
        std::cout << "Test 2b: Caught std::exception: " << ex.what() << "\n";
    }

    // Sub-test 2c: throws std::runtime_error → caught by std::exception handler
    try {
        throw std::runtime_error("Unknown error");
    } catch (const file_not_found_exception& ex) {
        std::cout << "Test 2c: Caught file_not_found_exception: " << ex.what() << "\n";
    } catch (const notepad_exception& ex) {
        std::cout << "Test 2c: Caught notepad_exception: " << ex.what() << "\n";
    } catch (const std::exception& ex) {
        std::cout << "Test 2c: Caught std::exception: " << ex.what() << "\n";
    }

    std::cout << "\n";
}

void open_file_inner(const std::string& filename)
{
    try {
        throw file_not_found_exception(filename);
    } catch (const notepad_exception& ex) {
        std::cout << "Inner catch in open_file: " << ex.what() << "\n";
        std::cout << "Rethrowing...\n";
        throw;
    }
}

void test_rethrow()
{
    std::cout << "Test 3: Rethrow\n";
    try {
        open_file_inner("missing.txt");
    } catch (const notepad_exception& ex) {
        std::cout << "Outer catch in main: " << ex.what() << "\n";
    }
    std::cout << "\n";
}

int main()
{
    test_single_catch();
    test_multiple_catches();
    test_rethrow();

    return 0;
}