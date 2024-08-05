import { fileManagerService } from "@/infrastructure/services/file-manager/file-manager.service";
import { existsSync, stat, unlink, writeFileSync } from "fs";

const logCleaningInterval: number = 120000
let timeout: NodeJS.Timeout | undefined;

export async function cleanLogs(
  maxFileSizeBytes?: number | undefined,
  daysToKeep?: number | undefined,
): Promise<void> {
  // clear existing timeout
  // in case we rerun it with different values
  if (timeout) clearTimeout(timeout)
  timeout = undefined


  console.log(
    'Validating app logs. Next attempt in ',
    logCleaningInterval
  )

  const size = maxFileSizeBytes ?? 1 * 1024 * 1024 // 1 MB
  const days = daysToKeep ?? 7 // 7 days
  const filePath = await fileManagerService.getLogPath()
  // Perform log cleaning
  const currentDate = new Date()
  if (existsSync(filePath))
  stat(filePath, (err, stats) => {
    if (err) {
      console.error('Error getting file stats:', err)
      return
    }

    // Check size
    if (stats.size > size) {
      writeFileSync(filePath, '', 'utf8')
    } else {
      // Check age
      const creationDate = new Date(stats.ctime)
      const daysDifference = Math.floor(
        (currentDate.getTime() - creationDate.getTime()) /
          (1000 * 3600 * 24)
      )
      if (daysDifference > days) {
        writeFileSync(filePath, '', 'utf8')
      }
    }
  })

  // Schedule the next execution with doubled delays
  timeout = setTimeout(
    () => this.cleanLogs(maxFileSizeBytes, daysToKeep),
    logCleaningInterval
  )
}
