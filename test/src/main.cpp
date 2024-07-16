
#include "ObjectPool.h"
#include <gtest/gtest.h>


TEST(ObjectPool, Basic)
{
    class PoolObj
    {
        public:
        bool Init()
        {
            _val = 0;
            return true;
        }
        void Release()
        {
            _val = -2;
        }

        int _val = -1;
    };


    ObjectPool<PoolObj> pool(2);
    pool.Init();

    PoolObj *obj1 = pool.ObjectAcquire();
    EXPECT_NE(nullptr, obj1);
    EXPECT_EQ(0, obj1->_val);
    obj1->_val = 1;

    PoolObj *obj2 = pool.ObjectAcquire();
    EXPECT_NE(nullptr, obj2);
    EXPECT_EQ(0, obj2->_val);
    obj2->_val = 2;

    pool.ObjectRelease(obj1);
    PoolObj *obj1_1 =  pool.ObjectAcquire();
    EXPECT_EQ(obj1_1, obj1);

    pool.ObjectRelease(obj2);
    PoolObj *obj2_1 =  pool.ObjectAcquire();
    EXPECT_EQ(obj2_1, obj2);
    pool.ObjectRelease(obj2);

    {
        ObjectPoolLock ol(pool);
        EXPECT_EQ(2, ol->_val);
    }

    PoolObj *obj2_2 =  pool.ObjectAcquire();
    EXPECT_EQ(obj2_2, obj2);
    pool.ObjectRelease(obj2);

    pool.Release();
}


TEST(ObjectPool, Init)
{
    class PoolObj
    {
        public:
        bool Init(int a1, const char *a2)
        {
            _a1 = a1;
            _a2 = a2;
            return true;
        }
        void Release()
        {
            _a1 = -1;
            _a2 = nullptr;
        }

        int         _a1 = -1;
        const char *_a2 = nullptr;
    };


    ObjectPool<PoolObj> pool(2);
    const char *a2 = "Hello";
    pool.Init(9, a2);

    PoolObj *p1 = pool.ObjectAcquire();
    PoolObj *p2 = pool.ObjectAcquire();

    EXPECT_NE   (nullptr,   p1);
    EXPECT_EQ   (9,         p1->_a1);
    EXPECT_EQ   (a2,        p1->_a2);
    EXPECT_STREQ(a2,        p1->_a2);

    EXPECT_NE   (nullptr,   p2);
    EXPECT_EQ   (9,         p2->_a1);
    EXPECT_EQ   (a2,        p2->_a2);
    EXPECT_STREQ(a2,        p2->_a2);

    pool.ObjectRelease(p2);
    pool.ObjectRelease(p1);
    pool.Release();
}




int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

