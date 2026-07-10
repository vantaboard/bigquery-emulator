import {
  BrowserMessageReader,
  BrowserMessageWriter,
  createConnection,
} from 'vscode-languageserver/browser.js';

import {
  BrowserBackendManager,
  formatWithFallbackBrowser,
} from './backends/browserManager.js';
import { createServer } from './createServer.js';

const reader = new BrowserMessageReader(self);
const writer = new BrowserMessageWriter(self);
const connection = createConnection(reader, writer);

createServer(connection as Parameters<typeof createServer>[0], {
  createBackendManager: (settings) => new BrowserBackendManager(settings),
  formatDocument: formatWithFallbackBrowser,
});
