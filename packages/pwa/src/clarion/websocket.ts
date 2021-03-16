import {
    ClarionConnectionManager,
    ClarionConnection,
    ClarionConnectionAcceptor,
} from "@clarionos/bios";

export class ClarionWebSocket implements ClarionConnection {
    uri: string;
    websocket: WebSocket;

    constructor(websocket: WebSocket) {
        this.websocket = websocket;
        this.uri = websocket.url;
    }

    setupOnMessage = (wasmCallback: (data: Uint8Array) => Promise<void>) => {
        this.websocket.onmessage = async (e: MessageEvent) => {
            try {
                let bytes: Uint8Array;
                if (typeof e.data === "string") {
                    bytes = new TextEncoder().encode(e.data);
                } else {
                    const dataBuffer = await (e.data as Blob).arrayBuffer();
                    bytes = new Uint8Array(dataBuffer);
                }
                console.info("received data ", e.data);
                wasmCallback(bytes);
            } catch (e) {
                console.error("!!! Unknown message data type to handle", e);
            }
        };
    };

    setupOnClose = (
        wasmCallback: (code: number, reason?: string) => Promise<void>
    ) => {
        this.websocket.onclose = (e) => wasmCallback(e.code, e.reason);
    };

    setupOnError = (wasmCallback: () => Promise<void>) => {
        this.websocket.onerror = wasmCallback;
    };

    sendMessage = async (data: Uint8Array): Promise<void> => {
        this.websocket.send(data);
    };

    close = async (): Promise<void> => {
        this.websocket.close();
    };

    openConnection = async (): Promise<void> => {
        return new Promise((resolve, reject) => {
            try {
                this.websocket.onopen = () => {
                    console.info(this.websocket.url, "ws connected!");
                    return resolve();
                };
            } catch (e) {
                reject(e);
            }
        });
    };
}

// We can't accept incoming connections on browsers, hence we create this DummyAcceptor
// that will never handle new incomming connections here.
export class DummyAcceptor implements ClarionConnectionAcceptor {
    listen: (cb: (connection: ClarionConnection) => void) => void;
}

export class ClarionConnectionCreator implements ClarionConnectionManager {
    createAcceptor = (_port: number, _protocol: string) => {
        return new DummyAcceptor();
    };

    connect = async (
        uri: string,
        onMessage: (data: Uint8Array) => Promise<void>,
        onClose: (code: number, reason?: string) => Promise<void>,
        onError: () => Promise<void>
    ): Promise<ClarionWebSocket> => {
        console.info(">>> creating new ws, connecting to", uri);
        const connection = new ClarionWebSocket(new WebSocket(uri));
        connection.setupOnMessage(onMessage);
        connection.setupOnClose(onClose);
        connection.setupOnError(onError);
        await connection.openConnection();
        return connection;
    };
}
