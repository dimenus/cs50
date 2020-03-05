#include <stdio.h>
#include <stdint.h>
#include <cs50.h>

bool validate_checksum(uint8_t digits[], uint64_t len)
{
    uint64_t checksum = 0;
    for (int64_t i = len - 2; i >= 0; i -= 2) {
        uint8_t tmp = digits[i] * 2;
        if (tmp >= 10) {
            checksum += tmp / 10;
        }
        checksum += tmp % 10;
    }
    for (int64_t i = len - 1; i >= 0; i -= 2) {
        checksum += (uint64_t)(digits[i]);
    }
    return checksum % 10 == 0;
}

int main(void)
{

    uint64_t raw_value = LONG_MAX;
    do {
        raw_value = get_long("Number: ");
    } while (raw_value == LONG_MAX);
    uint8_t raw_digits[24] = { 0 };
    uint64_t len = 0;
    while (raw_value > 0) { 
        uint64_t rem = raw_value % 10;
        raw_digits[len] = (uint8_t)rem;
        raw_value /= 10;
        len += 1;
    }

    //swap the digits so we can just chop off the LSByte
    uint8_t digits[24] = { 0 };
    uint64_t offset = len - 1;
    for (uint64_t i = 0; i < len; i++) {
        digits[i] = raw_digits[offset];
        offset -= 1;
    }

    if (!validate_checksum(digits, len)) {
        printf("INVALID\n");
        return 0;
    }

    uint8_t first = digits[0];
    uint8_t second = digits[1];
    if (first == 3 && (second == 4 || second == 7) && len == 15) {
        printf("AMEX\n");
    } else if (first ==5 && (second >= 1 && second <= 5) && len == 16) {
        printf("MASTERCARD\n");
    } else if (first == 4 && (len == 13 || len == 16)) {
        printf("VISA\n");
    } else {
        printf("INVALID\n");
    }
    return 0;
}
