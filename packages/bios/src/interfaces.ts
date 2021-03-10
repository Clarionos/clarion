/**
 * Clarion Interfaces
 */

export interface ClarionDbAdapter {
    open: (name: string) => Promise<any>;
    createTransaction: (db: any, writable?: boolean) => ClarionDbTrx;
}

export interface ClarionDbTrx {
    commit: () => void;
    abort: () => void;
    put: (key: Uint8Array, value: Uint8Array) => Promise<void>;
    delete: (key: Uint8Array) => Promise<void>;
    get: (key: Uint8Array) => Promise<Uint8Array>;
    createCursor: () => Promise<ClarionDbCursor>;
}

export interface ClarionDbCursor {
    getKey: () => Uint8Array | undefined;
    getValue: () => Uint8Array;
    hasValue: () => boolean;
    next: () => Promise<void>;
}

export interface ClarionWsAdapter {
    connect: (uri: string) => Promise<WebSocket>;
}

export class ClarionWebSocket implements ClarionWsAdapter {
    async connect(uri: string): Promise<WebSocket> {
        return new Promise((resolve, reject) => {
            const websocket = new WebSocket(uri);
            websocket.onopen = () => {
                delete websocket.onopen;
                delete websocket.onerror;

                // websocket.onclose = onClose;
                // websocket.onmessage = onMessage;
                // websocket.onerror = onError;

                return resolve(websocket);
            };
            websocket.onerror = reject;
        });
    }
}
