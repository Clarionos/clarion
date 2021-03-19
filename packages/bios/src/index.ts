export * from "./interfaces";
import "./polyfills";

import {
    ArgsHandler,
    BasicHandler,
    ConnectionHandler,
    CryptoHandler,
    DatabaseHandler,
    MemoryHandler,
} from "./handlers";
import { ClarionConnectionManager, ClarionDbManager } from "./interfaces";

export class Context {
    instance?: WebAssembly.Instance;
    module?: WebAssembly.Module;
    args: string[];
    memoryHandler: MemoryHandler;
    basicHandler: BasicHandler;
    argsHandler: ArgsHandler;
    databaseHandler: DatabaseHandler;
    connectionHandler: ConnectionHandler;
    cryptoHandler: CryptoHandler;

    constructor(
        args: string[],
        dbManager: ClarionDbManager,
        connectionManager: ClarionConnectionManager
    ) {
        this.args = args;
        this.memoryHandler = new MemoryHandler();
        this.argsHandler = new ArgsHandler(this.memoryHandler, args);
        this.basicHandler = new BasicHandler(this.memoryHandler);
        this.cryptoHandler = new CryptoHandler(this.memoryHandler);
        this.databaseHandler = new DatabaseHandler(
            this.memoryHandler,
            dbManager
        );
        this.connectionHandler = new ConnectionHandler(
            this.memoryHandler,
            connectionManager
        );
    }

    imports = () => {
        const clarion = {
            ...this.argsHandler.imports,
            ...this.basicHandler.imports,
            ...this.databaseHandler.imports,
            ...this.connectionHandler.imports,
            ...this.cryptoHandler.imports,
        };
        return { clarion };
    };

    async instanciate(wasmBytes: BufferSource) {
        this.module = await WebAssembly.compile(wasmBytes);
        this.instance = await WebAssembly.instantiate(
            this.module,
            this.imports()
        );
        this.memoryHandler.setInstance(this.instance);
    }
}
