import React, { useEffect } from "react";

import { getContext } from "./clarion";

// import { AuthPage } from "./auth";
import { ProfilePage } from "./users";

export const App = () => {
    useEffect(() => {
        loadClarionWasm();
    });

    // todo: implement clarion as a context/hook
    const loadClarionWasm = async () => {
        const clarionContext = await getContext();
        if (!clarionContext.instance) {
            console.info("wasm not loaded");
            return;
            // throw new Error("missing clarion instance!!");
        }
        (clarionContext.instance.exports.test as Function)();
    };

    // TODO: implement router
    // return <AuthPage />;
    return <ProfilePage />;
};
