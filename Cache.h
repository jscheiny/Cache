#ifndef SCHEINERMAN_CACHE_H
#define SCHEINERMAN_CACHE_H

#include <memory>
#include <map>


template<typename Key, typename Value>
class CacheDeleter;

template<typename Key, typename Value>
class Cache {

public:
    using cache_type = std::map<Key, std::weak_ptr<Value>>;
    using deleter_type = CacheDeleter<Key, Value>;

    template<typename... Args>
    Cache(Args&&... args) : cache_(std::forward<Args>(args)...) {}

    std::shared_ptr<Value> operator[] (const Key& key);

    size_t size() { return cache_.size(); }

private:
    cache_type cache_;

};

template<typename Key, typename Value>
std::shared_ptr<Value> Cache<Key, Value>::operator[] (const Key& key) {
    auto& value = cache_[key];
    auto ptr = value.lock();
    if(ptr)
        return ptr;
    value = ptr = std::shared_ptr<Value>(new Value(key), deleter_type(&cache_, key));
    return ptr;
}

template<typename Key, typename Value>
class CacheDeleter : public std::default_delete<Value> {

public:
    using cache_type = typename Cache<Key, Value>::cache_type;

    CacheDeleter(cache_type* cache, Key key) : cache_(cache), key_(key) {}

    void operator() (Value* value) const {
        std::default_delete<Value>::operator()(value);
        cache_->erase(cache_->find(key_));
    }

private:
    mutable cache_type* cache_;
    Key key_;

};

#endif
