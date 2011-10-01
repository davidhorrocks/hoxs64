#ifndef __TCARRAY_H__
#define	__TCARRAY_H__

template <class T>
class CArray;

template <class T>
class CArrayElement
{
public:
	CArrayElement()
	{
	};
	~CArrayElement()
	{
	};
	class CArrayElement &operator=(class CArrayElement &rhs)
	{
		m_data = rhs.m_data;
		return *this;
	}
	friend class CArray<T>;
private:
	T m_data;
};

template <class T>
class CArray
{
public:
	CArray()
	{
		m_mem=NULL;
		Clear();
	}
	~CArray()
	{
		Clear();
	}

	typedef int (*CompareFunc)(T &v1, T &v2);

	CArrayElement<T> *FindElement(CompareFunc fn, T &item)
	{
		unsigned __int3264 i;
		if (m_count == 0)
			return NULL;
		for (i=0 ; i < m_count ; i++)
		{
			if (fn(m_mem[i], item) == 0)
				return &m_mem[i];
		}
		return NULL;
	}

	//static const unsigned __int3264 MAXCOUNT = (((unsigned __int3264)1 << ((sizeof(__int3264)*8) - 1)) - 1);
	static const unsigned __int3264 MAXSCOUNT = ((unsigned __int3264)((signed __int3264)-1)) >> 1;
	static const int x = 1L;

	void Clear()
	{
		if (m_mem)
		{
			delete[] m_mem;
			m_mem = NULL;
		}
		m_array_size=0;	
		m_count=0;
	}
	void ClearCount()
	{
		m_count=0;
	}
	HRESULT Resize(unsigned __int3264  newsize)
	{
		if (newsize <= 0)
			return E_FAIL;
		//T* def = new T();
		//if (def==NULL)
		//	return E_OUTOFMEMORY;
		unsigned __int3264  i,j;
		CArrayElement<T> *newmem;
		newmem = new CArrayElement<T>[newsize];
		if (newmem == NULL)
		{
			//delete def;
			return E_OUTOFMEMORY;
		}
		j=m_count;
		if (j>newsize)
			j=newsize;
		for (i=0 ; i<j ; i++)
		{
			newmem[i] = m_mem[i];
			//m_mem[i].m_data = *def;
		}
		m_count = j;
		m_array_size = newsize;
		//delete def;
		delete[] m_mem;
		m_mem = newmem;
		return S_OK;
	}
	unsigned __int3264 Size()
	{
		return m_array_size;
	}
	unsigned __int3264 Count() const
	{
		return m_count;
	}
	HRESULT Append(const T &item)
	{
		unsigned __int3264 pindex = 0;
		return Append(item, &pindex);
	}
	HRESULT Append(const T &item, unsigned __int3264 *pindex)
	{
		unsigned __int3264  newsize;
		HRESULT r;
		if (m_count >= m_array_size)
		{
			if (m_array_size<1)
				newsize=1;
			else
			{
				if (m_array_size >= MAXSCOUNT)
					return E_FAIL;
				else if (m_array_size < (MAXSCOUNT / 2))
					newsize = m_array_size * 2;
				else
					newsize = MAXSCOUNT;
			}
			r=Resize(newsize);
			if (FAILED(r))
				return r;

		}
		m_mem[m_count].m_data = item;
		*pindex = m_count;
		m_count++;
		return S_OK;
	}
	T &operator[](unsigned __int3264 i) const
	{
		return m_mem[i].m_data;
	}
private:
	CArrayElement<T> *m_mem;
	unsigned __int3264  m_array_size;
	unsigned __int3264  m_count;
};

#endif