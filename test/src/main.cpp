
#include "ObjectPool.h"
#include <gtest/gtest.h>


TEST(DummyTest, DummyTest)
{
    class PoolObj
    {
        public:
        bool Init(const ParamList &)
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


int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

