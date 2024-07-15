export function extractCommonHeaders(headers: any) {
  const commonHeaders = [
    'Content-Type',
    'User-Agent',
    'Accept',
    'Authorization',
    'Origin',
    'Referer',
    'Connection',
  ];

  const extractedHeaders: Record<string, string> = {};

  for (const header of commonHeaders) {
    const value = headers[header];
    if (value) {
      extractedHeaders[header] = value;
    }
  }

  return extractedHeaders;
}
