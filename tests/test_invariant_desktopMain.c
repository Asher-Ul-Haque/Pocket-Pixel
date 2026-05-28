#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

/* Simulate the vulnerable pattern from desktopMain.c:
 * The invariant is: romSize must be validated against a maximum bound
 * before malloc/memcpy operations. Buffer reads must never exceed declared length.
 */

#define MAX_ROM_SIZE (64 * 1024 * 1024)  /* 64 MB maximum ROM size */
#define MIN_ROM_SIZE 1

typedef unsigned char u8;

/* Safe ROM loader that enforces bounds - this is what the code SHOULD do */
typedef struct {
    u8 *data;
    size_t size;
    int error;  /* 0 = success, non-zero = rejected */
} RomBuffer;

static RomBuffer safe_load_rom(const u8 *input, size_t declared_size, size_t actual_input_len) {
    RomBuffer result = {NULL, 0, 0};

    /* Invariant: reject if declared size exceeds maximum bounds */
    if (declared_size == 0 || declared_size > MAX_ROM_SIZE) {
        result.error = 1;
        return result;
    }

    /* Invariant: reject if declared size exceeds actual input length */
    if (declared_size > actual_input_len) {
        result.error = 2;
        return result;
    }

    /* Invariant: reject SIZE_MAX or near-overflow values */
    if (declared_size == SIZE_MAX || declared_size >= (SIZE_MAX / 2)) {
        result.error = 3;
        return result;
    }

    result.data = (u8*)malloc(declared_size);
    if (result.data == NULL) {
        result.error = 4;
        return result;
    }

    /* Only copy exactly declared_size bytes - no overread */
    memcpy(result.data, input, declared_size);
    result.size = declared_size;
    return result;
}

static void free_rom_buffer(RomBuffer *buf) {
    if (buf && buf->data) {
        free(buf->data);
        buf->data = NULL;
        buf->size = 0;
    }
}

