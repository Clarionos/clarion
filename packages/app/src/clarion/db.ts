import {
    ClarionDbManager,
    ClarionDbCursor,
    ClarionDbTrx,
} from "@clarionos/bios";

const DATABASE_OBJECTSTORE = "clarion";
const DATABASE_KEYPATH = "k";
const DATABASE_VALUEPATH = "v";

export class ClarionDb implements ClarionDbManager {
    open(name: string) {
        return new Promise((resolve, reject) => {
            const req = indexedDB.open(name, 1);
            req.onupgradeneeded = () =>
                req.result.createObjectStore(DATABASE_OBJECTSTORE, {
                    keyPath: DATABASE_KEYPATH,
                });
            req.onsuccess = () => resolve(req.result);
            req.onerror = reject;
        });
    }

    createTransaction(db: any, writable = false) {
        return new ClarionTrx(db, writable);
    }

    close = async (db: any) => {
        console.info("closing database", db);
    };
}

export class ClarionTrx implements ClarionDbTrx {
    trx: any;
    objectStore: any;

    constructor(db: any, writable?: boolean) {
        console.info(db, writable);
        this.trx = db.transaction(
            DATABASE_OBJECTSTORE,
            writable ? "readwrite" : "readonly"
        );
        this.objectStore = this.trx.objectStore(DATABASE_OBJECTSTORE);
    }

    abort() {
        this.trx.abort();
    }
    commit() {
        this.trx.commit();
    }

    put(key: Uint8Array, value: Uint8Array): Promise<void> {
        return new Promise((resolve, reject) => {
            const req = this.objectStore.put({
                [DATABASE_KEYPATH]: key,
                [DATABASE_VALUEPATH]: value,
            });
            req.onsuccess = resolve;
            req.onerror = reject;
        });
    }

    delete(key: Uint8Array): Promise<void> {
        return new Promise((resolve, reject) => {
            const req = this.objectStore.delete(key);
            req.onsuccess = resolve;
            req.onerror = reject;
        });
    }

    get(key: Uint8Array): Promise<Uint8Array> {
        return new Promise((resolve, reject) => {
            const req = this.objectStore.get(key);
            req.onsuccess = () => resolve(req.result[DATABASE_VALUEPATH]);
            req.onerror = reject;
        });
    }

    async createCursor(): Promise<ClarionDbCursor> {
        console.info("creating cursor");
        const cursor = await ClarionCursor.build(this.objectStore);
        console.info(cursor);
        return cursor;
    }
}

export class ClarionCursor implements ClarionDbCursor {
    cursor: any;

    constructor(indexedDbCursor: any) {
        this.cursor = indexedDbCursor;
    }

    getKey() {
        return this.cursor.result.key;
    }

    getValue() {
        return this.cursor.result.value[DATABASE_VALUEPATH];
    }

    hasValue() {
        return this.cursor.result ? true : false;
    }

    next(): Promise<void> {
        return new Promise((resolve, reject) => {
            this.cursor.onsuccess = resolve;
            this.cursor.onerror = reject;
            this.cursor.result.continue();
        });
    }

    close = async () => {
        console.info("closing cursor", this.cursor);
    };

    static async build(objectStore: any): Promise<ClarionCursor> {
        return new Promise((resolve, reject) => {
            const req = objectStore.openCursor();
            req.onsuccess = () => resolve(new ClarionCursor(req));
            req.onerror = reject;
        });
    }
}
