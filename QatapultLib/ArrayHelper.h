inline CString capitalize(const CString &s) {
    CString tmp=s.Left(1).MakeUpper() + s.Mid(1);
    return tmp;
}


template<typename T>
std::vector<T> Array(T &t0) {
    std::vector<T> v;
    v.push_back(t0);
    return v;
}

template<typename T>
std::vector<T> Array(T &t0,T &t1) {
    std::vector<T> v;
    v.push_back(t0);
    v.push_back(t1);
    return v;
}

template<typename T>
std::vector<T> Array(T &t0,T &t1, T &t2) {
    std::vector<T> v;
    v.push_back(t0);
    v.push_back(t1);
    v.push_back(t2);
    return v;
}

template<typename T>
std::vector<T> Array(T &t0,T &t1, T &t2, T &t3) {
    std::vector<T> v;
    v.push_back(t0);
    v.push_back(t1);
    v.push_back(t2);
    v.push_back(t3);
    return v;
}


template<typename T>
std::vector<T> Array(T &t0,T &t1, T &t2, T &t3, T &t4) {
    std::vector<T> v;
    v.push_back(t0);
    v.push_back(t1);
    v.push_back(t2);
    v.push_back(t3);
    v.push_back(t4);
    return v;
}


template<typename T>
std::vector<T> Array(T &t0,T &t1, T &t2, T &t3, T &t4, T &t5) {
    std::vector<T> v;
    v.push_back(t0);
    v.push_back(t1);
    v.push_back(t2);
    v.push_back(t3);
    v.push_back(t4);
    v.push_back(t5);
    return v;
}

template<typename T>
std::vector<T> Array(T &t0,T &t1, T &t2, T &t3, T &t4, T &t5, T &t6) {
    std::vector<T> v;
    v.push_back(t0);
    v.push_back(t1);
    v.push_back(t2);
    v.push_back(t3);
    v.push_back(t4);
    v.push_back(t5);
    v.push_back(t6);
    return v;
}

typedef unsigned char uint8;
typedef unsigned long uint32;

