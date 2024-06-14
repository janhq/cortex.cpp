// import { v4 } from 'uuid';
// import axios from 'axios';

// interface OpenTelemetryLog {
//   traceId: string;
//   spanId: string;
//   parentId: string | null;
//   name: string;
//   startTimeUnixNano?: string;
//   endTimeUnixNano?: string;
//   attributes?: {
//     key: string;
//     value: {
//       stringValue: string;
//     };
//   }[];
//   events?: { [key: string]: string }[];
// }

// interface TelemetryEvent {
//   modelId?: string;
//   appVersion: string;
//   osName: string;
//   osVersion: string;
//   architecture: string;
//   sessionId: string;
// }

// const logging = new Logging({ projectId: 'astute-strategy-387104' });
// const url = 'http://localhost:4318/v1/traces';
// const headers = {
//   'Content-Type': 'application/json',
// };

// function getCurrentTimeUnixNano() {
//   return BigInt(Date.now()) * BigInt(1000000);
// }
// export class Telemetry {
//   private queue: OpenTelemetryLog[];
//   private readonly MAX_QUEUE_SIZE = 1;

//   defaultEvent: TelemetryEvent = {
//     appVersion: '0.0.1',
//     osName: process.platform,
//     osVersion: process.version,
//     architecture: process.arch,
//     sessionId: v4(),
//   };

//   constructor() {
//     this.queue = [];
//   }
//   private convertEventToAttributes(
//     event: TelemetryEvent,
//   ): OpenTelemetryLog['attributes'] {
//     return Object.keys(event).map((key: keyof TelemetryEvent) => {
//       // Explicitly define the type of 'key'
//       return {
//         key,
//         value: {
//           stringValue: event[key] as string,
//         },
//       };
//     });
//   }
//   private async send(): Promise<void> {
//     // Send the data to the server
//     console.log('Sending data to the server');
//     const dataDto = {
//       resourceSpans: this.queue.map((trace) => ({
//         resource: {
//           attributes: [
//             {
//               key: 'service.name',
//               value: { stringValue: 'cortex' },
//             },
//           ],
//         },
//         scopeSpans: [
//           {
//             scope: {},
//             spans: [trace],
//           },
//         ],
//       })),
//     };
//     try {
//       console.log(JSON.stringify(dataDto));
//       const response = await axios.post(url, dataDto, { headers });
//       // Selects the log to write to
//       const log = logging.log('TELEMETRY');

//       // The metadata associated with the entry
//       const metadata = {
//         resource: { type: 'global' },
//         severity: 'INFO',
//         test: 'asdasdsahdjkash',
//       };

//       // Prepares a log entry
//       const entry = log.entry(metadata, dataDto.resourceSpans[0].scopeSpans[0]);

//       async function writeLog() {
//         // Writes the log entry
//         await log.write(entry);
//         console.log(`Logged:`);
//       }
//       await writeLog();
//       console.log('Response:', response.data);
//     } catch (error) {
//       console.error('Error:', error);
//     }
//     this.queue = [];
//   }

//   public addEvent(trace: OpenTelemetryLog, modelId: string): void {
//     const atrributes = this.convertEventToAttributes({
//       ...this.defaultEvent,
//       modelId,
//     });

//     this.queue.push({
//       ...trace,
//       startTimeUnixNano: getCurrentTimeUnixNano().toString(),
//       attributes: atrributes,
//     });
//     this.send();
//   }
// }

// export const telemetry = new Telemetry();
