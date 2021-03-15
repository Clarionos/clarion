/**
 * Clarion Interfaces
 */

export interface ClarionDbManager {
    open: (name: string) => Promise<any>; // todo: specify type
    createTransaction: (db: any, writable?: boolean) => ClarionDbTrx;
    close: (db: any) => Promise<void>;
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
    close: () => Promise<void>;
}

export interface ClarionConnection {
    uri: string;
    sendMessage: (data: Uint8Array) => Promise<void>;
    close: () => Promise<void>;
    setupOnMessage: (wasmCallback: (data: Uint8Array) => Promise<void>) => void;
    setupOnClose: (
        wasmCallback: (code: number, reason?: string) => Promise<void>
    ) => void;
    setupOnError: (wasmCallback: () => Promise<void>) => void;
}

export interface ClarionConnectionAcceptor {
    listen: (cb: (connection: ClarionConnection) => void) => void;
}

export interface ClarionConnectionManager {
    createAcceptor: (
        port: number,
        protocol: string
    ) => ClarionConnectionAcceptor;
    createConnection: (uri: string) => ClarionConnection;
    connect: (
        uri: string,
        onMessage: (data: Uint8Array) => Promise<void>,
        onClose: (code: number, reason?: string) => Promise<void>,
        onError: () => Promise<void>
    ) => Promise<ClarionConnection>;
}
