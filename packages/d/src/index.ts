// import * as Level from "level";
import * as lmdb from "node-lmdb";
import { readFileSync } from "fs";
import {
    ClarionConnectionManager,
    ClarionDbManager,
    Context,
} from "@clarionos/bios";

import { DATABASE, CLARION_WASM_PATH, SERVER_PORT } from "./config";
import { ClarionDb } from "./db";
import { ClarionServer } from "./connection";

const server = new ClarionServer(parseInt(SERVER_PORT));

let lmdbEnv = new lmdb.Env();

const main = async () => {
    console.info("> Initializing Clarion...");
    const db = initDb();

    await loadClarion(db, server);
    console.info("> Clarion WASM was loaded");

    // tolerance time since everything is async and we need to give time for
    // completing the wasm actions
    setTimeout(async () => {
        await db.printStats("foo");
        db.closeDbs();
        lmdbEnv.close();
    }, 1500);
};

const initDb = (): ClarionDb => {
    lmdbEnv.open({
        path: DATABASE,
        mapSize: 20 * 1024 * 1024 * 1024, // maximum database size
        maxDbs: 99,
    });
    return new ClarionDb(lmdbEnv);
};

const loadClarion = async (
    dbManager: ClarionDbManager,
    connManager: ClarionConnectionManager
) => {
    const clarionWasm = readFileSync(CLARION_WASM_PATH);
    const context = new Context(["wasm"], dbManager, connManager);
    await context.instanciate(clarionWasm);
    (context.instance!.exports._start as Function)();
};

main();
