import { ClarionDbManager, ClarionDbTrx } from "@clarionos/bios";

import { ClarionTrx } from "./trx";

export class ClarionDb implements ClarionDbManager {
    lmdbEnv: any;
    dbs: any[];

    constructor(lmdbEnv: any) {
        this.lmdbEnv = lmdbEnv;
        this.dbs = [];
    }

    createTransaction(db: any, writable = false): ClarionDbTrx {
        return new ClarionTrx(this.lmdbEnv, db, writable);
    }

    async open(name: string): Promise<any> {
        console.info("initializing lmdb with db name:", name);

        const db = this.lmdbEnv.openDbi({
            name,
            create: true,
            keyIsBuffer: true,
        });
        this.dbs.push(db);
        return Promise.resolve(db);
    }

    close(db: any): void {
        db.close();
        this.dbs = this.dbs.filter((i) => i !== db);
    }

    closeDbs(): void {
        this.dbs.forEach((db) => db.close());
    }

    async printStats(dbName: string) {
        console.info("Printing Db Stats...");

        const db = await this.open(dbName);
        const readTxn = this.createTransaction(db, false);

        const statTxn = this.lmdbEnv.beginTxn({ readOnly: true });
        console.info("Stats:\n", db.stat(statTxn));

        const cursor = await readTxn.createCursor();

        while (cursor && cursor.hasValue()) {
            const keyText = new TextDecoder().decode(cursor.getKey());
            const valueText = new TextDecoder().decode(cursor.getValue());
            console.info(`>>> k: ${keyText} - v: ${valueText}`);
            console.info(cursor.getKey(), cursor.getValue());
            await cursor.next();
        }

        readTxn.abort();
    }
}