/* Test payloads as size values representing adversarial ROM sizes */
START_TEST(test_rom_size_bounds_never_exceeded)
{
    /* Invariant: Buffer reads never exceed the declared length;
     * oversized or malformed romSize values must be rejected before allocation */

    typedef struct {
        size_t declared_size;
        size_t actual_data_len;
        const char *description;
        int should_be_rejected; /* 1 = must be rejected, 0 = may succeed */
    } TestCase;

    /* Create a large buffer to use as fake ROM data */
    const size_t LARGE_BUF_SIZE = 1024;
    u8 *large_buf = (u8*)malloc(LARGE_BUF_SIZE);
    ck_assert_ptr_nonnull(large_buf);
    memset(large_buf, 0xAA, LARGE_BUF_SIZE);

    TestCase cases[] = {
        /* Oversized by 2x normal max */
        { (size_t)(MAX_ROM_SIZE * 2ULL),       LARGE_BUF_SIZE, "2x max ROM size",           1 },
        /* Oversized by 10x normal max */
        { (size_t)(MAX_ROM_SIZE * 10ULL),      LARGE_BUF_SIZE, "10x max ROM size",           1 },
        /* SIZE_MAX - classic integer overflow attack */
        { SIZE_MAX,                             LARGE_BUF_SIZE, "SIZE_MAX declared size",     1 },
        /* SIZE_MAX - 1 */
        { SIZE_MAX - 1,                         LARGE_BUF_SIZE, "SIZE_MAX-1 declared size",   1 },
        /* SIZE_MAX / 2 + 1 - near overflow */
        { SIZE_MAX / 2 + 1,                     LARGE_BUF_SIZE, "SIZE_MAX/2+1 declared size", 1 },
        /* Zero size - must be rejected */
        { 0,                                    LARGE_BUF_SIZE, "zero declared size",         1 },
        /* Declared size larger than actual data - overread attempt */
        { LARGE_BUF_SIZE * 2,                   LARGE_BUF_SIZE, "declared > actual (2x)",     1 },
        /* Declared size 10x larger than actual data */
        { LARGE_BUF_SIZE * 10,                  LARGE_BUF_SIZE, "declared > actual (10x)",    1 },
        /* Exactly at max boundary - should succeed if data is large enough */
        /* (skipped here since LARGE_BUF_SIZE < MAX_ROM_SIZE, declared > actual) */
        { MAX_ROM_SIZE + 1,                     LARGE_BUF_SIZE, "MAX_ROM_SIZE+1",             1 },
        /* 0xFFFFFFFF - common 32-bit overflow value */
        { 0xFFFFFFFF,                           LARGE_BUF_SIZE, "0xFFFFFFFF declared size",   1 },
        /* 0x80000000 - sign bit flip attack */
        { 0x80000000,                           LARGE_BUF_SIZE, "0x80000000 declared size",   1 },
        /* Negative-looking value cast to size_t */
        { (size_t)(-1),                         LARGE_BUF_SIZE, "cast -1 to size_t",          1 },
        /* Negative-looking value cast to size_t */
        { (size_t)(-100),                       LARGE_BUF_SIZE, "cast -100 to size_t",        1 },
        /* Valid small ROM - should succeed */
        { 16,                                   LARGE_BUF_SIZE, "valid 16-byte ROM",           0 },
        /* Valid 1KB ROM - should succeed */
        { 1024,                                 LARGE_BUF_SIZE, "valid 1KB ROM",               0 },
    };

    int num_cases = sizeof(cases) / sizeof(cases[0]);

    for (int i = 0; i < num_cases; i++) {
        RomBuffer result = safe_load_rom(large_buf, cases[i].declared_size, cases[i].actual_data_len);

        if (cases[i].should_be_rejected) {
            /* Invariant: oversized/invalid inputs MUST be rejected */
            ck_assert_msg(result.error != 0,
                "SECURITY VIOLATION: Input '%s' (declared_size=%zu) was NOT rejected. "
                "Buffer reads may exceed declared length (CWE-120).",
                cases[i].description, cases[i].declared_size);

            /* Invariant: no memory should be allocated for rejected inputs */
            ck_assert_msg(result.data == NULL,
                "SECURITY VIOLATION: Memory allocated for rejected input '%s'. "
                "Potential double-free or use-after-free risk.",
                cases[i].description);

            ck_assert_msg(result.size == 0,
                "SECURITY VIOLATION: Non-zero size recorded for rejected input '%s'.",
                cases[i].description);
        } else {
            /* Valid inputs should succeed */
            ck_assert_msg(result.error == 0,
                "Valid input '%s' (declared_size=%zu) was incorrectly rejected.",
                cases[i].description, cases[i].declared_size);

            if (result.error == 0) {
                /* Invariant: allocated size must exactly match declared size */
                ck_assert_msg(result.size == cases[i].declared_size,
                    "SECURITY VIOLATION: Allocated size %zu != declared size %zu for '%s'. "
                    "Buffer boundary mismatch.",
                    result.size, cases[i].declared_size, cases[i].description);

                /* Invariant: allocated size must not exceed maximum */
                ck_assert_msg(result.size <= MAX_ROM_SIZE,
                    "SECURITY VIOLATION: Allocated size %zu exceeds MAX_ROM_SIZE %d for '%s'.",
                    result.size, MAX_ROM_SIZE, cases[i].description);

                free_rom_buffer(&result);
            }
        }
    }

    free(large_buf);
}
END_TEST

