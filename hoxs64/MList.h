#pragma once

template <class T>
class MList;

template <class T>
class MListElement
{
public:
	void MoveToBefore(MListElement<T> *node) noexcept
	{
		MListElement<T> *n,*p;

		assert(node!=NULL);
		assert(this->owner==node->owner);

		if (this == node)
		{
			return;
		}

		if (this->next == node)
		{
			return;
		}

		if (this == owner->m_head)
		{
			owner->m_head = this->next;
		}

		if (this == owner->m_tail)
		{
			owner->m_tail = this->prev;
		}

		if (node == owner->m_head)
		{
			owner->m_head = this;
		}

		//Unlink
		n = this->next;
		p = this->prev;
		if (p)
		{
			p->next = n;
		}

		if (n)
		{
			n->prev = p;
		}

		//Relink
		p = node->prev;
		n = node;
		n->prev=this;
		this->prev=p;
		this->next=n;
		if (p)
		{
			p->next = this;
		}
	}

	void MoveToAfter(MListElement<T> *node) noexcept
	{
		MListElement<T> *n,*p;

		assert(node!=NULL);
		assert(this->owner==node->owner);

		if (this == node)
		{
			return;
		}

		if (this->prev == node)
		{
			return;
		}

		if (this == owner->m_head)
		{
			owner->m_head = this->next;
		}

		if (this == owner->m_tail)
		{
			owner->m_tail = this->prev;
		}

		if (node == owner->m_tail)
		{
			owner->m_tail = this;
		}

		//Unlink
		n = this->next;
		p = this->prev;
		if (p)
		{
			p->next = n;
		}

		if (n)
		{
			n->prev = p;
		}
		
		//Relink
		p = node;
		n = node->next;
		p->next=this;
		this->prev=p;
		this->next=n;
		if (n)
		{
			n->prev = this;
		}
	}

	HRESULT InsertAfter(const T &item) noexcept
	{
		MListElement<T> *e = nullptr;
		try
		{
			e = new MListElement<T>;
		}
		catch (...) {}

		if (e == NULL)
		{
			return E_OUTOFMEMORY;
		}

		e->m_data = item;
		e->owner = owner;

		if (this == owner->m_tail)
		{
			owner->m_tail = e;
		}

		e->next = this->next;
		e->prev = this;

		this->next = e;
		owner->m_count++;
		return S_OK;
	}

	HRESULT InsertBefore(const T &item) noexcept
	{
		MListElement<T> *e;
		e = new MListElement<T>;
		if (e == NULL)
		{
			return E_OUTOFMEMORY;
		}

		e->m_data = item;
		e->owner = owner;

		if (this == owner->m_head)
		{
			owner->m_head = e;
		}

		e->next = this;
		e->prev = this->prev;

		this->prev = e;

		owner->m_count++;
		return S_OK;
	}

	friend class MList<T>;
	T m_data;

	MListElement<T> *Next() const noexcept
	{
		return next;
	}

	MListElement<T> *Prev() const noexcept
	{
		return prev;
	}

	MList<T> *Get_Owner() noexcept
	{
		return owner;
	}
private:
	MListElement<T> *prev = nullptr;
	MListElement<T> *next = nullptr;
	MList<T> *owner = nullptr;
};

template <class T>
class MList
{
public:
	friend class MListElement<T>;

	MList() noexcept
	{
		m_head = NULL;
		m_tail = NULL;
		Clear();
	}

	MList(const MList&) = delete;

	MList& operator=(const MList&) = delete;

	MList(MList&&) = delete;

	MList& operator=(MList&&) = delete;

	~MList() noexcept
	{
		Clear();
	}

	void Clear() noexcept
	{
		while (m_head)
		{
			Remove(m_head);
		}

		m_count = 0;
	}

	typedef int (*CompareFunc)(const T& v1, const T& v2);

	void MergeSort(CompareFunc fn) noexcept
	{
		m_cmp = fn;
		m_CurrentElement = m_head;
		if (m_count <= 1)
		{
			return;
		}

		m_head = MergeSortList(0, m_count - 1);
		m_tail = m_TailSort;
	}


