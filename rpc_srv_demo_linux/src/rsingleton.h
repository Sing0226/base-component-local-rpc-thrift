#ifndef R_SINGLETON_H
#define R_SINGLETON_H
#include <mutex>

template<typename T>
class Singleton
{
public:
    static T* getInstance()
    {
        if(nullptr == m_instance)
        {
            std::lock_guard lock(m_mutex);
            if(nullptr == m_instance)
            {
                T* ins = new T;
                m_instance = ins;
            }
        }
        return m_instance;
    }

protected:
    Singleton() {}
private:
    Singleton(const Singleton &)=delete;
    Singleton& operator=(const Singleton&)=delete;
    Singleton(const Singleton &&)=delete;
    Singleton& operator=(const Singleton&&)=delete;
    static T* m_instance;
    static std::mutex m_mutex;

    class DC{
        ~DC()
        {
            delete  m_instance;
            m_instance = nullptr;
        }
    };

    static DC dc;
};

template<typename T>
T* Singleton<T>::m_instance = nullptr;
template<typename T>
std::mutex Singleton<T>::m_mutex;

#endif // R_SINGLETON_H
