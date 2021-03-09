import { ClarionDbCursor, ClarionDbTrx } from "@clarionos/bios";
import { ClarionCursor } from "./cursor";

export class ClarionTrx implements ClarionDbTrx {
    txn: any;
    db: any;

    constructor(lmdbEnv: any, db: any, writable: boolean) {
        this.db = db;
        this.txn = lmdbEnv.beginTxn({ readOnly: !writable });
    }

    commit() {
        return this.txn.commit();
    }

    abort() {
        return this.txn.abort();
    }

    async put(key: Uint8Array, value: Uint8Array): Promise<void> {
        console.info("put", key, value);
        this.txn.putBinary(this.db, key, value);
    }

    async delete(key: Uint8Array): Promise<void> {
        this.txn.del(this.db, key);
    }

    async get(key: Uint8Array): Promise<Uint8Array> {
        const value = this.txn.getBinary(this.db, key);
        if (!value) {
            throw new Error(`invalid value for key ${key}`);
        }
        return new Uint8Array(value);
    }

    async createCursor(): Promise<ClarionDbCursor> {
        const cursor = new ClarionCursor(this.txn, this.db);
        return cursor;
    }
}
