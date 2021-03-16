import WebSocket from "ws";

import { ClarionConnection } from "@clarionos/bios";

export class Connection implements ClarionConnection {
    uri: string;
    websocket: WebSocket;

    constructor(websocket: WebSocket) {
        this.websocket = websocket;
        this.uri = websocket.url;
    }

    setupOnMessage = (wasmCallback: (data: Uint8Array) => Promise<void>) => {
        this.websocket.on("message", async (data: Buffer | string) => {
            try {
                let bytes: Uint8Array;
                if (typeof data === "string") {
                    bytes = new TextEncoder().encode(data);
                } else {
                    bytes = new Uint8Array(data);
                }
                wasmCallback(bytes);
            } catch (e) {
                console.error("!!! Unknown message data type to handle", e);
            }
        });
    };

    setupOnClose = (wasmCallback) => {
        this.websocket.on("close", () =>
            wasmCallback({ code: 1, reason: "connection closed" })
        );
    };

    setupOnError = (wasmCallback) => {
        this.websocket.on("error", wasmCallback);
    };

    sendMessage = async (data: Uint8Array): Promise<void> => {
        return new Promise((resolve, reject) => {
            this.websocket.send(data, (e) => {
                if (e) {
                    reject(e);
                }
                resolve();
            });
        });
    };

    close = async (): Promise<void> => {
        this.websocket.close();
    };

    openConnection = async (): Promise<void> => {
        return new Promise((resolve, reject) => {
            try {
                this.websocket.on("open", () => {
                    console.info(this.websocket.url, "ws connected!");
                    return resolve();
                });
            } catch (e) {
                reject(e);
            }
        });
    };
}
