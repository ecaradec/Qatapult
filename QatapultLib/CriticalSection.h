#pragma once
struct CriticalSection {
    CriticalSection() {
       ::InitializeCriticalSection(&m_crit);
    }
    ~CriticalSection() {
       ::DeleteCriticalSection(&m_crit);
    }
    CRITICAL_SECTION m_crit;
};
