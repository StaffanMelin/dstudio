#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>

int main()
{
    #define ROWS 8
    #define COLS 3
    int pin_r[ROWS] = {13, 12, 16, 17, 20, 22, 23, 24};
    int pin_c[COLS] = {25, 26, 27};

    const char *chipname = "gpiochip0";
    struct gpiod_chip *chip;
    struct gpiod_line *line_r[ROWS];
    struct gpiod_line *line_c[COLS];
    int val[ROWS][COLS];

    // Open GPIO chip
    chip = gpiod_chip_open_by_name(chipname);

    // Open GPIO lines
    for (int i = 0; i < ROWS; i++) {
        line_r[i] = gpiod_chip_get_line(chip, pin_r[i]);
        gpiod_line_request_output(line_r[i], "example1", 1); // HI default
    }
    for (int i = 0; i < COLS; i++) {
        line_c[i] = gpiod_chip_get_line(chip, pin_c[i]);
        gpiod_line_request_input(line_c[i], "example1");
    }

    while (true)
    {
        for (int r = 0; r < ROWS; r++) {
            gpiod_line_set_value(line_r[r], 0);
            usleep(10); // let row pin settle
            for (int c = 0; c < COLS; c++) {
                val[r][c] = gpiod_line_get_value(line_c[c]);
            }
            gpiod_line_set_value(line_r[r], 1);
            usleep(10);
        }

        std::cout << "\n";
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                std::cout << val[r][c] << " ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
        usleep(100000);
    }

    // Release lines and chip
    for (int r = 0; r < ROWS; r++) {
        gpiod_line_release(line_r[r]);
    }
    for (int c = 0; c < COLS; c++) {
        gpiod_line_release(line_c[c]);
    }
    gpiod_chip_close(chip);
    return 0;
}
