#include "TestPrologue.h"
#include <cal3d/memory.h>
#include <cal3d/vector4.h>

FIXTURE(MemoryFixture) {
};

TEST_F(MemoryFixture, can_grow_SSEArray) {
    cal3d::SSEArray<CalVector4> v;

    v.push_back(CalVector4(1, 2, 3, 4));
    CHECK_EQUAL(1u, v.size());

    CHECK_EQUAL(CalVector4(1, 2, 3, 4), v[0]);

    v.push_back(CalVector4(5, 6, 7, 8));
    CHECK_EQUAL(2u, v.size());
    CHECK_EQUAL(CalVector4(1, 2, 3, 4), v[0]);
    CHECK_EQUAL(CalVector4(5, 6, 7, 8), v[1]);

    v.push_back(CalVector4(9, 0, 9, 0));
    CHECK_EQUAL(3u, v.size());
    CHECK_EQUAL(CalVector4(1, 2, 3, 4), v[0]);
    CHECK_EQUAL(CalVector4(5, 6, 7, 8), v[1]);
    CHECK_EQUAL(CalVector4(9, 0, 9, 0), v[2]);

    v.push_back(CalVector4(8, 6, 4, 2));
    CHECK_EQUAL(4u, v.size());
    CHECK_EQUAL(CalVector4(1, 2, 3, 4), v[0]);
    CHECK_EQUAL(CalVector4(5, 6, 7, 8), v[1]);
    CHECK_EQUAL(CalVector4(9, 0, 9, 0), v[2]);
    CHECK_EQUAL(CalVector4(8, 6, 4, 2), v[3]);
}

TEST_F(MemoryFixture, can_copy_SSEArray) {
    cal3d::SSEArray<CalVector4> v;
    v.push_back(CalVector4(1, 2, 3, 4));
    v.push_back(CalVector4(5, 6, 7, 8));
    v.push_back(CalVector4(9, 0, 9, 0));
    v.push_back(CalVector4(8, 6, 4, 2));

    cal3d::SSEArray<CalVector4> u(v);
    CHECK_EQUAL(4u, u.size());
    CHECK_EQUAL(CalVector4(1, 2, 3, 4), u[0]);
    CHECK_EQUAL(CalVector4(5, 6, 7, 8), u[1]);
    CHECK_EQUAL(CalVector4(9, 0, 9, 0), u[2]);
    CHECK_EQUAL(CalVector4(8, 6, 4, 2), u[3]);
}

TEST_F(MemoryFixture, can_copy_empty_SSEArray) {
    cal3d::SSEArray<CalVector4> v;
    cal3d::SSEArray<CalVector4> u(v);
    CHECK_EQUAL(0u, u.size());
}

struct MagicValue {
    MagicValue()
        : value(10)
    {}

    MagicValue(const MagicValue& v) {
        CHECK_EQUAL(v.value, value);
    }

    MagicValue& operator=(const MagicValue& v) {
        CHECK_EQUAL(v.value, value);
        return *this;
    }

    unsigned value;
};

TEST_F(MemoryFixture, sized_initializer_uses_constructor) {
    cal3d::SSEArray<MagicValue> v(1);
    CHECK_EQUAL(1u, v.size());
    CHECK_EQUAL(10u, v[0].value);
}

TEST_F(MemoryFixture, can_initialize_with_array_iterators) {
    CalVector4 arr[] = {
        CalVector4(1, 2, 3),
        CalVector4(4, 5, 6)
    };
    cal3d::SSEArray<CalVector4> sseArray(arrayBegin(arr), arrayEnd(arr));
    CHECK_EQUAL(2U, sseArray.size());
    CHECK_EQUAL(CalVector4(1, 2, 3), sseArray[0]);
    CHECK_EQUAL(CalVector4(4, 5, 6), sseArray[1]);
}
