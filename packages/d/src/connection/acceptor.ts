import WebSocket, { AddressInfo } from "ws";
import { IncomingMessage } from "node:http";
import { ClarionConnection, ClarionConnectionAcceptor } from "@clarionos/bios";

import { Connection } from "./connection";

export class Acceptor implements ClarionConnectionAcceptor {
    server: WebSocket.Server;
    protocol: "wss" | "ws";

    constructor(port: number, protocol?: "wss" | "ws") {
        this.protocol = protocol || "ws";
        this.server = new WebSocket.Server({ port });
    }

    listen = (wasmCb: (connection: ClarionConnection) => void) => {
        console.info("Acceptor listening on port ", this.server.options.port);
        this.server.on("connection", (ws, req) =>
            this.onConnection(ws, req, wasmCb)
        );
    };

    onConnection = (
        ws: WebSocket,
        req: IncomingMessage,
        wasmCb: (connection: ClarionConnection) => void
    ) => {
        const {
            remoteAddress,
            remotePort,
            localAddress,
            localPort,
        } = req.socket;
        const connection = new Connection(
            ws,
            this.protocol,
            remoteAddress,
            remotePort,
            localAddress,
            localPort
        );
        wasmCb(connection);
    };
}
