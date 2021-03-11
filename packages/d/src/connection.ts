import WebSocket from "ws";

import { ClarionConnectionManager, ClarionConnection } from "@clarionos/bios";
import { IncomingMessage } from "node:http";

export class ClarionWebSocket implements ClarionConnection {
    uri: string;
    websocket: WebSocket;

    constructor(websocket: WebSocket) {
        this.websocket = websocket;
        this.uri = websocket.url;
    }

    async sendMessage(data: Uint8Array): Promise<void> {
        return new Promise((resolve, reject) => {
            this.websocket.send(data, (e) => {
                if (e) {
                    reject(e);
                }
                resolve();
            });
        });
    }

    async close(): Promise<void> {
        this.websocket.close();
    }
}

export class ClarionServer implements ClarionConnectionManager {
    server: WebSocket.Server;
    connections: {
        [key: string]: {
            connection: WebSocket;
            remoteAddress: string;
            connectedAt: Date;
        }; // todo: refactor
    } = {};

    constructor(port: number) {
        this.server = new WebSocket.Server({ port });
        this.server.on("connection", this.handleServerConnection);
        this.printServerStats();
    }

    connect = async (
        uri: string,
        onMessage: (data: Uint8Array) => Promise<void>,
        onClose: (code: number, reason?: string) => Promise<void>,
        onError: () => Promise<void>
    ): Promise<ClarionWebSocket> => {
        return new Promise((resolve, reject) => {
            console.info(">>> ws new connection! connecting to", uri);
            const websocket = new WebSocket(uri);
            websocket.on("open", () => {
                console.info(uri, "ws connected!");
                websocket.on("open", () => null);
                websocket.on("error", onError);
                return resolve(new ClarionWebSocket(websocket));
            });
            websocket.on("message", async (data: Buffer | string) => {
                try {
                    let bytes: Uint8Array;
                    if (typeof data === "string") {
                        bytes = new TextEncoder().encode(data);
                    } else {
                        bytes = new Uint8Array(data);
                    }
                    console.info("received data ", data);
                    onMessage(bytes);
                } catch (e) {
                    console.error("!!! Unknown message data type to handle", e);
                }
            });
            websocket.on("close", () =>
                onClose({ code: 1, reason: "connection closed" } as any)
            );
            websocket.on("error", reject);
        });
    };

    handleServerConnection = (ws: WebSocket, req: IncomingMessage) => {
        const wsId = `${Date.now()}-${Math.floor(Math.random() * 100000)}`; // todo: use an uuid
        this.connections[wsId] = {
            connection: ws,
            remoteAddress: req.socket.remoteAddress,
            connectedAt: new Date(),
        };
        console.info(
            "ClarionD incoming connection: ",
            req.socket.remoteAddress,
            wsId
        );

        ws.on("message", (message) => {
            console.log(
                "<<< clariond wsserver received: %s",
                message,
                " --> echoing..."
            );
            ws.send(message);
        });

        ws.on("close", () => {
            delete this.connections[wsId];
        });

        ws.send(">>> WELCOME TO CLARIOND SERVER!");
    };

    printServerStats = () => {
        console.info(
            `ClarionD Server Connections: ${
                Object.keys(this.connections).length
            }`
        );
        for (const key in this.connections) {
            console.info(
                ">>> ",
                key,
                this.connections[key].remoteAddress,
                this.connections[key].connectedAt
            );
        }
        setTimeout(this.printServerStats, 5000);
    };
}
