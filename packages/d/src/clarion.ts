import { Context } from "@clarionos/bios";
import { SERVER_PORT } from "./config";
import { ClarionServer } from "./connection";
import { DbManager } from "./db";

let clarionContext;
const server = new ClarionServer(parseInt(SERVER_PORT));
const dbManager = new DbManager();

export const initClarion = async (clarionWasm: Buffer) => {
    clarionContext = new Context(["wasm"], dbManager, server);
    await clarionContext.instanciate(clarionWasm);
    (clarionContext.instance!.exports._start as Function)();

    server.listen();
};

export const exitClarion = () => {
    dbManager.closeAll();
};