START_TEST(test_no_double_free_on_rejection)
{
    /* Invariant: rejected allocations must not result in double-free conditions
     * (mirrors the double-free bug at lines 73 and 88 in desktopMain.c) */

    u8 dummy_data[256];
    memset(dummy_data, 0xBB, sizeof(dummy_data));

    /* Simulate the double-free pattern: if romData is freed twice,
     * the second free must not be called on a valid pointer */

    size_t oversized_values[] = {
        SIZE_MAX,
        SIZE_MAX - 1,
        (size_t)(-1),
        (size_t)(-2),
        MAX_ROM_SIZE + 1,
        (size_t)(MAX_ROM_SIZE) * 2,
        0,
    };
    int num_values = sizeof(oversized_values) / sizeof(oversized_values[0]);

    for (int i = 0; i < num_values; i++) {
        RomBuffer result = safe_load_rom(dummy_data, oversized_values[i], sizeof(dummy_data));

        /* Invariant: rejected inputs must have NULL data pointer to prevent double-free */
        ck_assert_msg(result.data == NULL,
            "SECURITY VIOLATION: Non-NULL data pointer after rejection of size %zu. "
            "Double-free vulnerability possible (CWE-120/415).",
            oversized_values[i]);

        /* Safe to call free_rom_buffer even on rejected - must be idempotent */
        free_rom_buffer(&result);

        /* After free, pointer must be NULL */
        ck_assert_msg(result.data == NULL,
            "SECURITY VIOLATION: Data pointer not NULL after free_rom_buffer for size %zu.",
            oversized_values[i]);
    }
}
END_TEST

START_TEST(test_memcpy_never_reads_beyond_declared_length)
{
    /* Invariant: memcpy operations must never read beyond the declared buffer length */

    /* Create a canary-protected buffer to detect overreads */
    const size_t CANARY_SIZE = 64;
    const size_t DATA_SIZE = 128;
    const size_t TOTAL_SIZE = DATA_SIZE + CANARY_SIZE;
    const u8 CANARY_BYTE = 0xDE;
    const u8 DATA_BYTE = 0xAB;

    u8 *protected_buf = (u8*)malloc(TOTAL_SIZE);
    ck_assert_ptr_nonnull(protected_buf);

    /* Fill data region */
    memset(protected_buf, DATA_BYTE, DATA_SIZE);
    /* Fill canary region */
    memset(protected_buf + DATA_SIZE, CANARY_BYTE, CANARY_SIZE);

    /* Attempt to load with declared size exactly equal to DATA_SIZE (valid) */
    RomBuffer result = safe_load_rom(protected_buf, DATA_SIZE, DATA_SIZE);
    ck_assert_msg(result.error == 0, "Valid load of DATA_SIZE bytes failed unexpectedly.");
    ck_assert_msg(result.size == DATA_SIZE, "Loaded size mismatch.");

    if (result.data != NULL) {
        /* Verify only DATA_SIZE bytes were copied - no canary bytes should appear */
        int canary_leaked = 0;
        for (size_t j = 0; j < result.size; j++) {
            if (result.data[j] == CANARY_BYTE) {
                canary_leaked = 1;
                break;
            }
        }
        /* In this controlled test, canary byte == DATA_BYTE would be ambiguous,
         * but since they differ, a canary byte in result means overread */
        ck_assert_msg(canary_leaked == 0,
            "SECURITY VIOLATION: Canary byte detected in ROM buffer - "
            "memcpy read beyond declared length (CWE-120).");

        free_rom_buffer(&result);
    }

    /* Attempt to load with declared size larger than DATA_SIZE (must be rejected) */
    RomBuffer overread_result = safe_load_rom(protected_buf, DATA_SIZE + CANARY_SIZE + 1, DATA_SIZE);
    ck_assert_msg(overread_result.error != 0,
        "SECURITY VIOLATION: Overread attempt (declared > actual) was not rejected (CWE-120).");
    ck_assert_msg(overread_result.data == NULL,
        "SECURITY VIOLATION: Memory allocated for overread attempt.");

    /* Verify canary is still intact (no corruption from rejected operation) */
    for (size_t j = DATA_SIZE; j < TOTAL_SIZE; j++) {
        ck_assert_msg(protected_buf[j] == CANARY_BYTE,
            "SECURITY VIOLATION: Canary byte at offset %zu was corrupted - "
            "out-of-bounds write detected (CWE-120).", j);
    }

    free(protected_buf);
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security_CWE120_BufferOverread");
    tc_core = tcase_create("Core");

    tcase_set_timeout(tc_core, 30);
    tcase_add_test(tc_core, test_rom_size_bounds_never_exceeded);
    tcase_add_test(tc_core, test_no_double_free_on_rejection);
    tcase_add_test(tc_core, test_memcpy_never_reads_beyond_declared_length);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}