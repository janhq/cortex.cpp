
// Inspired by https://github.com/SaulBerrenson/sys_info

#include <cstdint>

namespace cortex::rsrc_mon {
class SystemInfoI{
public:
    SystemInfoI() = default;
    virtual ~SystemInfoI() = default;

    /**
	* @brief Get Total Available Memory Bytes
	*
	* \return
	*/
    virtual int64_t GetTotalMemory() = 0;


    /**
    * @brief Get Total Usage Memory Bytes at System
    *
    * \return
    */
    virtual int64_t GetTotalUsageMemory() = 0;


    virtual double GetCpuTotalUsage() = 0;
};


class SystemInfo: public SystemInfoI{
    
public:
    SystemInfo();
    virtual ~SystemInfo() override;

    int64_t GetTotalMemory() override;
    int64_t GetTotalUsageMemory() override;
    double GetCpuTotalUsage() override;

private:
    struct Impl;

    Impl* impl_;
};
}
