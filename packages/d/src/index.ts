import { readFileSync } from "fs";
import {
    ClarionConnectionManager,
    ClarionDbManager,
    Context,
} from "@clarionos/bios";

import { CLARION_WASM_PATH, SERVER_PORT } from "./config";
import { DbManager } from "./db";
import { ClarionServer } from "./connection";

let clarionContext;
const server = new ClarionServer(parseInt(SERVER_PORT));
const dbManager = new DbManager();

const main = async () => {
    console.info("> Initializing Clarion...");

    await loadClarion(dbManager, server);
    console.info("> Clarion WASM was loaded");

    // tolerance time since everything is async and we need to give time for
    // completing the wasm actions
    setTimeout(async () => {
        await dbManager.printStats("foo");
    }, 1500);
};

const loadClarion = async (
    dbManager: ClarionDbManager,
    connManager: ClarionConnectionManager
) => {
    const clarionWasm = readFileSync(CLARION_WASM_PATH);
    clarionContext = new Context(["wasm"], dbManager, connManager);
    await clarionContext.instanciate(clarionWasm);
    (clarionContext.instance!.exports._start as Function)();
};

main();
server.listen();

const exitHandler = (exit: boolean) => {
    if (exit) {
        process.exit();
    } else {
        console.info("Closing server...");
        dbManager.closeAll();
    }
};

process.on("exit", () => exitHandler(false));
[
    `SIGINT`,
    `SIGUSR1`,
    `SIGUSR2`,
    `uncaughtException`,
    `SIGTERM`,
].forEach((signal) => process.on(signal, () => exitHandler(true)));
