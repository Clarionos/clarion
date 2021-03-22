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

const exitHandler = (exit: boolean, code: number) => {
    if (exit) {
        process.exit(code);
    } else {
        console.info("Closing server...");
        exitClarion();
    }
};

process.on("exit", (code) => {
    console.info("exit code", code);
    exitHandler(false, code);
});
[`SIGINT`, `SIGUSR1`, `SIGUSR2`, `SIGTERM`, `uncaughtException`].forEach(
    (signal) =>
        process.on(signal, (code) => {
            console.info("signal", signal);
            exitHandler(true, code);
        })
);
