import {
  createConnection,
  ProposedFeatures,
} from 'vscode-languageserver/node.js';

import { BackendManager, formatWithFallback } from './backends/manager.js';
import { createServer } from './createServer.js';

const connection = createConnection(ProposedFeatures.all);

createServer(connection, {
  createBackendManager: (settings) => new BackendManager(settings),
  formatDocument: formatWithFallback,
});
