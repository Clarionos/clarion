import * as lmdb from "node-lmdb";
import { ClarionDbCursor } from "@clarionos/bios";

export class ClarionCursor implements ClarionDbCursor {
    key: Uint8Array | undefined = undefined;
    cursor: any; // todo: define lmdb cursor type

    constructor(txn: any, db: any) {
        this.cursor = new lmdb.Cursor(txn, db);
        this.begin();
    }

    getKey(): Uint8Array | undefined {
        return this.key;
    }

    getValue(): Uint8Array {
        return new Uint8Array(this.cursor.getCurrentBinary()); // todo: convert to Uint8Array?
    }

    hasValue(): boolean {
        return this.key !== undefined;
    }

    begin(): Uint8Array | undefined {
        const newKey = this.cursor.goToFirst();
        this.setKey(newKey);
        return this.key;
    }

    end(): Uint8Array | undefined {
        const newKey = this.cursor.goToLast();
        this.setKey(newKey);
        return this.key;
    }

    async next(): Promise<void> {
        const newKey = this.cursor.goToNext();
        this.setKey(newKey);
    }

    prev(): Uint8Array | undefined {
        const newKey = this.cursor.goToPrev();
        this.setKey(newKey);
        return this.key;
    }

    private setKey(key: any): void {
        if (key) {
            this.key = new Uint8Array(key);
        } else {
            this.key = undefined;
        }
    }
}
