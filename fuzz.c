#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cutf8.h"

#define FUZZ_ROUNDS 1000000
#define MAX_SEQ_LEN 17

int main(void) {
    FILE *pipe = popen("python3 check_utf8.py", "w");
    if (!pipe) {
        perror("popen");
        return 1;
    }

    srand((unsigned int)time(NULL));

    for (int round = 0; round < FUZZ_ROUNDS; ++round) {
        unsigned char buf[MAX_SEQ_LEN];
        int len = rand() % MAX_SEQ_LEN;

        for (int i = 0; i < len; ++i)
            buf[i] = (unsigned char)(rand() % 256);

        int valid = cutf8_valid_buff((const char *)buf, len);

        fprintf(pipe, "%d ", valid);
        for (int i = 0; i < len; ++i)
            fprintf(pipe, "%02X", buf[i]);
        fprintf(pipe, "\n");
    }

    pclose(pipe);
    return 0;
}