	MListElement<T>* FindElement(CompareFunc fn, const T& item) noexcept
	{
		if (m_count == 0)
		{
			return NULL;
		}

		for (MListElement<T>* p = m_head; p != NULL; p = p->next)
		{
			if (fn(p->m_data, item) == 0)
			{
				return p;
			}
		}

		return NULL;
	}

	void Remove(MListElement<T>* element) noexcept
	{
		MListElement<T>* n, * p;

		assert(element != nullptr);
		assert(m_count > 0);
		if (element == nullptr)
		{
			return;
		}

		assert(element->owner == this);
		if (element->owner != this)
		{
			return;
		}

		if (element == m_head)
		{
			m_head = element->next;
		}

		if (element == m_tail)
		{
			m_tail = element->prev;
		}

		n = element->next;
		p = element->prev;
		if (n)
		{
			n->prev = p;
		}

		if (p)
		{
			p->next = n;
		}

		m_count--;
		if (m_count == 0)
		{
			assert(m_head == NULL);
			assert(m_tail == NULL);
		}

		if (m_head)
		{
			assert(m_count > 0);
		}

		if (m_tail)
		{
			assert(m_count > 0);
		}


		delete element;
	}

	HRESULT Append(const T& item) noexcept
	{
		HRESULT hr;
		MListElement<T>* e = nullptr;
		if (m_tail)
		{
			assert(m_count > 0);
			hr = m_tail->InsertAfter(item);
			if (FAILED(hr))
			{
				return hr;
			}
		}
		else
		{
			assert(m_head == NULL);
			assert(m_count == 0);
			try
			{
				e = new MListElement<T>();
			}
			catch (...) {}
			if (e == NULL)
			{
				return E_OUTOFMEMORY;
			}

			e->m_data = item;
			e->owner = this;
			m_head = e;
			m_tail = e;
			e->next = NULL;
			e->prev = NULL;
			m_count = 1;
		}
		return S_OK;
	}

	unsigned long Count() noexcept
	{
		return m_count;
	}

	MListElement<T>* Head() noexcept
	{
		return m_head;
	}
	MListElement<T>* Tail() noexcept
	{
		return m_tail;
	}

private:
	MListElement<T>* MergeSortList(int start, int end)
	{
		MListElement<T>* n, * e;
		MListElement<T>* list1;
		MListElement<T>* list2;
		MListElement<T>* list3;
		if (start == end)
		{
			n = m_CurrentElement->next;
			e = m_CurrentElement;
			e->next = NULL;
			e->prev = NULL;
			if (n)
			{
				m_CurrentElement = n;
			}

			return e;
		}
		else
		{
			list1 = MergeSortList(start, start + (end - start) / 2);
			list2 = MergeSortList(start + (end - start) / 2 + 1, end);
			list3 = NULL;
			e = NULL;
			n = NULL;

			while (list1 != NULL || list2 != NULL)
			{
				if (list1 == NULL)
				{
					n = list2;
					list2 = list2->next;
				}
				else if (list2 == NULL)
				{
					n = list1;
					list1 = list1->next;
				}
				else
				{
					if (m_cmp(list1->m_data, list2->m_data) > 0)
					{
						n = list2;
						list2 = list2->next;
					}
					else
					{
						n = list1;
						list1 = list1->next;
					}

				}

				if (list3 == NULL)
				{
					list3 = n;
					n->prev = NULL;
					n->next = NULL;
				}
				else
				{
					e->next = n;
					n->prev = e;
					n->next = NULL;
				}

				m_TailSort = e = n;
			}

			return list3;
		}
	}

private:
	CompareFunc m_cmp = nullptr;
	MListElement<T>* m_CurrentElement = nullptr;
	MListElement<T>* m_TailSort = nullptr;
	MListElement<T>* m_head = nullptr;
	MListElement<T>* m_tail = nullptr;
	unsigned long  m_count = 0;
};
