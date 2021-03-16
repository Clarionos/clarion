import WebSocket from "ws";
import { IncomingMessage } from "node:http";
import { ClarionConnection, ClarionConnectionAcceptor } from "@clarionos/bios";

import { Connection } from "./connection";

export class Acceptor implements ClarionConnectionAcceptor {
    server: WebSocket.Server;
    statsInterval: NodeJS.Timeout;

    constructor(port: number) {
        this.server = new WebSocket.Server({ port });
    }

    listen = (wasmCb: (connection: ClarionConnection) => void) => {
        console.info("Acceptor listening on port ", this.server.options.port);
        if (!this.statsInterval) {
            this.statsInterval = setInterval(this.printStats, 1500);
        }
        this.server.on("connection", (ws, req) =>
            this.onConnection(ws, req, wasmCb)
        );
    };

    onConnection = (
        ws: WebSocket,
        req: IncomingMessage,
        wasmCb: (connection: ClarionConnection) => void
    ) => {
        console.info(
            "ClarionD incoming connection: ",
            req.socket.remoteAddress
        );
        const connection = new Connection(ws);
        wasmCb(connection);
    };

    printStats = () => {
        console.info(
            `Acceptor(${this.server.options.port}) Connections: ${this.server.clients.size}`
        );
    };
}
