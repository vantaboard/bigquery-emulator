import * as monaco from 'monaco-editor';
import EditorWorker from 'monaco-editor/esm/vs/editor/editor.worker.js?worker';
import JsonWorker from 'monaco-editor/esm/vs/language/json/json.worker.js?worker';
import CssWorker from 'monaco-editor/esm/vs/language/css/css.worker.js?worker';
import HtmlWorker from 'monaco-editor/esm/vs/language/html/html.worker.js?worker';
import TsWorker from 'monaco-editor/esm/vs/language/typescript/ts.worker.js?worker';

// Monaco ESM in Vite: use ?worker imports so Rollup resolves package
// exports (monaco-editor 0.55+ requires the .js suffix under "./*").
self.MonacoEnvironment = {
    getWorker(_workerId, label) {
        switch (label) {
            case 'json':
                return new JsonWorker();
            case 'css':
            case 'scss':
            case 'less':
                return new CssWorker();
            case 'html':
            case 'handlebars':
            case 'razor':
                return new HtmlWorker();
            case 'typescript':
            case 'javascript':
                return new TsWorker();
            default:
                return new EditorWorker();
        }
    },
};

export { monaco };
