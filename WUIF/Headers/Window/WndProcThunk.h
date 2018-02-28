/*Copyright 2017 Jonathan Campbell

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.*/
// provides thunking code
#pragma once
namespace WUIF {
    //thunk definition x86
#if defined(_M_IX86)
#pragma pack(push,1)
    struct wndprocThunk
    {
        unsigned short  m_push1;	    //push dword ptr [esp] ;push return address
        unsigned short  m_push2;
        unsigned short  m_mov1;         //mov dword ptr [esp+0x4], pThis ;set our new parameter by replacing old return address
        unsigned char   m_mov2;         //(esp+0x4 is first parameter)
        unsigned long   m_this;         //our object's *this to be moved to first parameter
        unsigned char   m_jmp;          //jmp opcode
        unsigned long   m_relproc;      //relative address for WndProc for jmp
        static   HANDLE heapaddr;       //heap address this thunk will be initialized too
        static   long   thunkInstances; //number of instances of this class
        bool Init(_In_ const void *const pThis, _In_ DWORD_PTR proc)
        {
            m_push1   = 0x34ff; //ff 34 24 push DWORD PTR [esp]
            m_push2   = 0xc724;
            m_mov1    = 0x2444; //c7 44 24 04 mov dword ptr [esp+0x4],
            m_mov2    = 0x04;
            m_this    = PtrToUlong(pThis);
            m_jmp     = 0xe9;   //jmp
            //calculate relative address of proc to jump to
            m_relproc = unsigned long((INT_PTR)proc - ((INT_PTR)this + sizeof(wndprocThunk)));
            // write block from data cache and flush from instruction cache
            return FlushInstructionCache(GetCurrentProcess(), this, sizeof(wndprocThunk));
        }
        WNDPROC GetThunkAddress()
        {
            //return the address for *this (wndprocThunk). Note: casted to WNDPROC
            return reinterpret_cast<WNDPROC>(this);
        }
        #ifdef _DEBUG
        static void* operator new(size_t size, int, char const*, int)
        #else
        static void* operator new(size_t size)
        #endif
        {
            InterlockedIncrement(&thunkInstances);
            //since we can't pass a value into delete we need to store our heap address for latter freeing
            if (heapaddr == NULL)
            {
                heapaddr = HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_CREATE_ENABLE_EXECUTE, 0, 0);
                if (heapaddr == NULL)
                {
                    throw WUIF::WUIF_exception(TEXT("HeapCreate failed for Window Thunk"));
                }
            }
            return HeapAlloc(heapaddr, HEAP_ZERO_MEMORY, size);
        }
        static void operator delete(void *pwndprocThunk) noexcept
        {
            //decrement thunk instance count and if zero or negative (i.e. last thunk) destroy the heap
            if (InterlockedDecrement(&thunkInstances) <= 0)
            {
                //check heapaddr is not NULL for added safety
                if (heapaddr)
                {
                    //we do not need to call HeapFree first
                    //if HeapDestroy fails we preserve the heapaddress and just to try to free the allocated heap memory
                    if (HeapDestroy(heapaddr))
                    {
                        heapaddr = NULL;
                    }
                }
            }
            //if we have not destroyed the heap try to free this thunk's memory allocation
            if (heapaddr != NULL)
            {
                HeapFree(heapaddr, 0, pwndprocThunk);
            }
        }
    };
#pragma pack(pop)
    //thunk definition x64
#elif defined(_M_AMD64)
#pragma pack(push,2)
    struct wndprocThunk
    {
        unsigned short     RaxMov;         //movabs rax, pThis ;push object *this into rax
        unsigned long long RaxImm;
        unsigned long      RspMov;         //mov [rsp+28], rax ;push *this into stack space as 5th function parameter
        unsigned short     RspMov1;
        unsigned short     Rax2Mov;        //movabs rax, proc ;load the address of WndProc function into rax
        unsigned long long ProcImm;
        unsigned short     RaxJmp;         //jmp rax ;jump to the start of WndProc function
        static   HANDLE    heapaddr;       //heap address this thunk will be initialized too
        static   long      thunkInstances; //number of instances of this class
        bool Init(_In_ const void *const pThis, _In_ const DWORD_PTR proc)
        {
            RaxMov  = 0xb848;                    //movabs rax (48 B8), pThis
            RaxImm  = reinterpret_cast<unsigned long long>(pThis);
            RspMov  = 0x24448948;                //mov qword ptr [rsp+28h], rax (48 89 44 24 28)
            RspMov1 = 0x9028;                    //to properly byte align the instruction we add a nop (no operation) (90)
            Rax2Mov = 0xb848;                    //movabs rax (48 B8), proc
            ProcImm = static_cast<unsigned long long>(proc);
            RaxJmp  = 0xe0ff;                    //jmp rax (FF EO)
            // write block from data cache and flush from instruction cache
            return FlushInstructionCache(GetCurrentProcess(), this, sizeof(wndprocThunk));
        }
        WNDPROC GetThunkAddress()
        {
            //return the address for *this (wndprocThunk). Note: casted to WNDPROC
            return reinterpret_cast<WNDPROC>(this);
        }
        #ifdef _DEBUG
        static void* operator new(size_t size, int, char const*, int)
        #else
        static void* operator new(size_t size)
        #endif
        {
            InterlockedIncrement(&thunkInstances);
            //since we can't pass a value into delete we need to store our heap address for latter freeing
            if (heapaddr == NULL)
            {
                heapaddr = HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_CREATE_ENABLE_EXECUTE, 0, 0);
                if (heapaddr == NULL)
                {
                    throw WUIF::WUIF_exception(TEXT("HeapCreate failed for Window Thunk"));
                }
            }
            return HeapAlloc(heapaddr, HEAP_ZERO_MEMORY, size);
        }
        static void operator delete(void *pwndprocThunk) noexcept
        {
            //decrement thunk instance count and if zero or negative (i.e. last thunk) destroy the heap
            if (InterlockedDecrement(&thunkInstances) <= 0)
            {
                //check heapaddr is not NULL for added safety
                if (heapaddr)
                {
                    //we do not need to call HeapFree first
                    //if HeapDestroy fails we preserve the heapaddress and just to try to free the allocated heap memory
                    if (HeapDestroy(heapaddr))
                    {
                        heapaddr = NULL;
                    }
                }
            }
            //if we have not destroyed the heap try to free this thunk's memory allocation
            if (heapaddr != NULL)
            {
                HeapFree(heapaddr, 0, pwndprocThunk);
            }
        }
    };
#pragma pack(pop)
#endif
};