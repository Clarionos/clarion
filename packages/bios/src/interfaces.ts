/**
 * Clarion Interfaces
 */

export interface ClarionDbManager {
    open: (name: string) => Promise<any>; // todo: specify type
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

export interface ClarionConnection {
    uri: string;
    sendMessage: (data: Uint8Array) => Promise<void>;
    close: () => Promise<void>;
}
export interface ClarionConnectionManager {
    connect: (
        uri: string,
        onMessage: (data: Uint8Array) => Promise<void>,
        onClose: (code: number, reason?: string) => Promise<void>,
        onError: () => Promise<void>
    ) => Promise<ClarionConnection>;
}
