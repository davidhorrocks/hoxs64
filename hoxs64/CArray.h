#pragma once
template <class T>
class CArray;

template <class T>
class CArrayElement
{
public:

	CArrayElement() = default;
	~CArrayElement() = default;
	CArrayElement(const CArrayElement&) = default;
	CArrayElement& operator=(const CArrayElement&) = default;
	CArrayElement(CArrayElement&&) = default;
	CArrayElement& operator=(CArrayElement&&) = default;
	friend class CArray<T>;
private:
	T m_data;
};

template <class T>
class CArray
{
public:
	CArray() noexcept
	{
		m_mem = NULL;
		Clear();
	}

	~CArray()
	{
		Clear();
	}

	CArray(const CArray&) = delete;
	CArray& operator=(const CArray&) = delete;
	CArray(CArray&&) = delete;
	CArray& operator=(CArray&&) = delete;

	typedef int (*CompareFunc)(T &v1, T &v2);

	CArrayElement<T> *FindElement(CompareFunc fn, T &item)
	{
		unsigned int i;
		if (m_count == 0)
		{
			return NULL;
		}

		for (i=0 ; i < m_count ; i++)
		{
			if (fn(m_mem[i], item) == 0)
			{
				return &m_mem[i];
			}
		}

		return NULL;
	}

	static const unsigned int MAXSCOUNT = ((unsigned int)((signed int)-1)) >> 1;
	static const int x = 1L;

	void Clear() noexcept
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

	HRESULT Resize(unsigned int  newsize)
	{
		if (newsize <= 0)
		{
			return E_FAIL;
		}

		//T* def = new T();
		//if (def==NULL)
		//	return E_OUTOFMEMORY;
		unsigned int  i,j;
		CArrayElement<T> *newmem;
		newmem = new CArrayElement<T>[newsize];
		if (newmem == NULL)
		{
			//delete def;
			return E_OUTOFMEMORY;
		}

		j=m_count;
		if (j>newsize)
		{
			j=newsize;
		}

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

	unsigned int Size() const
	{
		return m_array_size;
	}

	unsigned int Count() const
	{
		return m_count;
	}

	HRESULT Append(const T &item)
	{
		unsigned int pindex = 0;
		return Append(item, &pindex);
	}

	HRESULT Append(const T &item, unsigned int *pindex)
	{
		unsigned int  newsize;
		HRESULT r;
		if (m_count >= m_array_size)
		{
			if (m_array_size<1)
			{
				newsize=1;
			}
			else
			{
				if (m_array_size >= MAXSCOUNT)
				{
					return E_FAIL;
				}
				else if (m_array_size < (MAXSCOUNT / 2))
				{
					newsize = m_array_size * 2;
				}
				else
				{
					newsize = MAXSCOUNT;
				}
			}

			r=Resize(newsize);
			if (FAILED(r))
			{
				return r;
			}
		}

		m_mem[m_count].m_data = item;
		*pindex = m_count;
		m_count++;
		return S_OK;
	}

	T &operator[](unsigned int i) const
	{
		return m_mem[i].m_data;
	}
private:
	CArrayElement<T> *m_mem = nullptr;
	unsigned int  m_array_size = 0;
	unsigned int  m_count = 0;
};
