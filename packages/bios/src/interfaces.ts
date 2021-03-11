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
    sendMessage: (data: Uint8Array) => Promise<void>;
}

export class ClarionWebSocket implements ClarionWsAdapter {
    websocket: WebSocket;

    async connect(uri: string): Promise<WebSocket> {
        return new Promise((resolve, reject) => {
            this.websocket = new WebSocket(uri);
            this.websocket.onopen = () => {
                this.websocket.onopen = () => null;
                this.websocket.onerror = () => null;
                return resolve(this.websocket);
            };
            this.websocket.onerror = reject;
        });
    }

    async sendMessage(data: Uint8Array): Promise<void> {
        this.websocket.send(data);
    }
}
