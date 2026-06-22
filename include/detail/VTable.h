#pragma once 

// VTable
// function pointer table for type-erased callable operations
template<typename R, typename... Args>
struct VTable {
    R     (*invoke)   (void*, Args&&...);
    void  (*copy)     (void*, const void*);
    void  (*move)     (void*, void*, bool);
    void  (*destroy)  (void*);
};