import * as cortexCPP from 'cortex-cpp';

process.title = 'Cortex Engine Server';
const port = process.env.CORTEX_CPP_PORT
  ? parseInt(process.env.CORTEX_CPP_PORT)
  : 3929;
cortexCPP.start(port);
