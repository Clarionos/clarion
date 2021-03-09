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
