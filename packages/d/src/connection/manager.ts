import WebSocket from "ws";
import {
    ClarionConnection,
    ClarionConnectionAcceptor,
    ClarionConnectionManager,
} from "@clarionos/bios";

import { Acceptor } from "./acceptor";
import { Connection } from "./connection";

export class ConnectionManager implements ClarionConnectionManager {
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
                acceptor = new Acceptor(port);
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
    ): Promise<Connection> => {
        console.info(">>> creating new ws, connecting to", uri);
        const [protocol, address] = uri.split("://", 2);
        const [remoteAddress, maybeRemotePort] = address.split(":", 2);

        const remotePort =
            maybeRemotePort || (protocol === "ws" ? "80" : "443");

        const ws = new WebSocket(uri);
        const connection = new Connection(
            ws,
            protocol,
            remoteAddress,
            parseInt(remotePort),
            "",
            0
        );
        connection.setupOnMessage(onMessage);
        connection.setupOnClose(onClose);
        connection.setupOnError(onError);
        await connection.openConnection();
        return connection;
    };
}
