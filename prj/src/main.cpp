
#include "ObjectPool.h"

#include <iostream>
#include <unistd.h>
#include <thread>



class SampleObject
{
    public : 
        bool Init(const ParamList &paramList)
        {
            _idx = ++_usageCount;
            std::cout << "SampleObject::Init    -- "  << _idx << "\n";
            return true; 
        }
        void Release()
        {
            std::cout << "SampleObject::Release -- "  << _idx << "\n";
        }

        int idx() const
        {
            return _idx;
        }

    private:
        static int _usageCount;
        int _idx = 0;
};

int SampleObject::_usageCount = 0;


int main(int argc, const char * argv[])
{
    std::cout << "Hello  " << std::endl;
    
    
    ObjectPool<SampleObject> pool;
    ParamList params;
    params["Host"]      =  "111.222.3333.444";
    params["Port"]      =  "6666";
    params["Dbname"]    =  "dbname";
    params["User"]      =  "username";

    if (true == pool.Init(params))
    {
        const int count = 100;
        std::thread th[count];

        for (int i = 0; i < count; ++i)
        {
            th[i] = std::thread([&pool]() -> void
                {
                    ObjectPoolLock<SampleObject> p (pool);
                    std::cout << "Acquireing : " << p.get()->idx() << '\n';
                    usleep(10000);
                    std::cout << "Releasing  : " << p.get()->idx() << '\n';
                });
        }

        for (int i = 0; i < count; ++i)
            th[i].join();

    }


    pool.Release();
    
    return 0;
}
