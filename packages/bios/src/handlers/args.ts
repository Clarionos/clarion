import { MemoryHandler } from "./memory";

export class ArgsHandler {
    memoryHandler: MemoryHandler;
    args: string[];
    encodedArgs?: Uint8Array[];

    constructor(memoryHandler: MemoryHandler, args: string[]) {
        this.memoryHandler = memoryHandler;
        this.args = args;
    }

    get_arg_counts = (argc: number, argv_buf_size: number) => {
        if (!this.encodedArgs) {
            const encoder = new TextEncoder();
            this.encodedArgs = this.args.map((s) => encoder.encode(s));
        }
        let size = 0;
        for (const a of this.encodedArgs) size += a.length + 1;
        this.memoryHandler.uint32Array(argc, 1)[0] = this.encodedArgs.length;
        this.memoryHandler.uint32Array(argv_buf_size, 1)[0] = size;
    };

    get_args = (argv: number, argv_buf: number) => {
        const memory = this.memoryHandler.getMemory();
        const u8 = new Uint8Array(memory.buffer);
        const u32 = this.memoryHandler.uint32Array(
            argv,
            this.encodedArgs!.length
        );
        argv = 0;
        for (const a of this.encodedArgs!) {
            u32[argv++] = argv_buf;
            for (const ch of a) u8[argv_buf++] = ch;
            u8[argv_buf++] = 0;
        }
    };

    imports = {
        get_arg_counts: this.get_arg_counts,
        get_args: this.get_args,
    };
}
