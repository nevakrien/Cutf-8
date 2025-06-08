#include <stdio.h>
#include <string.h>
#include "cutf8.h"

void test_valid_utf8() {
    const char *text = "😀 汉字 🏳️‍⚧️"; /* emoji + Chinese chars + trans flag emoji */
    const char *p = text;
    char buf[4];
    size_t len=0;

    printf("== test_valid_utf8 ==\n");

    while (p && *p) {
        size_t cp_len = cutf8_copy(buf,p);
        if (cp_len == 0) {
            printf("Error copying valid UTF-8!\n");
            return;
        }

        /* echo to stdout */
        len=0;
        if (cutf8_put(stdout, buf, &len) != 0) {
            printf("\nError writing valid codepoint\n");
            return;
        }

        p = cutf8_skip(p);
        fflush(stdout);
    }

    printf("\n");
}

void test_get_put_file() {
    const char *filename = ".temp_utf8_test.txt";
    FILE *f;
    const char *text = "🏳️‍⚧️ 汉字";
    const char *p = text;
    char buf[4];
    size_t len;
    int i;
    char expected[12]; /* should be enough for 3 UTF-8 codepoints */
    char actual[12];
    size_t offset = 0;

    printf("== test_get_put_file ==\n");

    /* Step 1: Copy first 3 codepoints to file */
    f = fopen(filename, "wb");
    if (!f) {
        printf("Failed to open %s for write\n", filename);
        return;
    }

    for (i = 0; i < 3 && p && *p; ++i) {
        size_t copied = cutf8_copy(buf,p);
        memcpy(expected + offset, buf, copied);
        offset += copied;

        len=0;
        if (cutf8_put(f, buf, &len) != 0) {
            printf("Error writing to file.\n");
            fclose(f);
            return;
        }

        p = cutf8_skip(p);
    }

    fclose(f);

    /* Step 2: Read back the file and compare byte-by-byte */
    f = fopen(filename, "rb");
    if (!f) {
        printf("Failed to reopen %s for read\n", filename);
        return;
    }

    fread(actual, 1, offset, f);
    fclose(f);

    /* Step 3: Compare actual to expected */
    if (memcmp(expected, actual, offset) == 0) {
        printf("PASS: Output matches expected first 3 codepoints.\n");
    } else {
        printf("FAIL: Output does not match expected UTF-8 bytes.\n");
        for (i = 0; i < offset; ++i)
            printf("expected[%d] = %02X, actual[%d] = %02X\n", i, (unsigned char)expected[i], i, (unsigned char)actual[i]);
    }

    /* Step 4: Cleanup */
    remove(filename);
}

FILE* fmemopen(void* p, size_t size, const char* mode);

void test_malformed_sequences() {
    const char *bad[] = {
        "\xF0\x28\x8C\x28", /* invalid 4-byte */
        "\xC0",             /* truncated 2-byte */
        "\xE0\x80",         /* truncated 3-byte */
        "\xED\xA0\x80",     /* UTF-16 surrogate half */
        "\x80",             /* stray continuation byte */
        0
    };

    char tmp[4];
    size_t len;
    int i = 0;

    printf("== test_malformed_sequences ==\n");

    while (bad[i]) {
        const char *p = bad[i];
        printf("Test %d:\n", i);

        if (cutf8_skip(p) == 0) {
            printf("  skip: correctly rejected\n");
        } else {
            printf("  skip: FAILED to reject \"");
            fwrite(p,cutf8_skip(p)-p,1,stdout);
            puts("\"");
        }

        FILE *mem = fmemopen((void *)p, strlen(p), "rb");
        if (mem) {
            len=0;
            if (cutf8_get(mem, tmp, &len) != 0) {
                printf("  get : correctly rejected\n");
            } else {
                printf("  get : FAILED to reject\n");
            }
            fclose(mem);
        }

        ++i;
    }
}

int main() {
    test_valid_utf8();
    test_get_put_file();
    test_malformed_sequences();
    return 0;
}
