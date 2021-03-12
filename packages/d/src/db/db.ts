import * as lmdb from "node-lmdb";
import { ClarionDbManager, ClarionDbTrx } from "@clarionos/bios";

import { ClarionTrx } from "./trx";
import { DATABASE } from "../config";

export class DbManager implements ClarionDbManager {
    lmdbEnv = new lmdb.Env();
    dbs: { [key: string]: any } = {};

    constructor() {
        this.lmdbEnv = new lmdb.Env();
        this.lmdbEnv.open({
            path: DATABASE,
            mapSize: 20 * 1024 * 1024 * 1024, // maximum database size
            maxDbs: 99,
        });
    }

    createTransaction(db: any, writable = false): ClarionTrx {
        return new ClarionTrx(this.lmdbEnv, db, writable);
    }

    async open(name: string): Promise<any> {
        console.info("initializing lmdb with db name:", name);

        const db = this.lmdbEnv.openDbi({
            name,
            create: true,
            keyIsBuffer: true,
        });
        this.dbs[name] = db;
        return db;
    }

    closeAll(): void {
        Object.values(this.dbs).forEach((db) => db && db.close());
        this.dbs = {};
        this.lmdbEnv.close();
    }

    async close(db: any): Promise<void> {
        if (db) {
            console.info("closing db >>>", db, db.name);
            for (const [key, value] of Object.entries(this.dbs)) {
                if (value === db) {
                    delete this.dbs[key];
                    break;
                }
            }
            db.close();
            console.info("dbs after closing", this.dbs);
        }
    }

    async printStats(dbName: string) {
        console.info("Printing Db Stats...");

        const db = this.dbs[dbName] || (await this.open(dbName));
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
