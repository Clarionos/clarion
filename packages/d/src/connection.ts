import WebSocket from "ws";

import {
    ClarionConnectionManager,
    ClarionConnection,
    ClarionConnectionAcceptor,
} from "@clarionos/bios";
import { IncomingMessage } from "node:http";

export class ClarionWebSocket implements ClarionConnection {
    uri: string;
    websocket: WebSocket;
    onMessage: (data: Uint8Array) => Promise<void>;
    onClose: (code: number, reason?: string) => Promise<void>;
    onError: () => Promise<void>;

    constructor(
        websocket: WebSocket,
        wasmCbOnMessage,
        wasmCbOnClose,
        wasmCbOnError
    ) {
        this.websocket = websocket;
        this.uri = websocket.url;
        this.onMessage = wasmCbOnMessage;
        this.onClose = wasmCbOnClose;
        this.onError = wasmCbOnError;
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

    openConnection = async (): Promise<void> => {
        return new Promise((resolve, reject) => {
            this.websocket.on("message", async (data: Buffer | string) => {
                try {
                    let bytes: Uint8Array;
                    if (typeof data === "string") {
                        bytes = new TextEncoder().encode(data);
                    } else {
                        bytes = new Uint8Array(data);
                    }
                    console.info("received data ", data);
                    this.onMessage(bytes);
                } catch (e) {
                    console.error("!!! Unknown message data type to handle", e);
                }
            });
            this.websocket.on("close", () =>
                this.onClose({ code: 1, reason: "connection closed" } as any)
            );
            this.websocket.on("open", () => {
                console.info(this.websocket.url, "ws connected!");
                this.websocket.on("open", () => null);
                this.websocket.on("error", this.onError);
                return resolve();
            });
            this.websocket.on("error", reject);
        });
    };
}

export class ClarionWebSocketAcceptor implements ClarionConnectionAcceptor {
    server: WebSocket.Server;
    connections: {
        [key: string]: {
            connection: WebSocket;
            remoteAddress: string;
            connectedAt: Date;
            streamedBytes: number;
        }; // todo: refactor
    } = {};

    constructor(port: number) {
        this.server = new WebSocket.Server({ port });
    }

    printStats = () => {
        console.info(
            `Acceptor(${this.server.options.port}) Connections: ${
                Object.keys(this.connections).length
            }`
        );
        for (const key in this.connections) {
            console.info(">>> ", this.connectionInfo(key));
        }
    };

    connectionInfo = (key: string) => {
        const { remoteAddress, connectedAt, streamedBytes } = this.connections[
            key
        ];
        return `${key} ${remoteAddress} ${connectedAt.toISOString()} - StreamedData: ${streamedBytes}`;
    };

    listen = (
        onMessage: (data: Uint8Array) => Promise<void>,
        onClose: (code: number, reason?: string) => Promise<void>,
        onError: () => Promise<void>
    ) => {
        console.info("Acceptor listening on port ", this.server.options.port);
        setInterval(this.printStats, 1500);
        this.server.on("connection", () => {});
    };

    handleServerConnection = (ws: WebSocket, req: IncomingMessage) => {
        const wsId = `${Date.now()}-${Math.floor(Math.random() * 100000)}`; // todo: use an uuid
        this.connections[wsId] = {
            connection: ws,
            remoteAddress: req.socket.remoteAddress,
            connectedAt: new Date(),
            streamedBytes: 0,
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
            this.connections[wsId].streamedBytes += 1; // todo: get message size
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
}

export class ClarionServer implements ClarionConnectionManager {
    // server: WebSocket.Server;
    acceptors: { [port: number]: ClarionConnectionAcceptor } = {};
    connections: {
        [key: string]: {
            connection: WebSocket;
            remoteAddress: string;
            connectedAt: Date;
        }; // todo: refactor
    } = {};

    // constructor(port: number) {
    //     this.server = new WebSocket.Server({ port });
    // }

    createAcceptor = (port: number, protocol: string) => {
        console.info(
            `>>> Creating acceptor for port ${port} protocol ${protocol}`
        );

        if (this.acceptors[port]) {
            throw new Error("port is already being used");
        }

        let acceptor;
        switch (protocol) {
            case "ws":
                acceptor = new ClarionWebSocketAcceptor(port);
                break;
            default:
                throw new Error("invalid acceptor protocol");
        }

        this.acceptors[port] = acceptor;
        return acceptor;
    };

    connect = async (
        uri: string,
        onMessage: (data: Uint8Array) => Promise<void>,
        onClose: (code: number, reason?: string) => Promise<void>,
        onError: () => Promise<void>
    ): Promise<ClarionWebSocket> => {
        console.info(">>> ws new connection! connecting to", uri);
        const connection = new ClarionWebSocket(
            new WebSocket(uri),
            onMessage,
            onClose,
            onError
        );
        await connection.openConnection();
        return connection;
    };
}
