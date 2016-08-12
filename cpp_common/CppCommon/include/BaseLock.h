#ifndef __BASE_LOCK_H__
#define __BASE_LOCK_H__

#include "Atomic.h"

class BaseLock
{
public:
    BaseLock() 
	{
        value = 0;
    };

	virtual int Lock() 
	{
        return cas_lock(&value);
    };

	virtual int unLock() 
	{
        return cas_unlock(&value);
    };
	
private:
    volatile unsigned long value;
	
}; 

class BSLock
{
public:
    BSLock(BaseLock &lock): pLock(&lock) 
	{
        pLock->Lock();
    };

	~BSLock()
	{
        pLock->unLock();
    };

private:
    BaseLock *pLock;
};

#endif // __BASE_LOCK_H__