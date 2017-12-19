#ifndef OUT_UTILS_H
#define OUT_UTILS_H

#include <string>
#include <sstream>

#define BOLD        "\e[1m"
#define OFF         "\e[0m"

#define COLOR(id)   "\033[1;3" << id << "m"
#define UNUSED_VAR  (void)

#define ERROR_PRINT COLOR(color::red) << "Error: " << OFF
#define ERROR_SPACE "       "

enum color {
    red = 1,  
    green = 2, 
    yellow = 3,
    blue = 4,
    magenta = 5,
    cyan = 6,
};

#endif

