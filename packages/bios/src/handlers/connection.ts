import { throwError } from "../error";
import {
    ClarionConnectionManager,
    ClarionConnection,
    ClarionConnectionAcceptor,
} from "../interfaces";
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

    createAcceptor = (
        port: number,
        protocolPos: number,
        protocolLen: number
    ) => {
        const protocol = this.memoryHandler.decodeStr(protocolPos, protocolLen);
        return this.memoryHandler.addObj(
            this.connectionManager.createAcceptor(port, protocol)
        );
    };

    listenAcceptor = (
        acceptorIndex: number,
        wasmCbOnConnectionPtr: number,
        wasmCbOnConnectionIndex: number
    ) => {
        const acceptor = this.memoryHandler.getObj<ClarionConnectionAcceptor>(
            acceptorIndex
        );
        acceptor.listen((newConnection: ClarionConnection) => {
            const connection = this.memoryHandler.addObj(newConnection);
            console.info("new listened connection ", connection);
            this.memoryHandler.wasmCallback(
                wasmCbOnConnectionPtr,
                wasmCbOnConnectionIndex,
                connection
            );
        });
    };

    createConnection = (
        uriPos: number,
        uriLen: number,
        wasmCbPtr: number,
        wasmCbIndex: number
    ) => {
        try {
            const uri = this.memoryHandler.decodeStr(uriPos, uriLen);
            const connection = this.connectionManager.createConnection(uri);
            this.memoryHandler.wasmCallback(
                wasmCbIndex,
                wasmCbPtr,
                this.memoryHandler.addObj(connection)
            );
        } catch (e) {
            throwError(e);
        }
    };

    setupConnection = (
        connectionIndex: number,
        wasmCbOnMessagePtr: number,
        wasmCbOnMessageIndex: number,
        wasmCbOnClosePtr: number,
        wasmCbOnCloseIndex: number,
        wasmCbOnErrorPtr: number,
        wasmCbOnErrorIndex: number
    ) => {
        const connection = this.memoryHandler.getObj<ClarionConnection>(
            connectionIndex
        );
        connection.setupOnMessage(async (data: Uint8Array) => {
            this.memoryHandler.wasmCallback(
                wasmCbOnMessageIndex,
                wasmCbOnMessagePtr,
                this.memoryHandler.addObj(data)
            );
        });
        connection.setupOnClose(async (code) => {
            this.memoryHandler.wasmCallback(
                wasmCbOnCloseIndex,
                wasmCbOnClosePtr,
                code
            );
        });
        connection.setupOnError(async () => {
            this.memoryHandler.wasmCallback(
                wasmCbOnErrorIndex,
                wasmCbOnErrorPtr
            );
        });
    };

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
                async (data: Uint8Array) => {
                    this.memoryHandler.wasmCallback(
                        wasmCbOnMessageIndex,
                        wasmCbOnMessagePtr,
                        this.memoryHandler.addObj(data)
                    );
                },
                async (code) => {
                    this.memoryHandler.wasmCallback(
                        wasmCbOnCloseIndex,
                        wasmCbOnClosePtr,
                        code
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
        createAcceptor: this.createAcceptor.bind(this),
        listenAcceptor: this.listenAcceptor.bind(this),
        setupConnection: this.setupConnection.bind(this),
        connect: this.connect.bind(this),
        sendMessage: this.sendMessage.bind(this),
        close: this.close.bind(this),
    };
}
