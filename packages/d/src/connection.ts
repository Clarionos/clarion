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
    }

    connect = async (
        uri: string,
        onMessage: (data: Uint8Array) => Promise<void>,
        onClose: (code: number, reason?: string) => Promise<void>,
        onError: () => Promise<void>
    ): Promise<ClarionWebSocket> => {
        return new Promise((resolve, reject) => {
            try {
                console.info(">>> ws new connection! connecting to", uri);
                const websocket = new WebSocket(uri);
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
                        console.error(
                            "!!! Unknown message data type to handle",
                            e
                        );
                    }
                });
                websocket.on("close", () =>
                    onClose({ code: 1, reason: "connection closed" } as any)
                );
                websocket.on("error", onError);
                websocket.on("open", () => {
                    console.info(uri, "ws connected!");
                    websocket.on("open", () => null);
                    return resolve(new ClarionWebSocket(websocket));
                });
            } catch (e) {
                reject(e);
            }
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
            console.info("connection closed:", this.connectionInfo(wsId));
            delete this.connections[wsId];
        });

        ws.on("error", (error) => {
            console.error("error on connection", wsId, error);
            delete this.connections[wsId];
        });

        ws.send(">>> WELCOME TO CLARIOND SERVER!");
    };

    connectionInfo = (key: string) => {
        return `${key} ${
            this.connections[key].remoteAddress
        } ${this.connections[key].connectedAt.toISOString()}`;
    };

    printServerStats = () => {
        console.info(
            `ClarionD Server Connections: ${
                Object.keys(this.connections).length
            }`
        );
        for (const key in this.connections) {
            console.info(">>> ", this.connectionInfo(key));
        }
    };

    listen = () => {
        console.info("ClarionD listening on port ", this.server.options.port);
        setInterval(this.printServerStats, 1500);
        return this.server.on("connection", this.handleServerConnection);
    };
}
