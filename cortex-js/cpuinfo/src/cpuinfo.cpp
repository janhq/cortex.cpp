#include <napi.h>
#include <algorithm>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <limits.h>
#include <intrin.h>
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif

using namespace std;

#if defined(_WIN32) || defined(LINUX)
#define MAX_INTEL_TOP_LVL 4

class CPUID
{
    uint32_t regs[4];

public:
    explicit CPUID(unsigned funcId, unsigned subFuncId)
    {
#ifdef _WIN32
        __cpuidex((int *)regs, (int)funcId, (int)subFuncId);

#else
        asm volatile("cpuid" : "=a"(regs[0]), "=b"(regs[1]), "=c"(regs[2]), "=d"(regs[3])
                     : "a"(funcId), "c"(subFuncId));
        // ECX is set to zero for CPUID function 4
#endif
    }

    const uint32_t &EAX() const { return regs[0]; }
    const uint32_t &EBX() const { return regs[1]; }
    const uint32_t &ECX() const { return regs[2]; }
    const uint32_t &EDX() const { return regs[3]; }
};

class CPUInfo
{
public:
    CPUInfo();
    string vendor() const { return mVendorId; }
    string model() const { return mModelName; }
    int cores() const { return mNumCores; }
    float cpuSpeedInMHz() const { return mCPUMHz; }
    bool isSSE() const { return mIsSSE; }
    bool isSSE2() const { return mIsSSE2; }
    bool isSSE3() const { return mIsSSE3; }
    bool isSSE41() const { return mIsSSE41; }
    bool isSSE42() const { return mIsSSE42; }
    bool isAVX() const { return mIsAVX; }
    bool isAVX2() const { return mIsAVX2; }
    bool isAVX512() const { return mIsAVX512; }
    bool isHyperThreaded() const { return mIsHTT; }
    int logicalCpus() const { return mNumLogCpus; }

private:
    // Bit positions for data extractions
    static const uint32_t SSE_POS = 0x02000000;
    static const uint32_t SSE2_POS = 0x04000000;
    static const uint32_t SSE3_POS = 0x00000001;
    static const uint32_t SSE41_POS = 0x00080000;
    static const uint32_t SSE42_POS = 0x00100000;
    static const uint32_t AVX_POS = 0x10000000;
    static const uint32_t AVX2_POS = 0x00000020;
    static const uint32_t AVX512_POS = 0x00010000; // AVX-512F bit in EBX from CPUID function 7
    static const uint32_t LVL_NUM = 0x000000FF;
    static const uint32_t LVL_TYPE = 0x0000FF00;
    static const uint32_t LVL_CORES = 0x0000FFFF;

    // Attributes
    string mVendorId;
    string mModelName;
    int mNumSMT;
    int mNumCores;
    int mNumLogCpus;
    float mCPUMHz;
    bool mIsHTT;
    bool mIsSSE;
    bool mIsSSE2;
    bool mIsSSE3;
    bool mIsSSE41;
    bool mIsSSE42;
    bool mIsAVX;
    bool mIsAVX2;
    bool mIsAVX512;
};

CPUInfo::CPUInfo()
{
    // Get vendor name EAX=0
    CPUID cpuID0(0, 0);
    uint32_t HFS = cpuID0.EAX();
    mVendorId += string((const char *)&cpuID0.EBX(), 4);
    mVendorId += string((const char *)&cpuID0.EDX(), 4);
    mVendorId += string((const char *)&cpuID0.ECX(), 4);
    // Get SSE instructions availability
    CPUID cpuID1(1, 0);
    mIsHTT = cpuID1.EDX() & AVX_POS;
    mIsSSE = cpuID1.EDX() & SSE_POS;
    mIsSSE2 = cpuID1.EDX() & SSE2_POS;
    mIsSSE3 = cpuID1.ECX() & SSE3_POS;
    mIsSSE41 = cpuID1.ECX() & SSE41_POS;
    mIsSSE42 = cpuID1.ECX() & SSE41_POS;
    mIsAVX = cpuID1.ECX() & AVX_POS;
    // Get AVX2 and AVX512 instructions availability
    CPUID cpuID7(7, 0);
    mIsAVX2 = cpuID7.EBX() & AVX2_POS;
    mIsAVX512 = cpuID7.EBX() & AVX512_POS;

    string upVId = mVendorId;
    for_each(upVId.begin(), upVId.end(), [](char &in)
             { in = ::toupper(in); });
    // Get num of cores
    if (upVId.find("INTEL") != std::string::npos)
    {
        if (HFS >= 11)
        {
            for (int lvl = 0; lvl < MAX_INTEL_TOP_LVL; ++lvl)
            {
                CPUID cpuID4(0x0B, lvl);
                uint32_t currLevel = (LVL_TYPE & cpuID4.ECX()) >> 8;
                switch (currLevel)
                {
                case 0x01:
                    mNumSMT = LVL_CORES & cpuID4.EBX();
                    break;
                case 0x02:
                    mNumLogCpus = LVL_CORES & cpuID4.EBX();
                    break;
                default:
                    break;
                }
            }
            mNumCores = mNumLogCpus / mNumSMT;
        }
        else
        {
            if (HFS >= 1)
            {
                mNumLogCpus = (cpuID1.EBX() >> 16) & 0xFF;
                if (HFS >= 4)
                {
                    mNumCores = 1 + (CPUID(4, 0).EAX() >> 26) & 0x3F;
                }
            }
            if (mIsHTT)
            {
                if (!(mNumCores > 1))
                {
                    mNumCores = 1;
                    mNumLogCpus = (mNumLogCpus >= 2 ? mNumLogCpus : 2);
                }
            }
            else
            {
                mNumCores = mNumLogCpus = 1;
            }
        }
    }
    else if (upVId.find("AMD") != std::string::npos)
    {
        if (HFS >= 1)
        {
            mNumLogCpus = (cpuID1.EBX() >> 16) & 0xFF;
            if (CPUID(0x80000000, 0).EAX() >= 8)
            {
                mNumCores = 1 + (CPUID(0x80000008, 0).ECX() & 0xFF);
            }
        }
        if (mIsHTT)
        {
            if (!(mNumCores > 1))
            {
                mNumCores = 1;
                mNumLogCpus = (mNumLogCpus >= 2 ? mNumLogCpus : 2);
            }
        }
        else
        {
            mNumCores = mNumLogCpus = 1;
        }
    }
    else
    {
        cout << "Unexpected vendor id" << endl;
    }
    // Get processor brand string
    // This seems to be working for both Intel & AMD vendors
    for (int i = 0x80000002; i < 0x80000005; ++i)
    {
        CPUID cpuID(i, 0);
        mModelName += string((const char *)&cpuID.EAX(), 4);
        mModelName += string((const char *)&cpuID.EBX(), 4);
        mModelName += string((const char *)&cpuID.ECX(), 4);
        mModelName += string((const char *)&cpuID.EDX(), 4);
    }
}
#endif

Napi::Value Method(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::Array array = Napi::Array::New(env);

    uint32_t index = 0;

#if defined(_WIN32) || defined(LINUX)
    CPUInfo cinfo;
    if (cinfo.isAVX512())
    {
        array.Set(index++, Napi::String::New(env, "AVX512"));
    }
    if (cinfo.isAVX2())
    {
        array.Set(index++, Napi::String::New(env, "AVX2"));
    }
    if (cinfo.isAVX())
    {
        array.Set(index++, Napi::String::New(env, "AVX"));
    }

#endif
    return array;
}

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(Napi::String::New(env, "cpuInfo"),
                Napi::Function::New(env, Method));
    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)