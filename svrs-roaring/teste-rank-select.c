#include "roaring.c"
#include "roaring.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <math.h>

bool roaring_iterator_sumall(uint32_t value, void *param) {
    *(uint32_t *)param += value;
    return true;  // iterate till the end
}

int main() {
    uint32_t bsize, m, new_density, now; double density;
    srand(time(NULL));
    printf("Enter x for (2^x bitvector size): \n");scanf("%d", &bsize);
    printf("Enter density: (example: 0.30 = 30 percent): \n");scanf("%lf", &density);
    printf("Enter number of measurements: (0<m<=bitmap_size)\n");scanf("%d", &m);
    bsize = pow(2,bsize);
    new_density = (1/density);
    // create a new empty bitmap
    roaring_bitmap_t *r1 = roaring_bitmap_create();
    // then we can add values
    for (uint32_t i = 0; i < bsize; i++) {
        now = rand();
        if(now%new_density==0) roaring_bitmap_add(r1, i);
        //printf("value: %d", now%new_density); 
    }
    // check whether a value is contained
    uint32_t cardinality = roaring_bitmap_get_cardinality(r1);
    printf("Cardinality = %d \n", cardinality);

    // if your bitmaps have long runs, you can compress them by calling
    // run_optimize
    uint32_t expectedsizebasic = roaring_bitmap_portable_size_in_bytes(r1);
    printf("Size in MB: %lf\n", expectedsizebasic/1000000.0);
    //roaring_bitmap_run_optimize(r1);
    //uint32_t expectedsizerun = roaring_bitmap_portable_size_in_bytes(r1);
    //printf("size before run optimize %d MB, and after %d MB\n",           expectedsizebasic/1000000, expectedsizerun/1000000);

    // create a new bitmap containing the values {1,2,3,5,6}
    //roaring_bitmap_t *r2 = roaring_bitmap_of(5, 1, 2, 3, 5, 6);
    //roaring_bitmap_printf(r2);  // print it

    // we can also create a bitmap from a pointer to 32-bit integers
    //uint32_t somevalues[] = {2, 3, 4};
    //roaring_bitmap_t *r3 = roaring_bitmap_of_ptr(3, somevalues);

    /* we can also go in reverse and go from arrays to bitmaps
    uint64_t card1 = roaring_bitmap_get_cardinality(r1);
    uint32_t *arr1 = (uint32_t *)malloc(card1 * sizeof(uint32_t));
    assert(arr1 != NULL);
    roaring_bitmap_to_uint32_array(r1, arr1);
    roaring_bitmap_t *r1f = roaring_bitmap_of_ptr(card1, arr1);
    free(arr1);
    assert(roaring_bitmap_equals(r1, r1f));  // what we recover is equal
    roaring_bitmap_free(r1f);

    // we can go from arrays to bitmaps from "offset" by "limit"
    size_t offset = 100;
    size_t limit = 1000;
    uint32_t *arr3 = (uint32_t *)malloc(limit * sizeof(uint32_t));
    assert(arr3 != NULL);
    roaring_bitmap_range_uint32_array(r1, offset, limit, arr3);
    free(arr3);

    // we can copy and compare bitmaps
    roaring_bitmap_t *z = roaring_bitmap_copy(r3);
    assert(roaring_bitmap_equals(r3, z));  // what we recover is equal
    roaring_bitmap_free(z);

    // we can compute union two-by-two
    roaring_bitmap_t *r1_2_3 = roaring_bitmap_or(r1, r2);
    roaring_bitmap_or_inplace(r1_2_3, r3);

    // we can compute a big union
    const roaring_bitmap_t *allmybitmaps[] = {r1, r2, r3};
    roaring_bitmap_t *bigunion = roaring_bitmap_or_many(3, allmybitmaps);
    assert(
        roaring_bitmap_equals(r1_2_3, bigunion));  // what we recover is equal
    // can also do the big union with a heap
    roaring_bitmap_t *bigunionheap =
        roaring_bitmap_or_many_heap(3, allmybitmaps);
    assert(roaring_bitmap_equals(r1_2_3, bigunionheap));

    roaring_bitmap_free(r1_2_3);
    roaring_bitmap_free(bigunion);
    roaring_bitmap_free(bigunionheap);

    // we can compute intersection two-by-two
    roaring_bitmap_t *i1_2 = roaring_bitmap_and(r1, r2);
    roaring_bitmap_free(i1_2);

    // we can write a bitmap to a pointer and recover it later
    uint32_t expectedsize = roaring_bitmap_portable_size_in_bytes(r1);
    char *serializedbytes = malloc(expectedsize);
    roaring_bitmap_portable_serialize(r1, serializedbytes);
    roaring_bitmap_t *t = roaring_bitmap_portable_deserialize(serializedbytes);
    assert(roaring_bitmap_equals(r1, t));  // what we recover is equal
    roaring_bitmap_free(t);
    // we can also check whether there is a bitmap at a memory location without
    // reading it
    size_t sizeofbitmap =
        roaring_bitmap_portable_deserialize_size(serializedbytes, expectedsize);
    assert(sizeofbitmap ==
           expectedsize);  // sizeofbitmap would be zero if no bitmap were found
    // we can also read the bitmap "safely" by specifying a byte size limit:
    t = roaring_bitmap_portable_deserialize_safe(serializedbytes, expectedsize);
    assert(roaring_bitmap_equals(r1, t));  // what we recover is equal
    roaring_bitmap_free(t);

    free(serializedbytes);

    // we can iterate over all values using custom functions
    uint32_t counter = 0;
    clock_t start, end;
    start = clock();
    roaring_iterate(r1, roaring_iterator_sumall, &counter);
    end = clock();
    double real_time = (double)(end-start)/CLOCKS_PER_SEC;
    printf("start = %ld, end=%ld, elapsed_time=%ld, real_time=%lf", start, end, end-start, real_time);

    // we can also create iterator structs
    counter = 0;
    roaring_uint32_iterator_t *i = roaring_create_iterator(r1);
    while (i->has_value) {
        counter++;  // could use    i->current_value
        roaring_advance_uint32_iterator(i);
    }
    // you can skip over values and move the iterator with
    // roaring_move_uint32_iterator_equalorlarger(i,someintvalue)

    roaring_free_uint32_iterator(i);
    // roaring_bitmap_get_cardinality(r1) == counter

    // for greater speed, you can iterate over the data in bulk
    i = roaring_create_iterator(r1);
    uint32_t buffer[256];
    while (1) {
        uint32_t ret = roaring_read_uint32_iterator(i, buffer, 256);
        for (uint32_t j = 0; j < ret; j++) {
            counter += buffer[j];
        }
        if (ret < 256) {
            break;
        }
    }
    roaring_free_uint32_iterator(i);

    roaring_bitmap_free(r1);
    roaring_bitmap_free(r2);
    roaring_bitmap_free(r3);*/

    double rank, rank_time, select_time;
    clock_t start, end;

    start = clock();
    for(uint32_t i = 0; i < m; i++){
        rank = roaring_bitmap_rank(r1, ((i*1.0/m)*bsize));
        //rank = roaring_bitmap_rank(r1, 10000000); 
        //printf("rank: %f", rank);
    }
    end = clock();
    rank_time = (double)(end-start)/CLOCKS_PER_SEC;
    printf("Rank time: %f\n", rank_time);

    return EXIT_SUCCESS;
}