export const throwError = (message: string, e?: Error) => {
    console.error(">>> Error:", message);
    if (e) {
        console.error(e);
    }
    throw new Error(message);
};
