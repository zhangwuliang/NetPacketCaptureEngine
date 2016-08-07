#ifndef __OBJECT_MANAGER_H__
#define __OBJECT_MANAGER_H__

#include "BaseLock.h"
#include "List.h"
#include "Atomic.h"

template <class Object>
class ObjectManager
{
public:
    ObjectManager(unsigned int maxObjectNumber);
    ~ObjectManager();

    Object *AllocateObject(void);
    int FreeObject(Object *object);
	
private:
    int AllocateObjectPool(void);

private:
    BaseLock m_lock;
    Object *m_objects;
    list_node_t m_freeList;
    int m_freeNum;
    int m_currNum;
};



template <class Object>
ObjectManager<Object>::ObjectManager(unsigned int maxObjectNumber)
{
    if (maxObjectNumber <= 0 || maxObjectNumber > 10240) 
	{
        m_currNum = 0;
    } 
	else 
	{
        m_currNum = maxObjectNumber;
        if (AllocateObjectPool() != 0)
		{
            m_currNum = 0;
            m_freeNum = 0;
        }
    }
}

template <class Object>
ObjectManager<Object>::~ObjectManager()
{
    if (m_objects != NULL)
	{
        delete []m_objects;
    }
    m_currNum = 0;
    m_freeNum = 0;
}

template <class Object>
int ObjectManager<Object>::AllocateObjectPool(void)
{
    m_objects = new Object[m_currNum];
    if (m_objects == NULL)
	{
        return -1;
    }

    init_list_node(&m_freeList);
    for (int i = 0; i < m_currNum; i++) 
	{
        init_list_node(&m_objects[i].cs_list);
        list_add_node_tail(&m_objects[i].cs_list, &m_freeList);
    }

    m_freeNum = m_currNum;
    return 0;
}

template <class Object>
Object *ObjectManager<Object>::AllocateObject(void)
{
    BSLock lock(m_lock);
    Object *object = NULL;
    if (list_empty_node(&m_freeList)) 
	{
        return NULL;
    }

    object = (Object *)m_freeList.next;
    list_del_node(m_freeList.next);

    m_freeNum--;
    return object;
}

template <class Object>
int ObjectManager<Object>::FreeObject(Object *object)
{
    BSLock lock(m_lock);
    if (object == NULL) 
	{
        return -1;
    }

    init_list_node(&object->cs_list);

    list_add_node_tail(&object->cs_list, &m_freeList);
    m_freeNum++;
    return 0;
}


#endif
