/***
  This file is part of PulseAudio.

  Copyright 2016 Ahmed S. Darwish <darwish.07@gmail.com>

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with PulseAudio; if not, see <http://www.gnu.org/licenses/>.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <check.h>
#include <stdint.h>

#include <pulsecore/tagstruct.h>

static void test_tagstruct_function(pa_tagstruct* t) {
    int i, nqwords = 100;
    const char *srcstr = "1234567890", *dststr = NULL;
    uint64_t value;

    for (i = 0; i < nqwords; i++)
        pa_tagstruct_putu64(t, (uint64_t) i * 2L);
    pa_tagstruct_puts(t, srcstr);

    for (i = 0; i < nqwords; i++) {
        ck_assert(!pa_tagstruct_getu64(t, &value));
        ck_assert(value == (uint64_t )i * 2L);
    }
    ck_assert(!pa_tagstruct_gets(t, &dststr));
    ck_assert(!strcmp(srcstr, dststr));
}

/* Test tagstruct copy using an original tagstruct with
 * @nbytes uint8s (+ their tags) and a 7-byte string. */
static void test_tagstruct_copy(pa_tagstruct* orig, int nbytes) {
    pa_tagstruct *copy;
    const char *srcstr = "ABCDEF", *dststr = NULL;
    uint8_t i, value;

    for (i = 0; i < nbytes; i++)
        pa_tagstruct_putu8(orig, i);
    pa_tagstruct_puts(orig, srcstr);

    copy = pa_tagstruct_copy(orig);

    /* Zero the original tagstruct to make sure we don't have
     * _any_ dangling pointers to its data in the new copy. */
    memset(orig, 0, pa_tagstruct_sizeof());

    for (i = 0; i < nbytes; i++) {
        ck_assert(!pa_tagstruct_getu8(copy, &value));
        ck_assert(value == i);
    }
    ck_assert(!pa_tagstruct_gets(copy, &dststr));
    ck_assert(!strcmp(dststr, srcstr));

    /* Make sure the new tagstruct works as expected afterwards */
    test_tagstruct_function(copy);
}

START_TEST(appended_tagstruct_copy_test) {
    pa_tagstruct *orig = pa_tagstruct_new();

    /* Don't exceed tagstruct's MAX_APPENDED_SIZE. Remember
     * that each element has an accompanying 1-byte tag. */
    test_tagstruct_copy(orig, 60);
}
END_TEST

START_TEST(dynamic_tagstruct_copy_test) {
    pa_tagstruct *orig = pa_tagstruct_new();

    /* Exceed tagstruct's MAX_APPENDED_SIZE */
    test_tagstruct_copy(orig, 120);
}
END_TEST

START_TEST(fixed_tagstruct_copy_test) {
    pa_tagstruct *orig, *copy;
    const int count = 100;
    uint8_t data[count * 2], value;
    int i;

    /* Fill in the tagsturct-spec conforming format */
    for (i = 0; i < count*2; i+=2) {
        data[i] = PA_TAG_U8;
        data[i+1] = i;
    }

    orig = pa_tagstruct_new_fixed(data, sizeof(data));
    copy = pa_tagstruct_copy(orig);

    /* Zero the original tagstruct to make sure we don't have
     * _any_ dangling pointers to its data in the new copy. */
    memset(orig, 0, pa_tagstruct_sizeof());

    for (i = 0; i < count; i++) {
        ck_assert(!pa_tagstruct_getu8(copy, &value));
        ck_assert(value == i*2);
    }

    /* Make sure the new tagstruct works as expected afterwards */
    test_tagstruct_function(copy);
}
END_TEST


int main(int argc, char *argv[]) {
    int failed = 0;
    Suite *s;
    TCase *tc;
    SRunner *sr;

    if (!getenv("MAKE_CHECK"))
        pa_log_set_level(PA_LOG_DEBUG);

    s = suite_create("Tagstruct");
    tc = tcase_create("tagstruct_copy");
    tcase_add_test(tc, appended_tagstruct_copy_test);
    tcase_add_test(tc, dynamic_tagstruct_copy_test);
    tcase_add_test(tc, fixed_tagstruct_copy_test);

    /* The default timeout is too small,
     * set it to a reasonable large one. */
    tcase_set_timeout(tc, 60 * 60);
    suite_add_tcase(s, tc);

    sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
