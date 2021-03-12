import { ClarionDbTrx } from "@clarionos/bios";
import { ClarionCursor } from "./cursor";

export class ClarionTrx implements ClarionDbTrx {
    txn: any;
    db: any;
    cursors: ClarionCursor[] = [];

    constructor(lmdbEnv: any, db: any, writable: boolean) {
        this.db = db;
        this.txn = lmdbEnv.beginTxn({ readOnly: !writable });
    }

    commit() {
        this.releaseCursors();
        return this.txn.commit();
    }

    abort() {
        this.releaseCursors();
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

    async createCursor(): Promise<ClarionCursor> {
        const cursor = new ClarionCursor(this.txn, this.db);
        this.cursors.push(cursor);
        return cursor;
    }

    releaseCursors = () => {
        // todo: need to check for closed cursors...
        this.cursors.forEach((cursor) => cursor.close());
    };
}
