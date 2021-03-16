import { readFileSync } from "fs";

import { CLARION_WASM_PATH } from "./config";
import { exitClarion, initClarion } from "./clarion";

const main = async () => {
    console.info("> Initializing Clarion...");
    const clarionWasm = readFileSync(CLARION_WASM_PATH);
    await initClarion(clarionWasm);
    console.info("> Clarion WASM was loaded");
};

main();

const exitHandler = (exit: boolean) => {
    if (exit) {
        process.exit();
    } else {
        console.info("Closing server...");
        exitClarion();
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
