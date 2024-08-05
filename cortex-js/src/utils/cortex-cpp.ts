import * as cortexCPP from 'cortex-cpp';

const port = process.env.CORTEX_CPP_PORT
  ? parseInt(process.env.CORTEX_CPP_PORT)
  : 3929;
// eslint-disable-next-line @typescript-eslint/ban-ts-comment
// @ts-expect-error
cortexCPP.start(port);
