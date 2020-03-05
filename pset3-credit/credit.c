#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <cs50.h>
#include <sys/types.h>

int main(void)
{
    uint64_t raw_value = get_long("Number: ");
    uint8_t raw_digits[32] = { 0 };
    size_t len = 0;
    while (raw_value > 0) { 
        uint64_t rem = raw_value % 10;
        assert(rem < 0xFF);
        raw_digits[len] = (uint8_t)rem;
        raw_value /= 10;
        len += 1;
    }

    uint8_t digits[32] = { 0 };
    size_t offset = len - 1;
    for (size_t i = 0; i < len; i++) {
        digits[i] = raw_digits[offset];
        offset -= 1;
    }
    uint64_t checksum = 0;
    for (ssize_t i = len - 2; i >= 0; i -= 2) {
        uint64_t tmp = digits[i] * 2;
        if (tmp >= 10) {
            checksum += tmp / 10;
        }
        checksum += tmp % 10;
    }
    for (ssize_t i = len -1; i >= 0; i -= 2) {
        checksum += (uint64_t)(digits[i]);
    }
    if (checksum % 10 != 0) {
        printf("INVALID\n");
        return 0;
    }

    uint8_t first = digits[0];
    uint8_t second = digits[1];
    if (first == 3 && (second == 4 || second == 7)) {
        if (len != 15) {
            printf("INVALID\n");
        } else {
            printf("AMEX\n");
        }
    } else if (first ==5 && (second == 1 || second == 2 || second == 3 || second == 4 || second == 5)) {
        if (len != 16) {
            printf("INVALID\n");
        } else {
            printf("MASTERCARD\n");
        }
    } else if (first == 4) {
        if (len != 13 && len != 16) {
            printf("INVALID\n");
        } else {
            printf("VISA\n");
        }
    } else {
        printf("INVALID\n");
    }
    return 0;
}
