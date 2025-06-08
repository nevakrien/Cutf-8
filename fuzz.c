#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cutf8.h"

#define FUZZ_ROUNDS 100000
#define MAX_SEQ_LEN 16

// Weighted random length: shorter lengths are more likely
static int random_len(void) {
    int r = rand() % 100;
    if (r < 40) return 3 + rand() % 4;       // 40% chance: 3–6 bytes
    if (r < 80) return 1 + rand() % 3;       // 40% chance: 1–3 bytes
    return 7 + rand() % (MAX_SEQ_LEN - 7 + 1); // 20% chance: 7–MAX junk
}


// Weighted byte: ASCII most common, valid UTF-8 range frequent, junk rare
static unsigned char random_utf8ish_byte(void) {
    int r = rand() % 100;
    if (r < 60) return (unsigned char)(rand() % 0x80);         // ASCII
    if (r < 90) return (unsigned char)(0x80 + rand() % 0x40);  // continuation
    return (unsigned char)(rand() % 256);                      // full junk
}

int main(void) {
    FILE *pipe = popen("python3 check_utf8.py", "w");
    if (!pipe) {
        perror("popen");
        return 1;
    }

    srand((unsigned int)time(NULL));

    for (int round = 0; round < FUZZ_ROUNDS; ++round) {
        unsigned char buf[MAX_SEQ_LEN];
        int len = random_len();
        int i;

        for (i = 0; i < len; ++i)
            buf[i] = random_utf8ish_byte();

        int valid = cutf8_valid_buff((const char *)buf, len);

        fprintf(pipe, "%d ", valid);
        for (i = 0; i < len; ++i)
            fprintf(pipe, "%02X", buf[i]);
        fprintf(pipe, "\n");
    }

    pclose(pipe);
    return 0;
}
