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
                console.info("received data ", data);
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

export class ClarionWebSocketAcceptor implements ClarionConnectionAcceptor {
    server: WebSocket.Server;
    statsInterval: NodeJS.Timeout;
    connections: {
        [key: string]: {
            connection: ClarionWebSocket;
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
            `Acceptor(${this.server.options.port}) Connections: ${this.server.clients.size}`
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

    listen = (wasmCb: (connection: ClarionConnection) => void) => {
        console.info("Acceptor listening on port ", this.server.options.port);
        if (!this.statsInterval) {
            this.statsInterval = setInterval(this.printStats, 1500);
        }
        this.server.on("connection", (ws, req) =>
            this.handleServerConnection(ws, req, wasmCb)
        );
    };

    handleServerConnection = (
        ws: WebSocket,
        req: IncomingMessage,
        wasmCb: (connection: ClarionConnection) => void
    ) => {
        console.info(
            "ClarionD incoming connection: ",
            req.socket.remoteAddress
        );
        const connection = new ClarionWebSocket(ws);
        wasmCb(connection);
    };
}

export class ClarionServer implements ClarionConnectionManager {
    createConnection: (uri: string) => ClarionConnection;
    acceptors: { [port: number]: ClarionConnectionAcceptor } = {};

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
        const connection = new ClarionWebSocket(new WebSocket(uri));
        connection.setupOnMessage(onMessage);
        connection.setupOnClose(onClose);
        connection.setupOnError(onError);
        await connection.openConnection();
        return connection;
    };
}
