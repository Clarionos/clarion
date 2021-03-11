import { ClarionConnectionManager, ClarionConnection } from "@clarionos/bios";

export class ClarionWebSocket implements ClarionConnection {
    uri: string;
    websocket: WebSocket;

    constructor(websocket: WebSocket) {
        this.websocket = websocket;
        this.uri = websocket.url;
    }

    async sendMessage(data: Uint8Array): Promise<void> {
        this.websocket.send(data);
    }

    async close(): Promise<void> {
        this.websocket.close();
    }
}

export class ClarionConnectionCreator implements ClarionConnectionManager {
    async connect(
        uri: string,
        onMessage: (data: Uint8Array) => Promise<void>,
        onClose: (code: number, reason?: string) => Promise<void>,
        onError: () => Promise<void>
    ): Promise<ClarionWebSocket> {
        return new Promise((resolve, reject) => {
            const websocket = new WebSocket(uri);
            websocket.onopen = () => {
                websocket.onopen = () => null;
                websocket.onmessage = async (e: MessageEvent) => {
                    try {
                        let bytes: Uint8Array;
                        if (typeof e.data === "string") {
                            bytes = new TextEncoder().encode(e.data);
                        } else {
                            const dataBuffer = await (e.data as Blob).arrayBuffer();
                            bytes = new Uint8Array(dataBuffer);
                        }
                        console.info("received data ", e.data);
                        onMessage(bytes);
                    } catch (e) {
                        console.error(
                            "!!! Unknown message data type to handle",
                            e
                        );
                    }
                };
                websocket.onclose = (e) => onClose(e.code, e.reason);
                websocket.onerror = onError;
                return resolve(new ClarionWebSocket(websocket));
            };
            websocket.onerror = reject;
        });
    }
}
