// import * as Level from "level";
import * as lmdb from "node-lmdb";
import { readFileSync } from "fs";

import { ClarionDbAdapter, Context } from "@clarionos/bios";
import { DATABASE, CLARION_WASM_PATH } from "./config";

let lmdbEnv = new lmdb.Env();
let openedDb: any;

const main = async () => {
    console.info(">>> Initializing Clarion...");
    const db = await initDb();
    await loadClarion(db);
    printDb(db);
    db.close(openedDb);
    lmdbEnv.close();
};

const initDb = async (): Promise<ClarionDbAdapter> => {
    lmdbEnv.open({
        path: DATABASE,
        mapSize: 20 * 1024 * 1024 * 1024, // maximum database size
        maxDbs: 99,
    });

    return {
        open(name, callback) {
            console.info("initializing lmdb with db name:", name);

            try {
                const db = lmdbEnv.openDbi({
                    name,
                    create: true,
                    keyIsBuffer: true,
                });
                openedDb = db;
                callback(undefined, db);
            } catch (error) {
                callback(error, undefined);
            }
        },
        close: (db: any) => {
            db.close();
        },
        createTransaction: (db: any, writable?: boolean) => {
            const txn = lmdbEnv.beginTxn({ readOnly: !writable });
            return {
                commit: () => txn.commit(),
                abort: () => txn.abort(),
                put: (key: Uint8Array, value: Uint8Array) =>
                    txn.putBinary(db, key, value),
                delete: (key: Uint8Array) => txn.del(db, key),
                get: (key: Uint8Array) => txn.getBinary(db, key),
                createCursor: () => new lmdb.Cursor(txn, db),
                stat: () => db.stat(txn),
            };
        },
    };
};

const loadClarion = async (db: ClarionDbAdapter) => {
    const clarionWasm = readFileSync(CLARION_WASM_PATH);
    const context = new Context(["wasm"], db);
    await context.instanciate(clarionWasm);
    (context.instance!.exports._start as Function)();
};

const printDb = (dbAdapter: any) => {
    console.info("Printing current db...");
    dbAdapter.open("foo", (_err: any, db: any) => {
        const readTxn = dbAdapter.createTransaction(db, false);

        console.info("Printing DB Stats:\n", readTxn.stat());

        console.info("opening cursor");
        const cursor = readTxn.createCursor();
        console.info("cursor opened", cursor);

        for (
            let found = cursor.goToFirst();
            found !== null;
            found = cursor.goToNext()
        ) {
            console.info(`>>> k: ${found} - v: ${cursor.getCurrentBinary()}`);
        }

        readTxn.abort();
    });
};

main();
