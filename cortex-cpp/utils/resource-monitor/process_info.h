// Inspired by https://github.com/SaulBerrenson/sys_info

#pragma once

#include <cstdint>

namespace cortex::rsrc_mon {
class ProcessInfoI{
public:
    ProcessInfoI() = default;
    virtual ~ProcessInfoI() = default;

    /*!
     * Get Current % Cpu Usage
     * @return percent
     */
   virtual double GetCpuUsage() = 0;
   /*!
    * @brief Get Current Memory Usage
    * @return bytes
    */
   virtual int64_t GetMemoryUsage() = 0;
};


class ProcessInfo : public ProcessInfoI{
public:
    ProcessInfo();
    ~ProcessInfo() override;

    /*!
     * @copydoc ProcessInfoI
     * @return percent;
     */
    double GetCpuUsage() override;
    /*!
     * @copydoc ProcessInfoI
     * @return bytes;
     */
    int64_t GetMemoryUsage() override;

private:
    struct Impl;

    Impl* impl_;
};
}