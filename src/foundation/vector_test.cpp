#include "foundation/vector.h"

#include <gtest/gtest.h>

TEST(vector, insert)
{
    def_vector_of(int) v;
    vector_init(&v);
    EXPECT_EQ(0, v.capacity);

    vector_add(&v, 10);
    EXPECT_EQ(4, v.capacity);
    EXPECT_EQ(1, v.size);

    vector_add(&v, 11);
    EXPECT_EQ(4, v.capacity);
    EXPECT_EQ(2, v.size);

    vector_add(&v, 12);
    EXPECT_EQ(4, v.capacity);
    EXPECT_EQ(3, v.size);

    vector_add(&v, 13);
    EXPECT_EQ(4, v.capacity);
    EXPECT_EQ(4, v.size);

    vector_add(&v, 20);
    EXPECT_EQ(8, v.capacity);
    EXPECT_EQ(5, v.size);

    vector_add(&v, 21);
    vector_add(&v, 22);
    vector_add(&v, 23);
    EXPECT_EQ(8, v.capacity);
    EXPECT_EQ(8, v.size);

    vector_add(&v, 24);
    EXPECT_EQ(16, v.capacity);
    EXPECT_EQ(9, v.size);

    EXPECT_EQ(10, vector_at(&v, 0));
    EXPECT_EQ(11, vector_at(&v, 1));
    EXPECT_EQ(12, vector_at(&v, 2));
    EXPECT_EQ(13, vector_at(&v, 3));
    EXPECT_EQ(20, vector_at(&v, 4));
    EXPECT_EQ(21, vector_at(&v, 5));
    EXPECT_EQ(22, vector_at(&v, 6));
    EXPECT_EQ(23, vector_at(&v, 7));
    EXPECT_EQ(24, vector_at(&v, 8));

    vector_foreach(x, &v) {
        if (*x < 20) {
            *x += 5;
        }
    }

    EXPECT_EQ(15, vector_at(&v, 0));
    EXPECT_EQ(16, vector_at(&v, 1));
    EXPECT_EQ(17, vector_at(&v, 2));
    EXPECT_EQ(18, vector_at(&v, 3));
    EXPECT_EQ(20, vector_at(&v, 4));
    EXPECT_EQ(21, vector_at(&v, 5));
    EXPECT_EQ(22, vector_at(&v, 6));
    EXPECT_EQ(23, vector_at(&v, 7));
    EXPECT_EQ(24, vector_at(&v, 8));

    *vector_ptr_at(&v, 4) = 30;

    EXPECT_EQ(15, vector_at(&v, 0));
    EXPECT_EQ(16, vector_at(&v, 1));
    EXPECT_EQ(17, vector_at(&v, 2));
    EXPECT_EQ(18, vector_at(&v, 3));
    EXPECT_EQ(30, vector_at(&v, 4));
    EXPECT_EQ(21, vector_at(&v, 5));
    EXPECT_EQ(22, vector_at(&v, 6));
    EXPECT_EQ(23, vector_at(&v, 7));
    EXPECT_EQ(24, vector_at(&v, 8));

    EXPECT_EQ(24, vector_pop(&v));
    EXPECT_EQ(16, v.capacity);
    EXPECT_EQ(8, v.size);
}
