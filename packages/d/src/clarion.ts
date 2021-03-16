import { Context } from "@clarionos/bios";
import { ConnectionManager } from "./connection";
import { DbManager } from "./db";

let clarionContext;
const connectionManager = new ConnectionManager();
const dbManager = new DbManager();

export const initClarion = async (clarionWasm: Buffer) => {
    clarionContext = new Context(["wasm"], dbManager, connectionManager);
    await clarionContext.instanciate(clarionWasm);
    clarionContext.instance.exports.initServer();

    // runs the Clarion functionalities tests (setkvs, ws client connection etc.)
    setTimeout(clarionContext.instance.exports.test, 3000);

    setInterval(clarionContext.instance.exports.status, 1500);
};

export const exitClarion = () => {
    dbManager.closeAll();
};
