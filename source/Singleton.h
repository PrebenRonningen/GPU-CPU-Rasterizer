#pragma once

template <typename T>
class Singleton
{
public:
	static T* GetInstance()
	{
		if ( !m_pInstance )
		{
			m_pInstance = new T();
		}
		return m_pInstance;
	}

protected:
	static T* m_pInstance;
	Singleton() = default;
	~Singleton() = default;

	virtual void Reset()
	{
		if ( m_pInstance != nullptr )
		{
			delete m_pInstance;
			m_pInstance = nullptr;
		}
	};
private:

	Singleton& operator=(const Singleton& other)
	{
	};
	Singleton(const Singleton& other)
	{
	};
};

template<typename T>
typename T* Singleton<T>::m_pInstance = 0;