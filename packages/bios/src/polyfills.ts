if (typeof window !== "undefined") {
    (window as any).global = window;
    (window as any).global.Buffer =
        (window as any).global.Buffer || require("buffer").Buffer;
}
