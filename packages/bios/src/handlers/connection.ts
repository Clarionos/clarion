import { throwError } from "../error";
import { ClarionConnectionManager, ClarionConnection } from "../interfaces";
import { MemoryHandler } from "./memory";

export class ConnectionHandler {
    memoryHandler: MemoryHandler;
    connectionManager: ClarionConnectionManager;

    constructor(
        memoryHandler: MemoryHandler,
        connectionManager: ClarionConnectionManager
    ) {
        this.memoryHandler = memoryHandler;
        this.connectionManager = connectionManager;
    }

    connect = async (
        uriPos: number,
        uriLen: number,
        wasmCbOnMessagePtr: number,
        wasmCbOnMessageIndex: number,
        wasmCbOnClosePtr: number,
        wasmCbOnCloseIndex: number,
        wasmCbOnErrorPtr: number,
        wasmCbOnErrorIndex: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const uri = this.memoryHandler.decodeStr(uriPos, uriLen);
            const connection = await this.connectionManager.connect(
                uri,
                async (e: MessageEvent) => {
                    const dataBuffer = await e.data.arrayBuffer();
                    this.memoryHandler.wasmCallback(
                        wasmCbOnMessageIndex,
                        wasmCbOnMessagePtr,
                        this.memoryHandler.addObj(new Uint8Array(dataBuffer))
                    );
                },
                async (e: CloseEvent) => {
                    this.memoryHandler.wasmCallback(
                        wasmCbOnCloseIndex,
                        wasmCbOnClosePtr,
                        e.code
                    );
                },
                async () => {
                    this.memoryHandler.wasmCallback(
                        wasmCbOnErrorIndex,
                        wasmCbOnErrorPtr
                    );
                }
            );
            this.memoryHandler.wasmCallback(
                wasmCbIndex,
                wasmCbPtr,
                this.memoryHandler.addObj(connection)
            );
        } catch (e) {
            throwError(e);
        }
    };

    sendMessage = async (
        connectionIndex: number,
        messagePos: number,
        messageLen: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const connection = this.memoryHandler.getObj<ClarionConnection>(
                connectionIndex
            );
            const message = new Uint8Array(
                this.memoryHandler.uint8Array(messagePos, messageLen)
            );
            await connection.sendMessage(message);
            this.memoryHandler.wasmCallback(wasmCbIndex, wasmCbPtr);
        } catch (e) {
            throwError(e);
        }
    };

    close = (connectionIndex: number) => {
        try {
            const connection = this.memoryHandler.getObj<WebSocket>(
                connectionIndex
            );
            connection.close();
        } catch (e) {
            throwError(e);
        }
    };

    imports = {
        connect: this.connect,
        sendMessage: this.sendMessage,
        close: this.close,
    };
}
