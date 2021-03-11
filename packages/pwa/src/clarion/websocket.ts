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
    websocket: WebSocket;

    async connect(
        uri: string,
        onMessage: (e: MessageEvent<any>) => Promise<void>,
        onClose: (e: CloseEvent) => Promise<void>,
        onError: () => Promise<void>
    ): Promise<ClarionWebSocket> {
        return new Promise((resolve, reject) => {
            const websocket = new WebSocket(uri);
            websocket.onopen = () => {
                websocket.onopen = () => null;
                websocket.onmessage = onMessage;
                websocket.onclose = onClose;
                websocket.onerror = onError;
                return resolve(new ClarionWebSocket(websocket));
            };
            websocket.onerror = reject;
        });
    }
}
