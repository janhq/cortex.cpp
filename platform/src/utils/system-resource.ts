import osUtils from 'node-os-utils'

export type MemoryInformation = {
    total: osUtils.MemUsedInfo['totalMemMb'];
    used: osUtils.MemUsedInfo['usedMemMb'];
    free: osUtils.MemFreeInfo['freeMemMb']
}

export const getMemoryInformation = async (): Promise<MemoryInformation> => {
    const [usedMemoryInfo, freeMemoryInfo] = await Promise.all([osUtils.mem.used(), osUtils.mem.free()])
    return {
        total: usedMemoryInfo.totalMemMb,
        used: usedMemoryInfo.usedMemMb,
        free: freeMemoryInfo.freeMemMb
    }
}