
// clang-format off


#ifndef __OBJECT_POOL_H__
#define __OBJECT_POOL_H__

//#include <bs/FsDbHandler/FsDbHandler.h>
//#include <bs/FsDbHandler/FsDbLog.h>

#include <atomic>
#include <vector>
#include <set>
#include <map>
#include <condition_variable>
#include <iostream>


#define  LLOG_ERROR std::cout 

constexpr int ObjectPoolCountDefault = 8;

using ParamList     = std::map<std::string, std::string>;


template <typename TPoolObject>
class ObjectPool
{
    public  :

        ObjectPool(size_t size = ObjectPoolCountDefault)
            : _size(size)
            , _objContainer(size)
            , _objIndexFree()
            , _objIndexBusy()
        {
        }

        bool            Init()                              { return Init({});  }
        bool            Init(const ParamList  &prmList);
        void            Release();

        TPoolObject *   ObjectAcquire();
        void            ObjectRelease(TPoolObject *p);
        TPoolObject *   ObjectRaw(size_t idx);

        size_t          Size() const                        { return _size;     }

        void            Close(bool v);
        bool            Close() const;

    private:

       TPoolObject *    ReserveSlot();

        size_t                      _size = ObjectPoolCountDefault;
        ParamList                   _paramList;

        std::vector<TPoolObject>    _objContainer;
        std::set<TPoolObject*  >    _objIndexFree;
        std::set<TPoolObject*  >    _objIndexBusy;

        std::condition_variable     _cv;
        std::mutex                  _mtxPool;
        std::atomic_bool            _doClose = false;
};



template <typename TPoolObject>
bool ObjectPool<TPoolObject>::Init(const ParamList  &prmList)
{
    bool rv = true;

    Release();

    _paramList = prmList;

    for (auto &item : _objContainer)
    {
        if (true != item.Init(_paramList))
        {
            LLOG_ERROR << "Unable to initialize PoolObject";
            rv = false;
            break;
        }
        else
        {
            _objIndexFree.insert(&item);
        }
    }

    if (false == rv)
        Release();

    Close(false);
    return rv;
}


template <typename TPoolObject>
void ObjectPool<TPoolObject>::Release()
{
    Close(true);

    _objIndexFree.clear();
    _objIndexBusy.clear();

    for (auto &item : _objContainer)
        item.Release();
}


template <typename TPoolObject>
void ObjectPool<TPoolObject>::Close(bool val)
{
    std::unique_lock<std::mutex> lock(_mtxPool);
    _doClose = val;
    _cv.notify_all();
}


template <typename TPoolObject>
bool ObjectPool<TPoolObject>::Close() const
{
    return _doClose;
}


template <typename TPoolObject>
void ObjectPool<TPoolObject>::ObjectRelease(TPoolObject *p)
{
    std::unique_lock<std::mutex> lock(_mtxPool);

    auto it = _objIndexBusy.find(p);
    if (it != _objIndexBusy.end())
    {
        _objIndexBusy.erase(it);
        _objIndexFree.emplace(p);
        _cv.notify_all();
    }
}




template <typename TPoolObject>
TPoolObject *ObjectPool<TPoolObject>::ReserveSlot()
{
    TPoolObject *result = nullptr;
    if (false == Close())
    {
        auto itBegin = _objIndexFree.cbegin();
        if (itBegin != _objIndexFree.cend())
        {
            result = *itBegin;
            _objIndexFree.erase(itBegin);
            _objIndexBusy.emplace(result);
        }
    }

    return result;
}




template <typename TPoolObject>
TPoolObject *ObjectPool<TPoolObject>::ObjectAcquire()
{
    TPoolObject *item = nullptr;

    if (false == Close())
    {
        std::unique_lock<std::mutex> lock(_mtxPool);
        _cv.wait(lock,
            [this, &item]{ return (item = ReserveSlot()) || (true == Close());}   );

        if (item && (false == Close()))
        {
            return item;
        }
    }

    return nullptr;
}

template <typename TPoolObject>
TPoolObject *ObjectPool<TPoolObject>::ObjectRaw(size_t idx)
{
    if (false == Close() && (/*(idx > -1) &&*/ (idx < _size)) )
    {
        return &(_objContainer.at(idx));
    }
    return nullptr;
}


template <typename TPoolObject>
class ObjectPoolLock
{
    public:
        virtual ~ObjectPoolLock();
        ObjectPoolLock(ObjectPool <TPoolObject> &pool);
        ObjectPoolLock(const ObjectPoolLock &)              = delete;
        ObjectPoolLock(ObjectPoolLock &&)                   = default;
        ObjectPoolLock &operator = (const ObjectPoolLock &) = delete;
        ObjectPoolLock &operator = (ObjectPoolLock&&)       = default;

        operator TPoolObject*();
        operator TPoolObject*() const;
        operator TPoolObject&();
        operator TPoolObject&() const;
        TPoolObject*        operator ->();
        const TPoolObject*  operator ->() const;
        TPoolObject&        operator *();
        const TPoolObject&  operator *() const;

        TPoolObject*        get();
        const TPoolObject*  get() const;

    private:
        ObjectPool <TPoolObject>  & _pool;
        TPoolObject                   * _object;
};


template <typename TPoolObject>
ObjectPoolLock<TPoolObject>::ObjectPoolLock(ObjectPool <TPoolObject> &pool)
    : _pool(pool)
{
    _object = _pool.ObjectAcquire();
}


template <typename TPoolObject>
ObjectPoolLock<TPoolObject>::~ObjectPoolLock()
{
    _pool.ObjectRelease(_object);
}


template <typename TPoolObject>
ObjectPoolLock<TPoolObject>::operator TPoolObject*()
{
    return _object;
}


template <typename TPoolObject>
ObjectPoolLock<TPoolObject>::operator TPoolObject*() const
{
    return _object;
}


template <typename TPoolObject>
ObjectPoolLock<TPoolObject>::operator TPoolObject&()
{
    return operator TPoolObject*();
}


template <typename TPoolObject>
ObjectPoolLock<TPoolObject>::operator TPoolObject&() const
{
    return operator TPoolObject*();
}


template <typename TPoolObject>
TPoolObject* ObjectPoolLock<TPoolObject>::operator ->()
{
    return operator TPoolObject*();
}


template <typename TPoolObject>
const TPoolObject* ObjectPoolLock<TPoolObject>::operator ->() const
{
    return operator TPoolObject*();
}


template <typename TPoolObject>
TPoolObject& ObjectPoolLock<TPoolObject>::operator *()
{
    return operator TPoolObject&();
}


template <typename TPoolObject>
const TPoolObject& ObjectPoolLock<TPoolObject>::operator *() const
{
    return operator TPoolObject&();
}


template <typename TPoolObject>
TPoolObject* ObjectPoolLock<TPoolObject>::get()
{
    return operator TPoolObject*();
}


template <typename TPoolObject>
const TPoolObject*  ObjectPoolLock<TPoolObject>::get() const
{
    return operator TPoolObject*();
}




#endif //__OBJECT_POOL_H__

// clang-format on
