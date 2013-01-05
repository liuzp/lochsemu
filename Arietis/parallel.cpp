#include "stdafx.h"
#include "parallel.h"


Mutex::Mutex( bool initialOwner )
{
    m_hMutex = CreateMutex(NULL, initialOwner, NULL);
    if (INVALID_HANDLE_VALUE == m_hMutex) {
        LxFatal("Error creating mutex\n");
    }
}

Mutex::~Mutex()
{
    CloseHandle(m_hMutex);
}

void Mutex::Wait( int t )
{
    WaitForSingleObject(m_hMutex, t);
}

void Mutex::Release()
{
    ReleaseMutex(m_hMutex);
}

Semaphore::Semaphore()
{
    m_semaphore = CreateSemaphore(NULL, 0, 65535, NULL);
    if (INVALID_HANDLE_VALUE == m_semaphore) {
        LxFatal("Error creating semaphore\n");
    }
}

Semaphore::~Semaphore()
{
    CloseHandle(m_semaphore);
}

void Semaphore::Post( int count /*= 1*/ )
{
    if (!ReleaseSemaphore(m_semaphore, count, NULL)) {
        LxFatal("Error releasing semaphore\n");
    }
}

void Semaphore::Wait()
{
    if (WaitForSingleObject(m_semaphore, INFINITE) == WAIT_FAILED) {
        LxFatal("Error waiting for semaphore\n");
    }
}

bool Semaphore::TryWait()
{
    return WaitForSingleObject(m_semaphore, 0) == WAIT_OBJECT_0;
}