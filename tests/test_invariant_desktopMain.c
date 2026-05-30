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
        /* Exactly at max boundary + 1 - should be rejected */
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

    int num_cases = (int)(sizeof(cases) / sizeof(cases[0]));

    for (int i = 0; i < num_cases; i++) {
        RomBuffer result = safe_load_rom(large_buf, cases[i].declared_size, cases[i].actual_data_len);

        if (cases[i].should_be_rejected) {
            /* Invariant: oversized/invalid inputs MUST be rejected */
            ck_assert_msg(result.error != 0,
                "SECURITY VIOLATION: Input '%s' (declared_size=%zu) was NOT rejected. "
                "Buffer reads may exceed declared length (CWE-120).",
                cases[i].description, cases[i].declared_size);
        } else {
            /* Valid inputs should succeed */
            ck_assert_msg(result.error == 0,
                "Valid input '%s' (declared_size=%zu) was incorrectly rejected (error=%d).",
                cases[i].description, cases[i].declared_size, result.error);
            /* Verify buffer was allocated with correct size */
            ck_assert_msg(result.size == cases[i].declared_size,
                "Buffer size mismatch for '%s': expected %zu, got %zu.",
                cases[i].description, cases[i].declared_size, result.size);
        }

        free_rom_buffer(&result);
    }

    free(large_buf);
}
END_TEST

/* Test that free_rom_buffer is idempotent and safe to call multiple times
 * (verifies NULL-after-free pattern prevents use-after-free) */
START_TEST(test_free_rom_buffer_idempotent)
{
    /* Verify that free_rom_buffer sets pointer to NULL after freeing,
     * making repeated calls safe (idempotent free / NULL-after-free safety) */
    RomBuffer buf;
    buf.data = (u8*)malloc(64);
    ck_assert_ptr_nonnull(buf.data);
    buf.size = 64;
    buf.error = 0;

    /* First free should work */
    free_rom_buffer(&buf);
    ck_assert_ptr_null(buf.data);
    ck_assert_uint_eq(buf.size, 0);

    /* Second free should be safe (idempotent) */
    free_rom_buffer(&buf);
    ck_assert_ptr_null(buf.data);
    ck_assert_uint_eq(buf.size, 0);

    /* Calling with NULL struct pointer should not crash */
    free_rom_buffer(NULL);
}
END_TEST

/* Test boundary conditions at exactly MAX_ROM_SIZE */
START_TEST(test_rom_size_exact_boundary)
{
    /* We can't allocate MAX_ROM_SIZE in a test, but we can verify
     * the logic accepts MAX_ROM_SIZE when actual_input_len is sufficient */
    const size_t SMALL_BUF = 128;
    u8 *buf = (u8*)malloc(SMALL_BUF);
    ck_assert_ptr_nonnull(buf);
    memset(buf, 0xBB, SMALL_BUF);

    /* Exactly MAX_ROM_SIZE but actual data is smaller - should be rejected
     * because declared_size > actual_input_len */
    RomBuffer result = safe_load_rom(buf, MAX_ROM_SIZE, SMALL_BUF);
    ck_assert_msg(result.error != 0,
        "MAX_ROM_SIZE with insufficient actual data should be rejected");
    free_rom_buffer(&result);

    /* Just over MAX_ROM_SIZE - should always be rejected */
    result = safe_load_rom(buf, MAX_ROM_SIZE + 1, MAX_ROM_SIZE + 1);
    ck_assert_msg(result.error != 0,
        "MAX_ROM_SIZE+1 should always be rejected regardless of actual data");
    free_rom_buffer(&result);

    /* Valid size within bounds and within actual data */
    result = safe_load_rom(buf, SMALL_BUF, SMALL_BUF);
    ck_assert_msg(result.error == 0,
        "Valid size equal to actual data should succeed");
    ck_assert_uint_eq(result.size, SMALL_BUF);
    free_rom_buffer(&result);

    free(buf);
}
END_TEST

/* Test that buffer content is correctly copied without overread */
START_TEST(test_no_buffer_overread)
{
    /* Invariant: memcpy must not read beyond actual_input_len */
    const size_t DATA_SIZE = 256;
    u8 *source = (u8*)malloc(DATA_SIZE);
    ck_assert_ptr_nonnull(source);

    /* Fill with known pattern */
    for (size_t i = 0; i < DATA_SIZE; i++) {
        source[i] = (u8)(i & 0xFF);
    }

    /* Load a subset of the data */
    RomBuffer result = safe_load_rom(source, 128, DATA_SIZE);
    ck_assert_int_eq(result.error, 0);
    ck_assert_uint_eq(result.size, 128);

    /* Verify only the requested bytes were copied correctly */
    for (size_t i = 0; i < 128; i++) {
        ck_assert_uint_eq(result.data[i], (u8)(i & 0xFF));
    }

    free_rom_buffer(&result);
    free(source);
}
END_TEST

static Suite *rom_security_suite(void) {
    Suite *s = suite_create("ROM Security - Buffer Length Invariant");

    TCase *tc_bounds = tcase_create("Bounds Checking");
    tcase_add_test(tc_bounds, test_rom_size_bounds_never_exceeded);
    tcase_add_test(tc_bounds, test_rom_size_exact_boundary);
    suite_add_tcase(s, tc_bounds);

    TCase *tc_memory = tcase_create("Memory Safety");
    tcase_add_test(tc_memory, test_free_rom_buffer_idempotent);
    tcase_add_test(tc_memory, test_no_buffer_overread);
    suite_add_tcase(s, tc_memory);

    return s;
}

int main(void) {
    int number_failed;
    Suite *s = rom_security_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
